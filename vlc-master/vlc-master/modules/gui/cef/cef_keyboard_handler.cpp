#include "cef_keyboard_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <sstream>

VLCCefKeyboardHandler::VLCCefKeyboardHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), keyboard_shortcuts_enabled_(true),
      disc_loaded_(false) {
    
    Register8KDVDKeyBindings();
    msg_Info(vlc_obj_, "CEF keyboard handler created");
}

VLCCefKeyboardHandler::~VLCCefKeyboardHandler() {
    key_bindings_.clear();
    msg_Info(vlc_obj_, "CEF keyboard handler destroyed");
}

bool VLCCefKeyboardHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                                          const CefKeyEvent& event,
                                          CefEventHandle os_event,
                                          bool* is_keyboard_shortcut) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!keyboard_shortcuts_enabled_) {
        return false;
    }
    
    LogKeyEvent(event);
    
    // Handle VLC key events
    if (HandleVLCKeyEvent(event)) {
        *is_keyboard_shortcut = true;
        return true;
    }
    
    // Handle 8KDVD key events
    if (Handle8KDVDKeyEvent(event)) {
        *is_keyboard_shortcut = true;
        return true;
    }
    
    return false; // Use default handling
}

bool VLCCefKeyboardHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser,
                                        const CefKeyEvent& event,
                                        CefEventHandle os_event) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!keyboard_shortcuts_enabled_) {
        return false;
    }
    
    // Handle key release events
    if (event.type == KEYEVENT_KEYUP) {
        LogKeyEvent(event);
        return false; // Use default handling
    }
    
    return false; // Use default handling
}

bool VLCCefKeyboardHandler::Handle8KDVDKeyEvent(const CefKeyEvent& event) {
    if (!disc_loaded_) return false;
    
    // Handle 8KDVD navigation keys
    if (Handle8KDVDNavigationKey(event)) {
        return true;
    }
    
    // Handle 8KDVD control keys
    if (Handle8KDVDControlKey(event)) {
        return true;
    }
    
    return false;
}

void VLCCefKeyboardHandler::Register8KDVDKeyBindings() {
    // Register 8KDVD key bindings
    RegisterKeyBinding(KEY_8KDVD_UP, "navigate_up");
    RegisterKeyBinding(KEY_8KDVD_DOWN, "navigate_down");
    RegisterKeyBinding(KEY_8KDVD_LEFT, "navigate_left");
    RegisterKeyBinding(KEY_8KDVD_RIGHT, "navigate_right");
    RegisterKeyBinding(KEY_8KDVD_ENTER, "select");
    RegisterKeyBinding(KEY_8KDVD_ESCAPE, "back");
    RegisterKeyBinding(KEY_8KDVD_SPACE, "play_pause");
    RegisterKeyBinding(KEY_8KDVD_TAB, "next_menu");
    
    msg_Info(vlc_obj_, "8KDVD key bindings registered");
}

void VLCCefKeyboardHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefKeyboardHandler::SetKeyboardShortcutsEnabled(bool enable) {
    keyboard_shortcuts_enabled_ = enable;
    msg_Info(vlc_obj_, "Keyboard shortcuts %s", enable ? "enabled" : "disabled");
}

void VLCCefKeyboardHandler::RegisterKeyBinding(int key_code, const std::string& action) {
    key_bindings_[key_code] = action;
    msg_Dbg(vlc_obj_, "Key binding registered: %d -> %s", key_code, action.c_str());
}

void VLCCefKeyboardHandler::UnregisterKeyBinding(int key_code) {
    key_bindings_.erase(key_code);
    msg_Dbg(vlc_obj_, "Key binding unregistered: %d", key_code);
}

std::string VLCCefKeyboardHandler::GetKeyBindingAction(int key_code) {
    auto it = key_bindings_.find(key_code);
    if (it != key_bindings_.end()) {
        return it->second;
    }
    return "";
}

bool VLCCefKeyboardHandler::HandleVLCKeyEvent(const CefKeyEvent& event) {
    // Handle VLC-specific key events
    if (event.type == KEYEVENT_KEYDOWN) {
        std::string action = GetKeyBindingAction(event.windows_key_code);
        if (!action.empty()) {
            msg_Info(vlc_obj_, "VLC key event: %s", action.c_str());
            return true;
        }
    }
    
    return false;
}

bool VLCCefKeyboardHandler::Handle8KDVDNavigationKey(const CefKeyEvent& event) {
    if (event.type != KEYEVENT_KEYDOWN) return false;
    
    switch (event.windows_key_code) {
        case KEY_8KDVD_UP:
            msg_Info(vlc_obj_, "8KDVD: Navigate up");
            return true;
            
        case KEY_8KDVD_DOWN:
            msg_Info(vlc_obj_, "8KDVD: Navigate down");
            return true;
            
        case KEY_8KDVD_LEFT:
            msg_Info(vlc_obj_, "8KDVD: Navigate left");
            return true;
            
        case KEY_8KDVD_RIGHT:
            msg_Info(vlc_obj_, "8KDVD: Navigate right");
            return true;
            
        default:
            return false;
    }
}

bool VLCCefKeyboardHandler::Handle8KDVDControlKey(const CefKeyEvent& event) {
    if (event.type != KEYEVENT_KEYDOWN) return false;
    
    switch (event.windows_key_code) {
        case KEY_8KDVD_ENTER:
            msg_Info(vlc_obj_, "8KDVD: Select");
            return true;
            
        case KEY_8KDVD_ESCAPE:
            msg_Info(vlc_obj_, "8KDVD: Back");
            return true;
            
        case KEY_8KDVD_SPACE:
            msg_Info(vlc_obj_, "8KDVD: Play/Pause");
            return true;
            
        case KEY_8KDVD_TAB:
            msg_Info(vlc_obj_, "8KDVD: Next menu");
            return true;
            
        default:
            return false;
    }
}

std::string VLCCefKeyboardHandler::GetKeyName(int key_code) {
    switch (key_code) {
        case KEY_8KDVD_UP: return "Up";
        case KEY_8KDVD_DOWN: return "Down";
        case KEY_8KDVD_LEFT: return "Left";
        case KEY_8KDVD_RIGHT: return "Right";
        case KEY_8KDVD_ENTER: return "Enter";
        case KEY_8KDVD_ESCAPE: return "Escape";
        case KEY_8KDVD_SPACE: return "Space";
        case KEY_8KDVD_TAB: return "Tab";
        default: return "Unknown";
    }
}

void VLCCefKeyboardHandler::LogKeyEvent(const CefKeyEvent& event) {
    std::string key_name = GetKeyName(event.windows_key_code);
    std::string event_type = (event.type == KEYEVENT_KEYDOWN) ? "DOWN" : "UP";
    
    msg_Dbg(vlc_obj_, "CEF key event: %s %s (code: %d)", 
           key_name.c_str(), event_type.c_str(), event.windows_key_code);
}
