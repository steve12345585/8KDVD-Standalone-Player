#ifndef VLC_CEF_KEYBOARD_HANDLER_H
#define VLC_CEF_KEYBOARD_HANDLER_H

#include "include/cef_keyboard_handler.h"
#include "include/cef_browser.h"
#include "include/cef_key_event.h"
#include <vlc_common.h>
#include <string>
#include <map>

// CEF Keyboard Handler for VLC Integration
class VLCCefKeyboardHandler : public CefKeyboardHandler {
public:
    VLCCefKeyboardHandler(vlc_object_t *obj);
    ~VLCCefKeyboardHandler();
    
    // CefKeyboardHandler methods
    bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                       const CefKeyEvent& event,
                       CefEventHandle os_event,
                       bool* is_keyboard_shortcut) override;
    
    bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
                    const CefKeyEvent& event,
                    CefEventHandle os_event) override;
    
    // 8KDVD specific keyboard handling
    bool Handle8KDVDKeyEvent(const CefKeyEvent& event);
    void Register8KDVDKeyBindings();
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetKeyboardShortcutsEnabled(bool enable);
    
    // Key binding management
    void RegisterKeyBinding(int key_code, const std::string& action);
    void UnregisterKeyBinding(int key_code);
    std::string GetKeyBindingAction(int key_code);
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    bool keyboard_shortcuts_enabled_;
    
    // 8KDVD state
    bool disc_loaded_;
    std::string current_menu_;
    
    // Key bindings
    std::map<int, std::string> key_bindings_;
    
    // 8KDVD key codes
    enum KeyCodes {
        KEY_8KDVD_UP = 0x26,      // Arrow Up
        KEY_8KDVD_DOWN = 0x28,    // Arrow Down
        KEY_8KDVD_LEFT = 0x25,    // Arrow Left
        KEY_8KDVD_RIGHT = 0x27,   // Arrow Right
        KEY_8KDVD_ENTER = 0x0D,   // Enter
        KEY_8KDVD_ESCAPE = 0x1B,  // Escape
        KEY_8KDVD_SPACE = 0x20,   // Space
        KEY_8KDVD_TAB = 0x09      // Tab
    };
    
    // Helper methods
    bool HandleVLCKeyEvent(const CefKeyEvent& event);
    bool Handle8KDVDNavigationKey(const CefKeyEvent& event);
    bool Handle8KDVDControlKey(const CefKeyEvent& event);
    std::string GetKeyName(int key_code);
    void LogKeyEvent(const CefKeyEvent& event);
    
    IMPLEMENT_REFCOUNTING(VLCCefKeyboardHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefKeyboardHandler);
};

#endif // VLC_CEF_KEYBOARD_HANDLER_H
