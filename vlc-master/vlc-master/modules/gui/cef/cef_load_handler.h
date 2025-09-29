#ifndef VLC_CEF_LOAD_HANDLER_H
#define VLC_CEF_LOAD_HANDLER_H

#include "include/cef_load_handler.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include <vlc_common.h>
#include <string>

// CEF Load Handler for VLC Integration
class VLCCefLoadHandler : public CefLoadHandler {
public:
    VLCCefLoadHandler(vlc_object_t *obj);
    ~VLCCefLoadHandler();
    
    // CefLoadHandler methods
    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                              bool isLoading,
                              bool canGoBack,
                              bool canGoForward) override;
    
    void OnLoadStart(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     CefRefPtr<CefRequest> request) override;
    
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   int httpStatusCode) override;
    
    void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl) override;
    
    // 8KDVD specific load handling
    void On8KDVDMenuLoadStart(const std::string& menu_url);
    void On8KDVDMenuLoadEnd(const std::string& menu_url, bool success);
    void On8KDVDMenuLoadError(const std::string& menu_url, const std::string& error);
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetLoadTimeout(int timeout_ms);
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    int load_timeout_ms_;
    
    // 8KDVD state
    std::string current_menu_url_;
    bool menu_loading_;
    bool menu_loaded_;
    
    // Helper methods
    void Handle8KDVDLoadEvent(const std::string& event_type, const std::string& data);
    bool Is8KDVDMenuURL(const std::string& url);
    std::string ExtractMenuName(const std::string& url);
    
    IMPLEMENT_REFCOUNTING(VLCCefLoadHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefLoadHandler);
};

#endif // VLC_CEF_LOAD_HANDLER_H
