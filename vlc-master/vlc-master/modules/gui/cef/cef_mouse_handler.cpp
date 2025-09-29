#include "cef_mouse_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>

VLCCefMouseHandler::VLCCefMouseHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), mouse_events_enabled_(true),
      disc_loaded_(false), mouse_x_(0), mouse_y_(0), mouse_over_menu_(false) {
    
    msg_Info(vlc_obj_, "CEF mouse handler created");
}

VLCCefMouseHandler::~VLCCefMouseHandler() {
    msg_Info(vlc_obj_, "CEF mouse handler destroyed");
}

bool VLCCefMouseHandler::OnMouseClick(CefRefPtr<CefBrowser> browser,
                                     const CefMouseEvent& event,
                                     MouseButtonType type,
                                     bool mouseUp,
                                     int clickCount) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!mouse_events_enabled_) {
        return false;
    }
    
    UpdateMouseState(event);
    
    // Handle VLC mouse events
    if (HandleVLCMouseEvent(event, type, mouseUp)) {
        return true;
    }
    
    // Handle 8KDVD mouse events
    if (Handle8KDVDMouseEvent(event, type, mouseUp)) {
        return true;
    }
    
    return false; // Use default handling
}

bool VLCCefMouseHandler::OnMouseMove(CefRefPtr<CefBrowser> browser,
                                     const CefMouseEvent& event,
                                     bool mouseLeave) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!mouse_events_enabled_) {
        return false;
    }
    
    UpdateMouseState(event);
    
    // Handle 8KDVD mouse move
    Handle8KDVDMouseMove(event);
    
    return false; // Use default handling
}

bool VLCCefMouseHandler::OnMouseWheel(CefRefPtr<CefBrowser> browser,
                                     const CefMouseEvent& event,
                                     int deltaX,
                                     int deltaY) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!mouse_events_enabled_) {
        return false;
    }
    
    UpdateMouseState(event);
    
    // Handle 8KDVD mouse wheel
    Handle8KDVDMouseWheel(deltaX, deltaY);
    
    return false; // Use default handling
}

bool VLCCefMouseHandler::Handle8KDVDMouseEvent(const CefMouseEvent& event, MouseButtonType type, bool mouseUp) {
    if (!disc_loaded_) return false;
    
    std::string action = mouseUp ? "release" : "press";
    std::string button = GetMouseButtonName(type);
    
    msg_Dbg(vlc_obj_, "8KDVD mouse %s: %s at (%d, %d)", 
           action.c_str(), button.c_str(), event.x, event.y);
    
    // Handle 8KDVD menu interactions
    if (IsMouseOverMenu(event)) {
        if (type == MBT_LEFT && !mouseUp) {
            msg_Info(vlc_obj_, "8KDVD: Menu item clicked at (%d, %d)", event.x, event.y);
            return true;
        }
    }
    
    return false;
}

void VLCCefMouseHandler::Handle8KDVDMouseMove(const CefMouseEvent& event) {
    if (!disc_loaded_) return;
    
    // Update mouse over menu state
    bool was_over_menu = mouse_over_menu_;
    mouse_over_menu_ = IsMouseOverMenu(event);
    
    if (mouse_over_menu_ != was_over_menu) {
        msg_Dbg(vlc_obj_, "8KDVD: Mouse %s menu at (%d, %d)", 
               mouse_over_menu_ ? "entered" : "left", event.x, event.y);
    }
}

void VLCCefMouseHandler::Handle8KDVDMouseWheel(int deltaX, int deltaY) {
    if (!disc_loaded_) return;
    
    if (deltaY != 0) {
        msg_Dbg(vlc_obj_, "8KDVD: Mouse wheel %s (delta: %d)", 
               deltaY > 0 ? "up" : "down", deltaY);
    }
    
    if (deltaX != 0) {
        msg_Dbg(vlc_obj_, "8KDVD: Mouse wheel %s (delta: %d)", 
               deltaX > 0 ? "right" : "left", deltaX);
    }
}

void VLCCefMouseHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefMouseHandler::SetMouseEventsEnabled(bool enable) {
    mouse_events_enabled_ = enable;
    msg_Info(vlc_obj_, "Mouse events %s", enable ? "enabled" : "disabled");
}

void VLCCefMouseHandler::SetMousePosition(int x, int y) {
    mouse_x_ = x;
    mouse_y_ = y;
    msg_Dbg(vlc_obj_, "Mouse position set: (%d, %d)", x, y);
}

void VLCCefMouseHandler::GetMousePosition(int& x, int& y) {
    x = mouse_x_;
    y = mouse_y_;
}

bool VLCCefMouseHandler::IsMouseOver8KDVDMenu() {
    return mouse_over_menu_;
}

bool VLCCefMouseHandler::HandleVLCMouseEvent(const CefMouseEvent& event, MouseButtonType type, bool mouseUp) {
    // Handle VLC-specific mouse events
    if (type == MBT_LEFT && !mouseUp) {
        msg_Dbg(vlc_obj_, "VLC mouse click at (%d, %d)", event.x, event.y);
        return true;
    }
    
    return false;
}

bool VLCCefMouseHandler::IsMouseOverMenu(const CefMouseEvent& event) {
    // Check if mouse is over 8KDVD menu area
    // This would integrate with the actual menu layout
    return (event.x >= 0 && event.x < 1920 && event.y >= 0 && event.y < 1080);
}

void VLCCefMouseHandler::UpdateMouseState(const CefMouseEvent& event) {
    mouse_x_ = event.x;
    mouse_y_ = event.y;
    mouse_over_menu_ = IsMouseOverMenu(event);
}

std::string VLCCefMouseHandler::GetMouseButtonName(MouseButtonType type) {
    switch (type) {
        case MBT_LEFT: return "Left";
        case MBT_MIDDLE: return "Middle";
        case MBT_RIGHT: return "Right";
        default: return "Unknown";
    }
}

void VLCCefMouseHandler::LogMouseEvent(const CefMouseEvent& event, const std::string& action) {
    msg_Dbg(vlc_obj_, "CEF mouse %s: (%d, %d) modifiers: %d", 
           action.c_str(), event.x, event.y, event.modifiers);
}
