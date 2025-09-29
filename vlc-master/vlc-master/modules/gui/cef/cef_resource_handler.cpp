#include "cef_resource_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <filesystem>
#include <algorithm>

VLCCefResourceHandler::VLCCefResourceHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), resource_cache_enabled_(true),
      disc_loaded_(false), file_size_(0), bytes_read_(0) {
    
    msg_Info(vlc_obj_, "CEF resource handler created");
}

VLCCefResourceHandler::~VLCCefResourceHandler() {
    CloseResourceFile();
    msg_Info(vlc_obj_, "CEF resource handler destroyed");
}

bool VLCCefResourceHandler::ProcessRequest(CefRefPtr<CefRequest> request,
                                          CefRefPtr<CefCallback> callback) {
    CEF_REQUIRE_IO_THREAD();
    
    std::string url = request->GetURL().ToString();
    current_url_ = url;
    
    msg_Dbg(vlc_obj_, "CEF resource request: %s", url.c_str());
    
    // Handle 8KDVD resources
    if (Is8KDVDResource(url)) {
        std::string resource_path = Get8KDVDResourcePath(url);
        if (OpenResourceFile(resource_path)) {
            callback->Continue();
            return true;
        }
    }
    
    // Handle local file resources
    if (url.find("file://") == 0) {
        std::string file_path = url.substr(7); // Remove "file://"
        if (OpenResourceFile(file_path)) {
            callback->Continue();
            return true;
        }
    }
    
    // Resource not found
    callback->Continue();
    return false;
}

void VLCCefResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
                                               int64& response_length,
                                               CefString& redirectUrl) {
    CEF_REQUIRE_IO_THREAD();
    
    if (file_stream_.is_open()) {
        response->SetStatus(200);
        response->SetStatusText("OK");
        response->SetMimeType(GetMimeType(current_file_path_));
        response_length = file_size_;
        
        msg_Dbg(vlc_obj_, "CEF resource response: %s (%lld bytes)", 
               current_file_path_.c_str(), file_size_);
    } else {
        response->SetStatus(404);
        response->SetStatusText("Not Found");
        response_length = 0;
        
        msg_Err(vlc_obj_, "CEF resource not found: %s", current_url_.c_str());
    }
}

bool VLCCefResourceHandler::ReadResponse(void* data_out,
                                         int bytes_to_read,
                                         int& bytes_read,
                                         CefRefPtr<CefCallback> callback) {
    CEF_REQUIRE_IO_THREAD();
    
    if (!file_stream_.is_open()) {
        bytes_read = 0;
        return false;
    }
    
    file_stream_.read(static_cast<char*>(data_out), bytes_to_read);
    bytes_read = static_cast<int>(file_stream_.gcount());
    bytes_read_ += bytes_read;
    
    if (bytes_read == 0) {
        CloseResourceFile();
        msg_Dbg(vlc_obj_, "CEF resource read complete: %s", current_file_path_.c_str());
    }
    
    return true;
}

void VLCCefResourceHandler::Cancel() {
    CEF_REQUIRE_IO_THREAD();
    
    CloseResourceFile();
    msg_Dbg(vlc_obj_, "CEF resource request cancelled: %s", current_url_.c_str());
}

bool VLCCefResourceHandler::Handle8KDVDResource(const std::string& url, CefRefPtr<CefResponse> response) {
    if (!Is8KDVDResource(url)) return false;
    
    std::string resource_path = Get8KDVDResourcePath(url);
    msg_Info(vlc_obj_, "8KDVD resource request: %s -> %s", url.c_str(), resource_path.c_str());
    
    // TODO: Implement actual 8KDVD resource handling
    // This would serve resources from the 8KDVD disc
    
    return true;
}

bool VLCCefResourceHandler::Is8KDVDResource(const std::string& url) {
    // Check if URL is an 8KDVD resource
    return (url.find("8kdvd") != std::string::npos ||
            url.find("8KDVD") != std::string::npos ||
            url.find("evo8") != std::string::npos ||
            url.find("menu") != std::string::npos);
}

std::string VLCCefResourceHandler::Get8KDVDResourcePath(const std::string& url) {
    // Extract resource path from URL
    size_t protocol_pos = url.find("://");
    if (protocol_pos != std::string::npos) {
        return url.substr(protocol_pos + 3);
    }
    return url;
}

void VLCCefResourceHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefResourceHandler::SetResourceBasePath(const std::string& base_path) {
    resource_base_path_ = base_path;
    msg_Info(vlc_obj_, "Resource base path set: %s", base_path.c_str());
}

void VLCCefResourceHandler::SetResourceCacheEnabled(bool enable) {
    resource_cache_enabled_ = enable;
    msg_Info(vlc_obj_, "Resource cache %s", enable ? "enabled" : "disabled");
}

void VLCCefResourceHandler::ClearResourceCache() {
    // TODO: Implement resource cache clearing
    msg_Info(vlc_obj_, "Resource cache cleared");
}

bool VLCCefResourceHandler::HandleLocalFileResource(const std::string& url, CefRefPtr<CefResponse> response) {
    // Handle local file requests
    if (url.find("file://") == 0) {
        std::string file_path = url.substr(7); // Remove "file://"
        msg_Dbg(vlc_obj_, "Local file request: %s", file_path.c_str());
        return true;
    }
    return false;
}

bool VLCCefResourceHandler::Handle8KDVDMenuResource(const std::string& url, CefRefPtr<CefResponse> response) {
    // Handle 8KDVD menu resources
    if (url.find("menu") != std::string::npos) {
        msg_Info(vlc_obj_, "8KDVD menu resource: %s", url.c_str());
        return true;
    }
    return false;
}

bool VLCCefResourceHandler::Handle8KDVDMediaResource(const std::string& url, CefRefPtr<CefResponse> response) {
    // Handle 8KDVD media resources
    if (url.find("evo8") != std::string::npos) {
        msg_Info(vlc_obj_, "8KDVD media resource: %s", url.c_str());
        return true;
    }
    return false;
}

std::string VLCCefResourceHandler::GetMimeType(const std::string& filename) {
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

bool VLCCefResourceHandler::OpenResourceFile(const std::string& file_path) {
    CloseResourceFile();
    
    current_file_path_ = file_path;
    
    // Check if file exists
    if (!std::filesystem::exists(file_path)) {
        msg_Err(vlc_obj_, "Resource file not found: %s", file_path.c_str());
        return false;
    }
    
    // Open file
    file_stream_.open(file_path, std::ios::binary);
    if (!file_stream_.is_open()) {
        msg_Err(vlc_obj_, "Failed to open resource file: %s", file_path.c_str());
        return false;
    }
    
    // Get file size
    file_stream_.seekg(0, std::ios::end);
    file_size_ = file_stream_.tellg();
    file_stream_.seekg(0, std::ios::beg);
    bytes_read_ = 0;
    
    msg_Dbg(vlc_obj_, "Resource file opened: %s (%lld bytes)", file_path.c_str(), file_size_);
    return true;
}

void VLCCefResourceHandler::CloseResourceFile() {
    if (file_stream_.is_open()) {
        file_stream_.close();
        msg_Dbg(vlc_obj_, "Resource file closed: %s", current_file_path_.c_str());
    }
    current_file_path_.clear();
    file_size_ = 0;
    bytes_read_ = 0;
}
