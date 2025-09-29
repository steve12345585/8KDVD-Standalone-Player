#include "cef_display_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <iostream>

VLCCefDisplayHandler::VLCCefDisplayHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), debug_output_enabled_(true),
      fullscreen_mode_(false), current_width_(1920), current_height_(1080) {
    
    msg_Info(vlc_obj_, "CEF display handler created");
}

VLCCefDisplayHandler::~VLCCefDisplayHandler() {
    msg_Info(vlc_obj_, "CEF display handler destroyed");
}

void VLCCefDisplayHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
    CEF_REQUIRE_UI_THREAD();
    
    std::string title_str = title.ToString();
    msg_Info(vlc_obj_, "CEF title changed: %s", title_str.c_str());
    
    // Handle 8KDVD menu title changes
    if (title_str.find("8KDVD") != std::string::npos) {
        On8KDVDMenuLoaded(title_str);
    }
}

void VLCCefDisplayHandler::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!icon_urls.empty()) {
        std::string icon_url = icon_urls[0].ToString();
        msg_Dbg(vlc_obj_, "CEF favicon changed: %s", icon_url.c_str());
    }
}

void VLCCefDisplayHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) {
    CEF_REQUIRE_UI_THREAD();
    
    fullscreen_mode_ = fullscreen;
    msg_Info(vlc_obj_, "CEF fullscreen mode changed: %s", fullscreen ? "enabled" : "disabled");
    
    // Handle 8KDVD fullscreen mode
    if (vlc_integration_enabled_) {
        Handle8KDVDDisplayEvent("fullscreen_change", fullscreen ? "true" : "false");
    }
}

bool VLCCefDisplayHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                           cef_log_severity_t level,
                                           const CefString& message,
                                           const CefString& source,
                                           int line) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!debug_output_enabled_) return false;
    
    std::string message_str = message.ToString();
    std::string source_str = source.ToString();
    
    // Log CEF console messages
    LogCEFMessage(level, message_str);
    
    // Handle 8KDVD specific console messages
    if (message_str.find("8KDVD") != std::string::npos) {
        Handle8KDVDDisplayEvent("console_message", message_str);
    }
    
    return false; // Allow default handling
}

void VLCCefDisplayHandler::On8KDVDMenuLoaded(const std::string& menu_name) {
    current_menu_ = menu_name;
    msg_Info(vlc_obj_, "8KDVD menu loaded: %s", menu_name.c_str());
    
    if (vlc_integration_enabled_) {
        Handle8KDVDDisplayEvent("menu_loaded", menu_name);
    }
}

void VLCCefDisplayHandler::On8KDVDMenuError(const std::string& error_message) {
    msg_Err(vlc_obj_, "8KDVD menu error: %s", error_message.c_str());
    
    if (vlc_integration_enabled_) {
        Handle8KDVDDisplayEvent("menu_error", error_message);
    }
}

void VLCCefDisplayHandler::On8KDVDResolutionChange(int width, int height) {
    current_width_ = width;
    current_height_ = height;
    msg_Info(vlc_obj_, "8KDVD resolution changed: %dx%d", width, height);
    
    if (vlc_integration_enabled_) {
        std::string resolution = std::to_string(width) + "x" + std::to_string(height);
        Handle8KDVDDisplayEvent("resolution_change", resolution);
    }
}

void VLCCefDisplayHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefDisplayHandler::SetDebugOutput(bool enable) {
    debug_output_enabled_ = enable;
    msg_Info(vlc_obj_, "Debug output %s", enable ? "enabled" : "disabled");
}

void VLCCefDisplayHandler::LogCEFMessage(cef_log_severity_t level, const std::string& message) {
    switch (level) {
        case LOGSEVERITY_DEBUG:
            msg_Dbg(vlc_obj_, "CEF Debug: %s", message.c_str());
            break;
        case LOGSEVERITY_INFO:
            msg_Info(vlc_obj_, "CEF Info: %s", message.c_str());
            break;
        case LOGSEVERITY_WARNING:
            msg_Warn(vlc_obj_, "CEF Warning: %s", message.c_str());
            break;
        case LOGSEVERITY_ERROR:
            msg_Err(vlc_obj_, "CEF Error: %s", message.c_str());
            break;
        case LOGSEVERITY_FATAL:
            msg_Err(vlc_obj_, "CEF Fatal: %s", message.c_str());
            break;
        default:
            msg_Info(vlc_obj_, "CEF: %s", message.c_str());
            break;
    }
}

void VLCCefDisplayHandler::Handle8KDVDDisplayEvent(const std::string& event_type, const std::string& data) {
    msg_Dbg(vlc_obj_, "8KDVD display event: %s = %s", event_type.c_str(), data.c_str());
    
    // TODO: Implement specific 8KDVD display event handling
    // This would integrate with VLC's display system
}

// Window resizing
void VLCCefDisplayHandler::OnWindowResize(int width, int height) {
    current_width_ = width;
    current_height_ = height;
    msg_Info(vlc_obj_, "8KDVD window resized: %dx%d", width, height);
    
    if (vlc_integration_enabled_) {
        Handle8KDVDDisplayEvent("window_resize", std::to_string(width) + "x" + std::to_string(height));
    }
}

// Fullscreen support
void VLCCefDisplayHandler::OnFullscreenToggle(bool fullscreen) {
    fullscreen_mode_ = fullscreen;
    msg_Info(vlc_obj_, "8KDVD fullscreen %s", fullscreen ? "enabled" : "disabled");
    
    if (vlc_integration_enabled_) {
        Handle8KDVDDisplayEvent("fullscreen_toggle", fullscreen ? "true" : "false");
    }
}

// Display changes
void VLCCefDisplayHandler::OnDisplayChange(int display_width, int display_height) {
    msg_Info(vlc_obj_, "8KDVD display changed: %dx%d", display_width, display_height);
    
    if (vlc_integration_enabled_) {
        std::string resolution = std::to_string(display_width) + "x" + std::to_string(display_height);
        Handle8KDVDDisplayEvent("display_change", resolution);
    }
}
