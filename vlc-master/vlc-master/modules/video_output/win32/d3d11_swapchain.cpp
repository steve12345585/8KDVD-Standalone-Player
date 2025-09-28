/*****************************************************************************
 * d3d11_swapchain.c: Direct3D11 swapchain handled by the display module
 *****************************************************************************
 * Copyright (C) 2014-2019 VLC authors and VideoLAN
 *
 * Authors: Martell Malone <martellmalone@gmail.com>
 *          Steve Lhomme <robux4@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc/libvlc.h>
#include <vlc/libvlc_picture.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_renderer_discoverer.h>
#include <vlc/libvlc_media_player.h>

#include <cassert>

#include <windows.h>

#include "dxgi_swapchain.h"
#include "d3d11_swapchain.h"
#include "d3d11_shaders.h"

#include <new>

using Microsoft::WRL::ComPtr;

struct d3d11_local_swapchain
{
    struct dxgi_swapchain *sys = nullptr;

    vlc_object_t           *obj = nullptr;
    d3d11_device_t         *d3d_dev = nullptr;
    bool                   match_display = true;

    ComPtr<ID3D11RenderTargetView> swapchainTargetView[DXGI_MAX_RENDER_TARGET];
};

static bool UpdateSwapchain( d3d11_local_swapchain *display, const libvlc_video_render_cfg_t *cfg )
{
    HRESULT hr;

    D3D11_TEXTURE2D_DESC dsc = { };
    uint8_t bitsPerChannel = 0;

    if ( display->swapchainTargetView[0].Get() ) {
        ComPtr<ID3D11Resource> res;
        display->swapchainTargetView[0]->GetResource( &res );
        if ( res )
        {
            ComPtr<ID3D11Texture2D> res2d;
            if (SUCCEEDED(res.As(&res2d)))
                res2d->GetDesc( &dsc );
        }
        assert(DXGI_GetPixelFormat(display->sys)->formatTexture == dsc.Format);
        bitsPerChannel = DXGI_GetPixelFormat(display->sys)->bitsPerChannel;
    }

    if ( dsc.Width == cfg->width && dsc.Height == cfg->height && cfg->bitdepth <= bitsPerChannel )
        /* TODO also check the colorimetry */
        return true; /* nothing changed */

    for ( size_t i = 0; i < ARRAY_SIZE( display->swapchainTargetView ); i++ )
        display->swapchainTargetView[i].Reset();

    const d3d_format_t *newPixelFormat = NULL;
    /* favor RGB formats first */
    newPixelFormat = FindD3D11Format( display->obj, display->d3d_dev, 0, DXGI_RGB_FORMAT,
                                      cfg->bitdepth > 8 ? 10 : 8,
                                      0, 0, 0,
                                      DXGI_CHROMA_CPU, D3D11_FORMAT_SUPPORT_DISPLAY );
    if (unlikely(newPixelFormat == NULL))
        // try with alpha
        newPixelFormat = FindD3D11Format( display->obj, display->d3d_dev, 0, DXGI_RGB_FORMAT,
                                        cfg->bitdepth > 8 ? 10 : 8,
                                        0, 0, 1,
                                        DXGI_CHROMA_CPU, D3D11_FORMAT_SUPPORT_DISPLAY );
    if (unlikely(newPixelFormat == NULL))
        // try YUV without alpha
        newPixelFormat = FindD3D11Format( display->obj, display->d3d_dev, 0, DXGI_YUV_FORMAT,
                                          cfg->bitdepth > 8 ? 10 : 8,
                                          0, 0, 0,
                                          DXGI_CHROMA_CPU, D3D11_FORMAT_SUPPORT_DISPLAY );
    if (unlikely(newPixelFormat == NULL)) {
        msg_Err(display->obj, "Could not get the SwapChain format.");
        return false;
    }

    ComPtr<IDXGIDevice> pDXGIDevice;
    hr = display->d3d_dev->d3ddevice->QueryInterface(IID_GRAPHICS_PPV_ARGS(&pDXGIDevice));
    if (FAILED(hr)) {
        return false;
    }
    ComPtr<IDXGIAdapter> dxgiadapter;
    hr = pDXGIDevice->GetAdapter(&dxgiadapter);
    if (FAILED(hr)) {
        return false;
    }

    if (!DXGI_UpdateSwapChain( display->sys, dxgiadapter.Get(), display->d3d_dev->d3ddevice,
                               newPixelFormat, cfg, display->match_display ))
        return false;

    ComPtr<ID3D11Resource> pBackBuffer;
    hr = DXGI_GetSwapChain1(display->sys)->GetBuffer( 0, IID_GRAPHICS_PPV_ARGS(&pBackBuffer) );
    if ( FAILED( hr ) ) {
        msg_Err( display->obj, "Could not get the backbuffer for the Swapchain. (hr=0x%lX)", hr );
        return false;
    }

    hr = D3D11_CreateRenderTargets( display->d3d_dev, pBackBuffer.Get(),
                                    DXGI_GetPixelFormat(display->sys), display->swapchainTargetView );
    if ( FAILED( hr ) ) {
        msg_Err( display->obj, "Failed to create the target view. (hr=0x%lX)", hr );
        return false;
    }

    D3D11_ClearRenderTargets( display->d3d_dev, DXGI_GetPixelFormat(display->sys), display->swapchainTargetView );

    return true;
}

