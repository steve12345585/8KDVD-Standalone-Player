#ifndef VLC_CEF_CONTEXT_MENU_HANDLER_H
#define VLC_CEF_CONTEXT_MENU_HANDLER_H

#include "include/cef_context_menu_handler.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_context_menu_params.h"
#include "include/cef_menu_model.h"
#include <vlc_common.h>
#include <string>

// CEF Context Menu Handler for VLC Integration
class VLCCefContextMenuHandler : public CefContextMenuHandler {
public:
    VLCCefContextMenuHandler(vlc_object_t *obj);
    ~VLCCefContextMenuHandler();
    
    // CefContextMenuHandler methods
    void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefContextMenuParams> params,
                            CefRefPtr<CefMenuModel> model) override;
    
    bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefContextMenuParams> params,
                             int command_id,
                             EventFlags event_flags) override;
    
    void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame) override;
    
    // 8KDVD specific context menu handling
    void Add8KDVDMenuItems(CefRefPtr<CefMenuModel> model);
    bool Handle8KDVDMenuCommand(int command_id);
    
    // VLC integration
    void SetVLCIntegration(bool enable);
    void SetContextMenuEnabled(bool enable);
    
private:
    vlc_object_t *vlc_obj_;
    bool vlc_integration_enabled_;
    bool context_menu_enabled_;
    
    // 8KDVD state
    bool disc_loaded_;
    std::string current_menu_;
    
    // Context menu command IDs
    enum ContextMenuCommands {
        CMD_8KDVD_PLAY_TITLE = 1000,
        CMD_8KDVD_SHOW_MENU,
        CMD_8KDVD_NAVIGATE_UP,
        CMD_8KDVD_NAVIGATE_DOWN,
        CMD_8KDVD_NAVIGATE_LEFT,
        CMD_8KDVD_NAVIGATE_RIGHT,
        CMD_8KDVD_SELECT,
        CMD_8KDVD_BACK,
        CMD_8KDVD_SETTINGS,
        CMD_8KDVD_ABOUT
    };
    
    // Helper methods
    void AddVLCMenuItems(CefRefPtr<CefMenuModel> model);
    void Add8KDVDNavigationItems(CefRefPtr<CefMenuModel> model);
    void Add8KDVDControlItems(CefRefPtr<CefMenuModel> model);
    bool HandleVLCMenuCommand(int command_id);
    bool Handle8KDVDNavigationCommand(int command_id);
    bool Handle8KDVDControlCommand(int command_id);
    
    IMPLEMENT_REFCOUNTING(VLCCefContextMenuHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefContextMenuHandler);
};

#endif // VLC_CEF_CONTEXT_MENU_HANDLER_H
