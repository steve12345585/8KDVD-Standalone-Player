#ifndef VLC_CEF_REQUEST_HANDLER_H
#define VLC_CEF_REQUEST_HANDLER_H

#include "include/cef_request_handler.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_request.h"
#include "include/cef_response.h"
#include <vlc_common.h>
#include <string>

// CEF Request Handler for VLC Integration
class VLCCefRequestHandler : public CefRequestHandler {
public:
    VLCCefRequestHandler(vlc_object_t *obj);
    ~VLCCefRequestHandler();
    
    // CefRequestHandler methods
    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefRequest> request,
                        bool user_gesture,
                        bool is_redirect) override;
    
    bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          const CefString& target_url,
                          WindowOpenDisposition target_disposition,
                          bool user_gesture) override;
    
    CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                      CefRefPtr<CefFrame> frame,
                                                      CefRefPtr<CefRequest> request) override;
    
    void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefRequest> request,
                               CefRefPtr<CefResponse> response) override;
    
    // 8KDVD specific request handling
    bool Handle8KDVDRequest(const std::string& url, CefRefPtr<CefResponse> response);
    bool Is8KDVDResource(const std::string& url);
    std::string Get8KDVDResourcePath(const std::string& url);
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetResourceBasePath(const std::string& base_path);
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    std::string resource_base_path_;
    
    // 8KDVD state
    std::string current_disc_path_;
    bool disc_loaded_;
    
    // Helper methods
    bool HandleLocalFileRequest(const std::string& url, CefRefPtr<CefResponse> response);
    bool Handle8KDVDMenuRequest(const std::string& url, CefRefPtr<CefResponse> response);
    bool Handle8KDVDResourceRequest(const std::string& url, CefRefPtr<CefResponse> response);
    std::string GetMimeType(const std::string& filename);
    
    IMPLEMENT_REFCOUNTING(VLCCefRequestHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefRequestHandler);
};

#endif // VLC_CEF_REQUEST_HANDLER_H
