#include "cef_request_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <fstream>
#include <sstream>
#include <algorithm>

VLCCefRequestHandler::VLCCefRequestHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), disc_loaded_(false) {
    
    msg_Info(vlc_obj_, "CEF request handler created");
}

VLCCefRequestHandler::~VLCCefRequestHandler() {
    msg_Info(vlc_obj_, "CEF request handler destroyed");
}

bool VLCCefRequestHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefRequest> request,
                                         bool user_gesture,
                                         bool is_redirect) {
    CEF_REQUIRE_UI_THREAD();
    
    std::string url = request->GetURL().ToString();
    msg_Dbg(vlc_obj_, "CEF before browse: %s", url.c_str());
    
    // Handle 8KDVD navigation
    if (Is8KDVDResource(url)) {
        msg_Info(vlc_obj_, "8KDVD navigation: %s", url.c_str());
        return false; // Allow navigation
    }
    
    return false; // Allow all navigation
}

bool VLCCefRequestHandler::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          const CefString& target_url,
                                          WindowOpenDisposition target_disposition,
                                          bool user_gesture) {
    CEF_REQUIRE_UI_THREAD();
    
    std::string url = target_url.ToString();
    msg_Dbg(vlc_obj_, "CEF open URL from tab: %s", url.c_str());
    
    // Handle 8KDVD tab opening
    if (Is8KDVDResource(url)) {
        msg_Info(vlc_obj_, "8KDVD tab open: %s", url.c_str());
        return false; // Allow opening
    }
    
    return false; // Allow all tab opening
}

CefRefPtr<CefResourceHandler> VLCCefRequestHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                                      CefRefPtr<CefFrame> frame,
                                                                      CefRefPtr<CefRequest> request) {
    CEF_REQUIRE_IO_THREAD();
    
    std::string url = request->GetURL().ToString();
    msg_Dbg(vlc_obj_, "CEF resource handler: %s", url.c_str());
    
    // Handle 8KDVD resources
    if (Is8KDVDResource(url)) {
        return nullptr; // Use default handler for now
    }
    
    return nullptr; // Use default handler
}

void VLCCefRequestHandler::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefRequest> request,
                                                 CefRefPtr<CefResponse> response) {
    CEF_REQUIRE_IO_THREAD();
    
    std::string url = request->GetURL().ToString();
    int status_code = response->GetStatus();
    
    msg_Dbg(vlc_obj_, "CEF resource load complete: %s (status: %d)", url.c_str(), status_code);
    
    // Handle 8KDVD resource load completion
    if (Is8KDVDResource(url)) {
        msg_Info(vlc_obj_, "8KDVD resource loaded: %s (status: %d)", url.c_str(), status_code);
    }
}

bool VLCCefRequestHandler::Handle8KDVDRequest(const std::string& url, CefRefPtr<CefResponse> response) {
    if (!Is8KDVDResource(url)) return false;
    
    std::string resource_path = Get8KDVDResourcePath(url);
    msg_Info(vlc_obj_, "8KDVD request: %s -> %s", url.c_str(), resource_path.c_str());
    
    // TODO: Implement actual 8KDVD resource handling
    // This would serve resources from the 8KDVD disc
    
    return true;
}

bool VLCCefRequestHandler::Is8KDVDResource(const std::string& url) {
    // Check if URL is an 8KDVD resource
    return (url.find("8kdvd") != std::string::npos ||
            url.find("8KDVD") != std::string::npos ||
            url.find("evo8") != std::string::npos ||
            url.find("menu") != std::string::npos);
}

std::string VLCCefRequestHandler::Get8KDVDResourcePath(const std::string& url) {
    // Extract resource path from URL
    size_t protocol_pos = url.find("://");
    if (protocol_pos != std::string::npos) {
        return url.substr(protocol_pos + 3);
    }
    return url;
}

void VLCCefRequestHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefRequestHandler::SetResourceBasePath(const std::string& base_path) {
    resource_base_path_ = base_path;
    msg_Info(vlc_obj_, "Resource base path set: %s", base_path.c_str());
}

bool VLCCefRequestHandler::HandleLocalFileRequest(const std::string& url, CefRefPtr<CefResponse> response) {
    // Handle local file requests
    if (url.find("file://") == 0) {
        std::string file_path = url.substr(7); // Remove "file://"
        msg_Dbg(vlc_obj_, "Local file request: %s", file_path.c_str());
        return true;
    }
    return false;
}

bool VLCCefRequestHandler::Handle8KDVDMenuRequest(const std::string& url, CefRefPtr<CefResponse> response) {
    // Handle 8KDVD menu requests
    if (url.find("menu") != std::string::npos) {
        msg_Info(vlc_obj_, "8KDVD menu request: %s", url.c_str());
        return true;
    }
    return false;
}

bool VLCCefRequestHandler::Handle8KDVDResourceRequest(const std::string& url, CefRefPtr<CefResponse> response) {
    // Handle 8KDVD resource requests
    if (Is8KDVDResource(url)) {
        msg_Info(vlc_obj_, "8KDVD resource request: %s", url.c_str());
        return true;
    }
    return false;
}

std::string VLCCefRequestHandler::GetMimeType(const std::string& filename) {
    // Get MIME type based on file extension
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string extension = filename.substr(dot_pos + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        if (extension == "html" || extension == "htm") return "text/html";
        if (extension == "css") return "text/css";
        if (extension == "js") return "application/javascript";
        if (extension == "json") return "application/json";
        if (extension == "png") return "image/png";
        if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
        if (extension == "gif") return "image/gif";
        if (extension == "svg") return "image/svg+xml";
        if (extension == "ico") return "image/x-icon";
        if (extension == "woff") return "font/woff";
        if (extension == "woff2") return "font/woff2";
        if (extension == "ttf") return "font/ttf";
        if (extension == "otf") return "font/otf";
    }
    
    return "application/octet-stream";
}
