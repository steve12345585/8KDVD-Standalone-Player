#include "cef_load_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <algorithm>

VLCCefLoadHandler::VLCCefLoadHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), load_timeout_ms_(30000),
      menu_loading_(false), menu_loaded_(false) {
    
    msg_Info(vlc_obj_, "CEF load handler created");
}

VLCCefLoadHandler::~VLCCefLoadHandler() {
    msg_Info(vlc_obj_, "CEF load handler destroyed");
}

void VLCCefLoadHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                            bool isLoading,
                                            bool canGoBack,
                                            bool canGoForward) {
    CEF_REQUIRE_UI_THREAD();
    
    msg_Dbg(vlc_obj_, "CEF loading state changed: loading=%s, back=%s, forward=%s",
           isLoading ? "true" : "false",
           canGoBack ? "true" : "false",
           canGoForward ? "true" : "false");
    
    // Handle 8KDVD menu loading state
    if (Is8KDVDMenuURL(current_menu_url_)) {
        menu_loading_ = isLoading;
        if (!isLoading && menu_loading_) {
            On8KDVDMenuLoadEnd(current_menu_url_, true);
        }
    }
}

void VLCCefLoadHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefRequest> request) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!frame->IsMain()) return;
    
    std::string url = request->GetURL().ToString();
    current_menu_url_ = url;
    
    msg_Info(vlc_obj_, "CEF load started: %s", url.c_str());
    
    // Handle 8KDVD menu load start
    if (Is8KDVDMenuURL(url)) {
        On8KDVDMenuLoadStart(url);
    }
}

void VLCCefLoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 int httpStatusCode) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!frame->IsMain()) return;
    
    std::string url = frame->GetURL().ToString();
    bool success = (httpStatusCode >= 200 && httpStatusCode < 300);
    
    msg_Info(vlc_obj_, "CEF load ended: %s (status: %d)", url.c_str(), httpStatusCode);
    
    // Handle 8KDVD menu load end
    if (Is8KDVDMenuURL(url)) {
        On8KDVDMenuLoadEnd(url, success);
    }
}

void VLCCefLoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   ErrorCode errorCode,
                                   const CefString& errorText,
                                   const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!frame->IsMain()) return;
    
    std::string url = failedUrl.ToString();
    std::string error = errorText.ToString();
    
    msg_Err(vlc_obj_, "CEF load error: %s (code: %d, error: %s)", 
           url.c_str(), static_cast<int>(errorCode), error.c_str());
    
    // Handle 8KDVD menu load error
    if (Is8KDVDMenuURL(url)) {
        On8KDVDMenuLoadError(url, error);
    }
}

void VLCCefLoadHandler::On8KDVDMenuLoadStart(const std::string& menu_url) {
    menu_loading_ = true;
    menu_loaded_ = false;
    
    std::string menu_name = ExtractMenuName(menu_url);
    msg_Info(vlc_obj_, "8KDVD menu load started: %s", menu_name.c_str());
    
    if (vlc_integration_enabled_) {
        Handle8KDVDLoadEvent("menu_load_start", menu_name);
    }
}

void VLCCefLoadHandler::On8KDVDMenuLoadEnd(const std::string& menu_url, bool success) {
    menu_loading_ = false;
    menu_loaded_ = success;
    
    std::string menu_name = ExtractMenuName(menu_url);
    msg_Info(vlc_obj_, "8KDVD menu load ended: %s (success: %s)", 
           menu_name.c_str(), success ? "true" : "false");
    
    if (vlc_integration_enabled_) {
        std::string data = menu_name + ":" + (success ? "success" : "failed");
        Handle8KDVDLoadEvent("menu_load_end", data);
    }
}

void VLCCefLoadHandler::On8KDVDMenuLoadError(const std::string& menu_url, const std::string& error) {
    menu_loading_ = false;
    menu_loaded_ = false;
    
    std::string menu_name = ExtractMenuName(menu_url);
    msg_Err(vlc_obj_, "8KDVD menu load error: %s - %s", menu_name.c_str(), error.c_str());
    
    if (vlc_integration_enabled_) {
        std::string data = menu_name + ":" + error;
        Handle8KDVDLoadEvent("menu_load_error", data);
    }
}

void VLCCefLoadHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefLoadHandler::SetLoadTimeout(int timeout_ms) {
    load_timeout_ms_ = timeout_ms;
    msg_Info(vlc_obj_, "Load timeout set: %d ms", timeout_ms);
}

void VLCCefLoadHandler::Handle8KDVDLoadEvent(const std::string& event_type, const std::string& data) {
    msg_Dbg(vlc_obj_, "8KDVD load event: %s = %s", event_type.c_str(), data.c_str());
    
    // TODO: Implement specific 8KDVD load event handling
    // This would integrate with VLC's menu system
}

bool VLCCefLoadHandler::Is8KDVDMenuURL(const std::string& url) {
    // Check if URL is an 8KDVD menu
    return (url.find("8kdvd") != std::string::npos ||
            url.find("8KDVD") != std::string::npos ||
            url.find("menu") != std::string::npos);
}

std::string VLCCefLoadHandler::ExtractMenuName(const std::string& url) {
    // Extract menu name from URL
    size_t last_slash = url.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string filename = url.substr(last_slash + 1);
        size_t dot_pos = filename.find_last_of('.');
        if (dot_pos != std::string::npos) {
            return filename.substr(0, dot_pos);
        }
        return filename;
    }
    return url;
}

// Error handling
void VLCCefLoadHandler::HandleLoadError(const std::string& url, const std::string& error) {
    msg_Err(vlc_obj_, "CEF load error: %s - %s", url.c_str(), error.c_str());
    
    if (vlc_integration_enabled_) {
        Handle8KDVDLoadEvent("load_error", url + ":" + error);
    }
}

// Loading state management
void VLCCefLoadHandler::SetLoadingState(bool loading) {
    menu_loading_ = loading;
    msg_Dbg(vlc_obj_, "8KDVD loading state: %s", loading ? "loading" : "idle");
    
    if (vlc_integration_enabled_) {
        Handle8KDVDLoadEvent("loading_state", loading ? "loading" : "idle");
    }
}

bool VLCCefLoadHandler::IsLoading() const {
    return menu_loading_;
}

bool VLCCefLoadHandler::IsLoaded() const {
    return menu_loaded_;
}
