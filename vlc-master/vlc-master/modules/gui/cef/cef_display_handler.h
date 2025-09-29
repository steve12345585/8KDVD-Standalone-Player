#ifndef VLC_CEF_DISPLAY_HANDLER_H
#define VLC_CEF_DISPLAY_HANDLER_H

#include "include/cef_display_handler.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include <vlc_common.h>
#include <string>

// CEF Display Handler for VLC Integration
class VLCCefDisplayHandler : public CefDisplayHandler {
public:
    VLCCefDisplayHandler(vlc_object_t *obj);
    ~VLCCefDisplayHandler();
    
    // CefDisplayHandler methods
    void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
    void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) override;
    void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) override;
    bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                         cef_log_severity_t level,
                         const CefString& message,
                         const CefString& source,
                         int line) override;
    
    // 8KDVD specific display handling
    void On8KDVDMenuLoaded(const std::string& menu_name);
    void On8KDVDMenuError(const std::string& error_message);
    void On8KDVDResolutionChange(int width, int height);
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetDebugOutput(bool enable);
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    bool debug_output_enabled_;
    
    // 8KDVD state
    std::string current_menu_;
    bool fullscreen_mode_;
    int current_width_;
    int current_height_;
    
    // Helper methods
    void LogCEFMessage(cef_log_severity_t level, const std::string& message);
    void Handle8KDVDDisplayEvent(const std::string& event_type, const std::string& data);
    
    IMPLEMENT_REFCOUNTING(VLCCefDisplayHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefDisplayHandler);
};

#endif // VLC_CEF_DISPLAY_HANDLER_H