void D3D11_LocalSwapchainCleanupDevice( void *opaque )
{
    d3d11_local_swapchain *display = static_cast<d3d11_local_swapchain *>(opaque);
    DXGI_LocalSwapchainCleanupDevice(display->sys);
    delete display;
}

bool D3D11_LocalSwapchainUpdateOutput( void *opaque, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *out )
{
    d3d11_local_swapchain *display = static_cast<d3d11_local_swapchain *>(opaque);
    if ( !UpdateSwapchain( display, cfg ) )
        return false;
    DXGI_SwapchainUpdateOutput(display->sys, out);
    return true;
}

void D3D11_LocalSwapchainSwap( void *opaque )
{
    d3d11_local_swapchain *display = static_cast<d3d11_local_swapchain *>(opaque);
    DXGI_LocalSwapchainSwap( display->sys );
}

void D3D11_LocalSwapchainSetMetadata( void *opaque, libvlc_video_metadata_type_t type, const void *metadata )
{
    d3d11_local_swapchain *display = static_cast<d3d11_local_swapchain *>(opaque);
    DXGI_LocalSwapchainSetMetadata( display->sys, type, metadata );
}

bool D3D11_LocalSwapchainStartEndRendering( void *opaque, bool enter )
{
    d3d11_local_swapchain *display = static_cast<d3d11_local_swapchain *>(opaque);

    if ( enter )
        D3D11_ClearRenderTargets( display->d3d_dev, DXGI_GetPixelFormat(display->sys), display->swapchainTargetView );

    return true;
}

bool D3D11_LocalSwapchainSelectPlane( void *opaque, size_t plane, void *out )
{
    d3d11_local_swapchain *display = static_cast<d3d11_local_swapchain *>(opaque);
    if (!display->swapchainTargetView[plane].Get())
        return false;
    ID3D11RenderTargetView **output = static_cast<ID3D11RenderTargetView **>(out);
    *output = display->swapchainTargetView[plane].Get();
    return true;
}

void *D3D11_CreateLocalSwapchain(vlc_object_t *o, d3d11_device_t *d3d_dev, dxgi_swapchain *sys, bool match_display)
{
    d3d11_local_swapchain *display = new (std::nothrow) d3d11_local_swapchain();
    if (unlikely(display == NULL))
        return NULL;

    display->sys = sys;
    display->obj = o;
    display->d3d_dev = d3d_dev;
    display->match_display = match_display;

    return display;
}
