#include "cef_message_router.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <vlc_playlist.h>
#include <vlc_input.h>
#include <sstream>

VLCCefMessageRouter::VLCCefMessageRouter(vlc_object_t *obj) : vlc_obj_(obj) {
    Register8KDVDHandlers();
}

VLCCefMessageRouter::~VLCCefMessageRouter() {
    handlers_.clear();
}

void VLCCefMessageRouter::RegisterHandler(const std::string& message_name, MessageHandler handler) {
    handlers_[message_name] = handler;
    msg_Dbg(vlc_obj_, "CEF message handler registered: %s", message_name.c_str());
}

void VLCCefMessageRouter::UnregisterHandler(const std::string& message_name) {
    handlers_.erase(message_name);
    msg_Dbg(vlc_obj_, "CEF message handler unregistered: %s", message_name.c_str());
}

bool VLCCefMessageRouter::SendMessageToCEF(CefRefPtr<CefBrowser> browser, 
                                          const std::string& message_name,
                                          const std::string& data) {
    if (!browser) return false;
    
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(message_name);
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    args->SetString(0, data);
    
    bool success = browser->SendProcessMessage(PID_RENDERER, message);
    msg_Dbg(vlc_obj_, "CEF message sent: %s (success: %s)", 
           message_name.c_str(), success ? "true" : "false");
    
    return success;
}

bool VLCCefMessageRouter::HandleCEFMessage(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefProcessMessage> message) {
    const std::string& message_name = message->GetName();
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    
    std::string data;
    if (args->GetSize() > 0) {
        data = args->GetString(0).ToString();
    }
    
    auto it = handlers_.find(message_name);
    if (it != handlers_.end()) {
        it->second(message_name, data);
        return true;
    }
    
    msg_Warn(vlc_obj_, "CEF message handler not found: %s", message_name.c_str());
    return false;
}

void VLCCefMessageRouter::Register8KDVDHandlers() {
    RegisterHandler("8kdvd_play_title", 
        [this](const std::string& name, const std::string& data) {
            HandlePlayTitle(name, data);
        });
    
    RegisterHandler("8kdvd_show_menu", 
        [this](const std::string& name, const std::string& data) {
            HandleShowMenu(name, data);
        });
    
    RegisterHandler("8kdvd_navigate_menu", 
        [this](const std::string& name, const std::string& data) {
            HandleNavigateMenu(name, data);
        });
    
    RegisterHandler("8kdvd_volume_change", 
        [this](const std::string& name, const std::string& data) {
            HandleVolumeChange(name, data);
        });
    
    RegisterHandler("8kdvd_seek_to", 
        [this](const std::string& name, const std::string& data) {
            HandleSeekTo(name, data);
        });
}

void VLCCefMessageRouter::HandlePlayTitle(const std::string& message_name, const std::string& data) {
    try {
        int title_id = std::stoi(data);
        msg_Info(vlc_obj_, "8KDVD: Playing title %d", title_id);
        
        // TODO: Implement actual title playback
        // This would integrate with VLC's playlist system
        
    } catch (const std::exception& e) {
        msg_Err(vlc_obj_, "8KDVD: Invalid title ID: %s", data.c_str());
    }
}

void VLCCefMessageRouter::HandleShowMenu(const std::string& message_name, const std::string& data) {
    msg_Info(vlc_obj_, "8KDVD: Showing menu: %s", data.c_str());
    
    // TODO: Implement menu display logic
    // This would control which HTML5 menu is shown
}

void VLCCefMessageRouter::HandleNavigateMenu(const std::string& message_name, const std::string& data) {
    msg_Info(vlc_obj_, "8KDVD: Navigate menu: %s", data.c_str());
    
    // TODO: Implement menu navigation
    // This would handle menu button navigation
}

void VLCCefMessageRouter::HandleVolumeChange(const std::string& message_name, const std::string& data) {
    try {
        int volume = std::stoi(data);
        msg_Info(vlc_obj_, "8KDVD: Volume change to %d", volume);
        
        // TODO: Implement volume control
        // This would integrate with VLC's audio system
        
    } catch (const std::exception& e) {
        msg_Err(vlc_obj_, "8KDVD: Invalid volume: %s", data.c_str());
    }
}

void VLCCefMessageRouter::HandleSeekTo(const std::string& message_name, const std::string& data) {
    try {
        int position = std::stoi(data);
        msg_Info(vlc_obj_, "8KDVD: Seek to position %d", position);
        
        // TODO: Implement seek functionality
        // This would integrate with VLC's playback control
        
    } catch (const std::exception& e) {
        msg_Err(vlc_obj_, "8KDVD: Invalid position: %s", data.c_str());
    }
}

void VLCCefMessageRouter::SendPlayTitle(int title_id) {
    std::stringstream ss;
    ss << title_id;
    SendMessageToCEF(nullptr, "vlc_play_title", ss.str());
}

void VLCCefMessageRouter::SendShowMenu(const std::string& menu_type) {
    SendMessageToCEF(nullptr, "vlc_show_menu", menu_type);
}

void VLCCefMessageRouter::SendNavigateMenu(const std::string& direction) {
    SendMessageToCEF(nullptr, "vlc_navigate_menu", direction);
}

void VLCCefMessageRouter::SendVolumeChange(int volume) {
    std::stringstream ss;
    ss << volume;
    SendMessageToCEF(nullptr, "vlc_volume_change", ss.str());
}

void VLCCefMessageRouter::SendSeekTo(int position) {
    std::stringstream ss;
    ss << position;
    SendMessageToCEF(nullptr, "vlc_seek_to", ss.str());
}

// JavaScript → C++ message passing
bool VLCCefMessageRouter::SendJavaScriptMessage(CefRefPtr<CefBrowser> browser, 
                                                const std::string& function_name,
                                                const std::string& data) {
    if (!browser) return false;
    
    // Execute JavaScript function in the browser
    std::string js_code = "if (window." + function_name + ") { window." + function_name + "('" + data + "'); }";
    browser->GetMainFrame()->ExecuteJavaScript(js_code, "", 0);
    
    msg_Dbg(vlc_obj_, "JavaScript message sent: %s('%s')", function_name.c_str(), data.c_str());
    return true;
}

// C++ → JavaScript message passing
bool VLCCefMessageRouter::SendCppToJavaScript(CefRefPtr<CefBrowser> browser,
                                             const std::string& message_type,
                                             const std::string& data) {
    if (!browser) return false;
    
    // Send message to JavaScript via CEF's message system
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cpp_to_js");
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    args->SetString(0, message_type);
    args->SetString(1, data);
    
    bool success = browser->SendProcessMessage(PID_RENDERER, message);
    msg_Dbg(vlc_obj_, "C++ to JavaScript message: %s = %s (success: %s)", 
           message_type.c_str(), data.c_str(), success ? "true" : "false");
    
    return success;
}

// Asynchronous message processing
void VLCCefMessageRouter::ProcessAsyncMessages() {
    // Process any pending asynchronous messages
    // This would be called from VLC's main loop
    msg_Dbg(vlc_obj_, "Processing asynchronous CEF messages");
}
