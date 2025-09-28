/*****************************************************************************
 * Copyright (C) 2020 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#include <QScreen>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <vlc_window.h>
#include <vlc_cxx_helpers.hpp>

#include "compositor_x11.hpp"
#include "compositor_x11_renderwindow.hpp"
#include "compositor_x11_uisurface.hpp"
#include "mainctx.hpp"
#include "interface_window_handler.hpp"
#include "video_window_handler.hpp"
#include "mainui.hpp"
#include "compositor_accessibility.hpp"

#ifdef QT_DECLARATIVE_PRIVATE
#include <private/qquickitem_p.h>
#include <private/qquickwindow_p.h>
#endif

#ifndef X_DISPLAY_MISSING
#include <X11/Xlib.h>
#endif

using namespace vlc;

int CompositorX11::windowEnable(const vlc_window_cfg_t *)
{
    commonWindowEnable();
    m_renderWindow->enableVideoWindow();
    return VLC_SUCCESS;
}

void CompositorX11::windowDisable()
{
    m_renderWindow->disableVideoWindow();
    commonWindowDisable();
}

CompositorX11::CompositorX11(qt_intf_t *p_intf, QObject *parent)
    : CompositorVideo(p_intf, parent)
{
}

CompositorX11::~CompositorX11()
{
    if (m_conn)
        xcb_disconnect(m_conn);
}

static bool checkExtensionPresent(qt_intf_t* intf, xcb_connection_t *conn, const char* extension)
{
    bool ret = queryExtension(conn, extension, nullptr, nullptr);
    if (! ret)
        msg_Warn(intf, "X11 extension %s is missing", extension);
    return ret;
}

#define REGISTER_XCB_EXTENSION(c, extension, minor, major) \
    do { \
        xcb_ ## extension  ##_query_version_cookie_t cookie = xcb_## extension  ##_query_version(c, minor, major); \
        xcb_generic_error_t* error = nullptr; \
        auto reply = wrap_cptr(xcb_## extension  ##_query_version_reply(c, cookie, &error)); \
        if (error) { \
            msg_Warn(m_intf, "X11 extension %s is too old", #extension); \
            free(error); \
            return false; \
        } \
    } while(0)


bool CompositorX11::init()
{
    if (!qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
    {
        msg_Info(m_intf, "not running an X11 platform");
        return false;
    }

    if (QQuickWindow::graphicsApi() != QSGRendererInterface::OpenGL)
    {
        msg_Warn(m_intf, "Running on X11, but graphics api is not OpenGL." \
                         "CompositorX11 only supports OpenGL for now.\n" \
                         "FIXME: Support vulkan as well.");
        return false;
    }

    //open a separated X11 connection from Qt to be able to receive
    //damage events independently from Qt (even when Qt's main loop is stalled)
    m_conn = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(m_conn) != 0)
    {
        msg_Warn(m_intf, "can't open X11 connection");
        return false;
    }

    //check X11 extensions
    if (!checkExtensionPresent(m_intf, m_conn, "DAMAGE"))
        return false;
    REGISTER_XCB_EXTENSION(m_conn, damage, XCB_DAMAGE_MAJOR_VERSION, XCB_DAMAGE_MINOR_VERSION);

    if (!checkExtensionPresent(m_intf, m_conn, "RENDER"))
        return false;
    //0.11 required for Blend mode operators
    static_assert (XCB_RENDER_MAJOR_VERSION == 0 && XCB_RENDER_MINOR_VERSION >= 11,
            "X11 Render version is too old, 0.11+ is required");
    REGISTER_XCB_EXTENSION(m_conn, render, XCB_RENDER_MAJOR_VERSION, XCB_RENDER_MINOR_VERSION);

    if (!checkExtensionPresent(m_intf, m_conn, "Composite"))
        return false;
    //0.2 is required for NameWindowPixmap
    static_assert (XCB_COMPOSITE_MAJOR_VERSION == 0 && XCB_COMPOSITE_MINOR_VERSION >= 2,
            "X11 Composite version is too old, 0.2+ is required");
    REGISTER_XCB_EXTENSION(m_conn, composite, XCB_COMPOSITE_MAJOR_VERSION, XCB_COMPOSITE_MINOR_VERSION);

    if (!checkExtensionPresent(m_intf, m_conn, "XFIXES"))
        return false;
    //2.x is required for SetWindowShapeRegion
    static_assert (XCB_XFIXES_MAJOR_VERSION >= 2,
            "X11 Fixes version is too old, 2.0+ is required");
    REGISTER_XCB_EXTENSION(m_conn, xfixes, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);

    // check whether we're running under "XWAYLAND"
    auto screens = qApp->screens();
    bool isXWayland = std::any_of(screens.begin(), screens.end(), [](QScreen* screen){
        return screen->name().startsWith("XWAYLAND");
    });

    if (isXWayland)
    {
        // if the X11 server is XWayland, test if it is a broken version
        const xcb_setup_t* setup = xcb_get_setup(m_conn);

        if (setup == nullptr)
        {
            msg_Info(m_intf, "error checking for XWayland version");
            return false;
        }

        if (setup->release_number < 12100000u)
        {
            msg_Info(m_intf, "running a broken XWayland version, disabling X11 composition");
            return false;
        }
    }

#if !defined(QT_NO_ACCESSIBILITY) && defined(QT_DECLARATIVE_PRIVATE)
   QAccessible::installFactory(&compositionAccessibleFactory);
#endif

    return true;
}

bool CompositorX11::makeMainInterface(MainCtx* mainCtx, std::function<void (QQuickWindow *)> aboutToShowQuickWindowCallback)
{
    m_mainCtx = mainCtx;

    m_renderWindow = std::make_unique<vlc::CompositorX11RenderWindow>(m_intf, m_conn);
    if (!m_renderWindow->init())
        return false;

    m_videoWidget = std::make_unique<DummyNativeWidget>();
    // widget would normally require WindowTransparentForInput, without this
    // we end up with an invisible area within our window that grabs our mouse events.
    // But using this this causes rendering issues with some VoutDisplay
    // (xcb_render for instance) instead, we manually, set a null input region afterwards
    // see setTransparentForMouseEvent
    m_videoWidget->winId();
    m_videoWidget->windowHandle()->setParent(m_renderWindow.get());

    //update manually EventMask as we don't use WindowTransparentForInput
    const uint32_t mask = XCB_CW_EVENT_MASK;
    const uint32_t values = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE;

    const auto connection =  qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->connection();

    xcb_change_window_attributes(connection, m_videoWidget->winId(), mask, &values);
    setTransparentForMouseEvent(connection, m_videoWidget->winId());
    m_videoWidget->setVisible(true);

    connect(m_mainCtx, &MainCtx::hasAcrylicSurfaceChanged, m_renderWindow.get(), &CompositorX11RenderWindow::setAcrylic);

    m_qmlView = std::make_unique<CompositorX11UISurface>(m_renderWindow.get());

    m_qmlView->setFlag(Qt::WindowType::WindowTransparentForInput);
    m_qmlView->setParent(m_renderWindow.get());
    m_qmlView->create();
    CompositorVideo::Flags flags = CompositorVideo::CAN_SHOW_PIP | HAS_ACRYLIC;
    if (m_renderWindow->supportExtendedFrame())
        flags |= CompositorVideo::HAS_EXTENDED_FRAME;
    if (!commonGUICreate(m_renderWindow.get(), m_qmlView.get(), flags))
        return false;

    if (aboutToShowQuickWindowCallback)
        aboutToShowQuickWindowCallback(m_qmlView->getOffscreenWindow());

    m_qmlView->setVisible(true);

    if (m_mainCtx->hasAcrylicSurface())
        m_renderWindow->m_hasAcrylic = true;

    m_renderWindow->setInterfaceWindow(m_qmlView.get());
    m_renderWindow->setVideoWindow(m_videoWidget->windowHandle());

    m_renderWindow->startRendering();

    return true;
}

void CompositorX11::destroyMainInterface()
{
    commonIntfDestroy();
    m_videoWidget.reset();
    m_renderWindow.reset();
}

void CompositorX11::unloadGUI()
{
    m_renderWindow->stopRendering();
    m_qmlView.reset();
    commonGUIDestroy();
}

void CompositorX11::onSurfacePositionChanged(const QPointF& position)
{
    m_renderWindow->setVideoPosition({static_cast<int>(position.x()), static_cast<int>(position.y())});
}

void CompositorX11::onSurfaceSizeChanged(const QSizeF& size)
{
    const QSizeF area = (size / m_videoWidget->window()->devicePixelRatioF());
    m_renderWindow->setVideoSize({static_cast<int>(std::ceil(area.width())), static_cast<int>(std::ceil(area.height()))});
}

bool CompositorX11::setupVoutWindow(vlc_window_t* p_wnd, VoutDestroyCb destroyCb)
{
    if (m_wnd)
        return false;

    p_wnd->type = VLC_WINDOW_TYPE_XID;
    p_wnd->handle.xid = m_videoWidget->winId();
#ifndef X_DISPLAY_MISSING
    assert(qGuiApp);
    const auto x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    assert(x11App); // already checked in `init()`
    assert(x11App->display());
    p_wnd->display.x11 = XDisplayString(x11App->display());
#endif

    commonSetupVoutWindow(p_wnd, destroyCb);
    return true;
}

QWindow* CompositorX11::interfaceMainWindow() const { return m_renderWindow.get(); }

QQuickWindow *CompositorX11::quickWindow() const
{
    assert(m_qmlView);
    return m_qmlView->getOffscreenWindow();
}

QQuickItem * CompositorX11::activeFocusItem() const /* override */
{
    return m_qmlView->activeFocusItem();
}

///////// DummyNativeWidget

DummyNativeWidget::DummyNativeWidget(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_MouseTracking, true);
    setAttribute(Qt::WA_TranslucentBackground, false);
    QWindow* w =  window()->windowHandle();
    assert(w);
    /*
     * force the window not to have an alpha channel, the  parent window
     * may have an alpha channel and child widget would inhertit the format
     * even if we set Qt::WA_TranslucentBackground to false. having an alpha
     * in this surface would lead to the video begin semi-tranparent.
     */
    QSurfaceFormat format = w->format();
    format.setAlphaBufferSize(0);
    w->setFormat(format);
}

DummyNativeWidget::~DummyNativeWidget()
{

}

QPaintEngine* DummyNativeWidget::paintEngine() const
{
    return nullptr;
}
