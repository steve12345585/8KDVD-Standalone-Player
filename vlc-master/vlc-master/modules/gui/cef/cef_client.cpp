#include "cef_client.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>

VLCCefClient::VLCCefClient(vlc_object_t *obj) 
    : vlc_obj_(obj), render_width_(1920), render_height_(1080) {
}

void VLCCefClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) {
    CEF_REQUIRE_UI_THREAD();
    msg_Info(vlc_obj_, "CEF browser title changed: %s", title.ToString().c_str());
}

bool VLCCefClient::OnBeforePopup(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    const CefString& target_frame_name,
    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
    bool user_gesture,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings,
    CefRefPtr<CefDictionaryValue>& extra_info,
    bool* no_javascript_access) {
    
    CEF_REQUIRE_UI_THREAD();
    
    // Block popups in 8KDVD menu mode
    msg_Warn(vlc_obj_, "CEF popup blocked: %s", target_url.ToString().c_str());
    return true;
}

void VLCCefClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    browser_ = browser;
    msg_Info(vlc_obj_, "CEF browser created successfully");
}

void VLCCefClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    if (browser_->IsSame(browser)) {
        browser_ = nullptr;
    }
    msg_Info(vlc_obj_, "CEF browser closing");
}

void VLCCefClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              TransitionType transition_type) {
    CEF_REQUIRE_UI_THREAD();
    
    if (frame->IsMain()) {
        msg_Info(vlc_obj_, "CEF started loading: %s", 
                frame->GetURL().ToString().c_str());
    }
}

void VLCCefClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            int httpStatusCode) {
    CEF_REQUIRE_UI_THREAD();
    
    if (frame->IsMain()) {
        msg_Info(vlc_obj_, "CEF finished loading: %s (status: %d)", 
                frame->GetURL().ToString().c_str(), httpStatusCode);
                
        // Inject 8KDVD JavaScript API
        if (httpStatusCode == 200) {
            const char* js_code = 
                "if (typeof window.vlc === 'undefined') {"
                "  window.vlc = {};"
                "  window.vlc.playTitle = function(titleId) {"
                "    console.log('Playing title: ' + titleId);"
                "  };"
                "  window.vlc.showMenu = function(menuType) {"
                "    console.log('Showing menu: ' + menuType);"
                "  };"
                "  window.vlc.navigateMenu = function(direction) {"
                "    console.log('Navigate menu: ' + direction);"
                "  };"
                "  console.log('VLC 8KDVD API initialized');"
                "}";
            
            frame->ExecuteJavaScript(js_code, frame->GetURL(), 0);
        }
    }
}

void VLCCefClient::OnLoadError(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              ErrorCode errorCode,
                              const CefString& errorText,
                              const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();
    
    if (frame->IsMain()) {
        msg_Err(vlc_obj_, "CEF load error: %s (%d) for URL: %s",
               errorText.ToString().c_str(), errorCode, 
               failedUrl.ToString().c_str());
    }
}

void VLCCefClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    CEF_REQUIRE_UI_THREAD();
    rect = CefRect(0, 0, render_width_, render_height_);
}

void VLCCefClient::OnPaint(CefRefPtr<CefBrowser> browser,
                          PaintElementType type,
                          const RectList& dirtyRects,
                          const void* buffer,
                          int width,
                          int height) {
    CEF_REQUIRE_UI_THREAD();
    
    if (type == PET_VIEW) {
        // Handle the rendered frame buffer
        // This is where we would integrate with VLC's video output
        // For now, just log the paint event
        msg_Dbg(vlc_obj_, "CEF paint: %dx%d", width, height);
    }
}

void VLCCefClient::SetRenderSize(int width, int height) {
    render_width_ = width;
    render_height_ = height;
    
    if (browser_) {
        browser_->GetHost()->WasResized();
    }
}
