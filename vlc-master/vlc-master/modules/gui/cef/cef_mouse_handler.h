#ifndef VLC_CEF_MOUSE_HANDLER_H
#define VLC_CEF_MOUSE_HANDLER_H

#include "include/cef_mouse_handler.h"
#include "include/cef_browser.h"
#include "include/cef_mouse_event.h"
#include <vlc_common.h>
#include <string>

// CEF Mouse Handler for VLC Integration
class VLCCefMouseHandler : public CefMouseHandler {
public:
    VLCCefMouseHandler(vlc_object_t *obj);
    ~VLCCefMouseHandler();
    
    // CefMouseHandler methods
    bool OnMouseClick(CefRefPtr<CefBrowser> browser,
                      const CefMouseEvent& event,
                      MouseButtonType type,
                      bool mouseUp,
                      int clickCount) override;
    
    bool OnMouseMove(CefRefPtr<CefBrowser> browser,
                     const CefMouseEvent& event,
                     bool mouseLeave) override;
    
    bool OnMouseWheel(CefRefPtr<CefBrowser> browser,
                      const CefMouseEvent& event,
                      int deltaX,
                      int deltaY) override;
    
    // 8KDVD specific mouse handling
    bool Handle8KDVDMouseEvent(const CefMouseEvent& event, MouseButtonType type, bool mouseUp);
    void Handle8KDVDMouseMove(const CefMouseEvent& event);
    void Handle8KDVDMouseWheel(int deltaX, int deltaY);
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetMouseEventsEnabled(bool enable);
    
    // Mouse state management
    void SetMousePosition(int x, int y);
    void GetMousePosition(int& x, int& y);
    bool IsMouseOver8KDVDMenu();
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    bool mouse_events_enabled_;
    
    // 8KDVD state
    bool disc_loaded_;
    std::string current_menu_;
    int mouse_x_;
    int mouse_y_;
    bool mouse_over_menu_;
    
    // Helper methods
    bool HandleVLCMouseEvent(const CefMouseEvent& event, MouseButtonType type, bool mouseUp);
    bool IsMouseOverMenu(const CefMouseEvent& event);
    void UpdateMouseState(const CefMouseEvent& event);
    std::string GetMouseButtonName(MouseButtonType type);
    void LogMouseEvent(const CefMouseEvent& event, const std::string& action);
    
    IMPLEMENT_REFCOUNTING(VLCCefMouseHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefMouseHandler);
};

#endif // VLC_CEF_MOUSE_HANDLER_H
