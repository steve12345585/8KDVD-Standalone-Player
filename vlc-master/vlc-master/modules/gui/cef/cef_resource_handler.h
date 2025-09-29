#ifndef VLC_CEF_RESOURCE_HANDLER_H
#define VLC_CEF_RESOURCE_HANDLER_H

#include "include/cef_resource_handler.h"
#include "include/cef_request.h"
#include "include/cef_response.h"
#include "include/cef_callback.h"
#include <vlc_common.h>
#include <string>
#include <fstream>

// CEF Resource Handler for VLC Integration
class VLCCefResourceHandler : public CefResourceHandler {
public:
    VLCCefResourceHandler(vlc_object_t *obj);
    ~VLCCefResourceHandler();
    
    // CefResourceHandler methods
    bool ProcessRequest(CefRefPtr<CefRequest> request,
                       CefRefPtr<CefCallback> callback) override;
    
    void GetResponseHeaders(CefRefPtr<CefResponse> response,
                           int64& response_length,
                           CefString& redirectUrl) override;
    
    bool ReadResponse(void* data_out,
                      int bytes_to_read,
                      int& bytes_read,
                      CefRefPtr<CefCallback> callback) override;
    
    void Cancel() override;
    
    // 8KDVD specific resource handling
    bool Handle8KDVDResource(const std::string& url, CefRefPtr<CefResponse> response);
    bool Is8KDVDResource(const std::string& url);
    std::string Get8KDVDResourcePath(const std::string& url);
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetResourceBasePath(const std::string& base_path);
    
    // Resource management
    void SetResourceCacheEnabled(bool enable);
    void ClearResourceCache();
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    std::string resource_base_path_;
    bool resource_cache_enabled_;
    
    // 8KDVD state
    bool disc_loaded_;
    std::string current_disc_path_;
    
    // Resource handling
    std::string current_url_;
    std::string current_file_path_;
    std::ifstream file_stream_;
    int64 file_size_;
    int64 bytes_read_;
    
    // Helper methods
    bool HandleLocalFileResource(const std::string& url, CefRefPtr<CefResponse> response);
    bool Handle8KDVDMenuResource(const std::string& url, CefRefPtr<CefResponse> response);
    bool Handle8KDVDMediaResource(const std::string& url, CefRefPtr<CefResponse> response);
    std::string GetMimeType(const std::string& filename);
    bool OpenResourceFile(const std::string& file_path);
    void CloseResourceFile();
    
    IMPLEMENT_REFCOUNTING(VLCCefResourceHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefResourceHandler);
};

#endif // VLC_CEF_RESOURCE_HANDLER_H
