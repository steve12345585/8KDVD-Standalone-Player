#include "cef_context_menu_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>

VLCCefContextMenuHandler::VLCCefContextMenuHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vlc_integration_enabled_(true), context_menu_enabled_(true),
      disc_loaded_(false) {
    
    msg_Info(vlc_obj_, "CEF context menu handler created");
}

VLCCefContextMenuHandler::~VLCCefContextMenuHandler() {
    msg_Info(vlc_obj_, "CEF context menu handler destroyed");
}

void VLCCefContextMenuHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                                  CefRefPtr<CefFrame> frame,
                                                  CefRefPtr<CefContextMenuParams> params,
                                                  CefRefPtr<CefMenuModel> model) {
    CEF_REQUIRE_UI_THREAD();
    
    if (!context_menu_enabled_) {
        model->Clear();
        return;
    }
    
    msg_Dbg(vlc_obj_, "CEF context menu before: %s", params->GetLinkUrl().ToString().c_str());
    
    // Add VLC menu items
    if (vlc_integration_enabled_) {
        AddVLCMenuItems(model);
    }
    
    // Add 8KDVD menu items
    if (disc_loaded_) {
        Add8KDVDMenuItems(model);
    }
}

bool VLCCefContextMenuHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                                   CefRefPtr<CefFrame> frame,
                                                   CefRefPtr<CefContextMenuParams> params,
                                                   int command_id,
                                                   EventFlags event_flags) {
    CEF_REQUIRE_UI_THREAD();
    
    msg_Dbg(vlc_obj_, "CEF context menu command: %d", command_id);
    
    // Handle VLC menu commands
    if (HandleVLCMenuCommand(command_id)) {
        return true;
    }
    
    // Handle 8KDVD menu commands
    if (Handle8KDVDMenuCommand(command_id)) {
        return true;
    }
    
    return false; // Use default handling
}

void VLCCefContextMenuHandler::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser,
                                                     CefRefPtr<CefFrame> frame) {
    CEF_REQUIRE_UI_THREAD();
    
    msg_Dbg(vlc_obj_, "CEF context menu dismissed");
}

void VLCCefContextMenuHandler::Add8KDVDMenuItems(CefRefPtr<CefMenuModel> model) {
    // Add separator
    model->AddSeparator();
    
    // Add 8KDVD navigation items
    Add8KDVDNavigationItems(model);
    
    // Add 8KDVD control items
    Add8KDVDControlItems(model);
}

bool VLCCefContextMenuHandler::Handle8KDVDMenuCommand(int command_id) {
    switch (command_id) {
        case CMD_8KDVD_PLAY_TITLE:
            msg_Info(vlc_obj_, "8KDVD: Play title command");
            return true;
            
        case CMD_8KDVD_SHOW_MENU:
            msg_Info(vlc_obj_, "8KDVD: Show menu command");
            return true;
            
        case CMD_8KDVD_NAVIGATE_UP:
            msg_Info(vlc_obj_, "8KDVD: Navigate up command");
            return true;
            
        case CMD_8KDVD_NAVIGATE_DOWN:
            msg_Info(vlc_obj_, "8KDVD: Navigate down command");
            return true;
            
        case CMD_8KDVD_NAVIGATE_LEFT:
            msg_Info(vlc_obj_, "8KDVD: Navigate left command");
            return true;
            
        case CMD_8KDVD_NAVIGATE_RIGHT:
            msg_Info(vlc_obj_, "8KDVD: Navigate right command");
            return true;
            
        case CMD_8KDVD_SELECT:
            msg_Info(vlc_obj_, "8KDVD: Select command");
            return true;
            
        case CMD_8KDVD_BACK:
            msg_Info(vlc_obj_, "8KDVD: Back command");
            return true;
            
        case CMD_8KDVD_SETTINGS:
            msg_Info(vlc_obj_, "8KDVD: Settings command");
            return true;
            
        case CMD_8KDVD_ABOUT:
            msg_Info(vlc_obj_, "8KDVD: About command");
            return true;
            
        default:
            return false;
    }
}

void VLCCefContextMenuHandler::SetVLCIntegration(bool enable) {
    vlc_integration_enabled_ = enable;
    msg_Info(vlc_obj_, "VLC integration %s", enable ? "enabled" : "disabled");
}

void VLCCefContextMenuHandler::SetContextMenuEnabled(bool enable) {
    context_menu_enabled_ = enable;
    msg_Info(vlc_obj_, "Context menu %s", enable ? "enabled" : "disabled");
}

void VLCCefContextMenuHandler::AddVLCMenuItems(CefRefPtr<CefMenuModel> model) {
    // Add VLC-specific menu items
    model->AddItem(CMD_8KDVD_PLAY_TITLE, "VLC: Play Title");
    model->AddItem(CMD_8KDVD_SHOW_MENU, "VLC: Show Menu");
    model->AddSeparator();
}

void VLCCefContextMenuHandler::Add8KDVDNavigationItems(CefRefPtr<CefMenuModel> model) {
    // Add 8KDVD navigation items
    model->AddItem(CMD_8KDVD_NAVIGATE_UP, "8KDVD: Navigate Up");
    model->AddItem(CMD_8KDVD_NAVIGATE_DOWN, "8KDVD: Navigate Down");
    model->AddItem(CMD_8KDVD_NAVIGATE_LEFT, "8KDVD: Navigate Left");
    model->AddItem(CMD_8KDVD_NAVIGATE_RIGHT, "8KDVD: Navigate Right");
    model->AddSeparator();
}

void VLCCefContextMenuHandler::Add8KDVDControlItems(CefRefPtr<CefMenuModel> model) {
    // Add 8KDVD control items
    model->AddItem(CMD_8KDVD_SELECT, "8KDVD: Select");
    model->AddItem(CMD_8KDVD_BACK, "8KDVD: Back");
    model->AddSeparator();
    model->AddItem(CMD_8KDVD_SETTINGS, "8KDVD: Settings");
    model->AddItem(CMD_8KDVD_ABOUT, "8KDVD: About");
}

bool VLCCefContextMenuHandler::HandleVLCMenuCommand(int command_id) {
    // Handle VLC-specific menu commands
    switch (command_id) {
        case CMD_8KDVD_PLAY_TITLE:
            msg_Info(vlc_obj_, "VLC: Play title command");
            return true;
            
        case CMD_8KDVD_SHOW_MENU:
            msg_Info(vlc_obj_, "VLC: Show menu command");
            return true;
            
        default:
            return false;
    }
}

bool VLCCefContextMenuHandler::Handle8KDVDNavigationCommand(int command_id) {
    // Handle 8KDVD navigation commands
    switch (command_id) {
        case CMD_8KDVD_NAVIGATE_UP:
        case CMD_8KDVD_NAVIGATE_DOWN:
        case CMD_8KDVD_NAVIGATE_LEFT:
        case CMD_8KDVD_NAVIGATE_RIGHT:
            msg_Info(vlc_obj_, "8KDVD: Navigation command %d", command_id);
            return true;
            
        default:
            return false;
    }
}

bool VLCCefContextMenuHandler::Handle8KDVDControlCommand(int command_id) {
    // Handle 8KDVD control commands
    switch (command_id) {
        case CMD_8KDVD_SELECT:
        case CMD_8KDVD_BACK:
        case CMD_8KDVD_SETTINGS:
        case CMD_8KDVD_ABOUT:
            msg_Info(vlc_obj_, "8KDVD: Control command %d", command_id);
            return true;
            
        default:
            return false;
    }
}
