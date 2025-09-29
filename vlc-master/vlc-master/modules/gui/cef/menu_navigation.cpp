#include "menu_navigation.h"
#include "html5_menu_system.h"
#include <vlc_messages.h>
#include <cstring>

// Menu Navigation System Implementation
struct menu_navigation_t {
    vlc_object_t *obj;
    html5_menu_system_t *menu_system;
    std::string current_menu_id;
    nav_state_t state;
    bool vlc_integration_enabled;
    bool debug_output_enabled;
    bool kdvd_mode_enabled;
};

menu_navigation_t* menu_navigation_create(vlc_object_t *obj) {
    menu_navigation_t *nav = new menu_navigation_t();
    if (!nav) return nullptr;
    
    nav->obj = obj;
    nav->menu_system = nullptr;
    nav->current_menu_id.clear();
    nav->vlc_integration_enabled = true;
    nav->debug_output_enabled = false;
    nav->kdvd_mode_enabled = false;
    
    // Initialize navigation state
    nav->state.current_item = 0;
    nav->state.total_items = 0;
    nav->state.can_navigate_up = false;
    nav->state.can_navigate_down = false;
    nav->state.can_navigate_left = false;
    nav->state.can_navigate_right = false;
    nav->state.can_select = false;
    nav->state.can_go_back = false;
    
    msg_Info(obj, "Menu navigation system created");
    return nav;
}

void menu_navigation_destroy(menu_navigation_t *nav) {
    if (!nav) return;
    
    nav->current_menu_id.clear();
    delete nav;
    msg_Info(nav->obj, "Menu navigation system destroyed");
}

int menu_navigation_navigate(menu_navigation_t *nav, nav_direction_t direction) {
    if (!nav) return -1;
    
    switch (direction) {
        case NAV_UP:
            if (nav->state.can_navigate_up) {
                nav->state.current_item--;
                if (nav->state.current_item < 0) {
                    nav->state.current_item = nav->state.total_items - 1;
                }
                msg_Dbg(nav->obj, "Navigation: Up (item %d)", nav->state.current_item);
            }
            break;
            
        case NAV_DOWN:
            if (nav->state.can_navigate_down) {
                nav->state.current_item++;
                if (nav->state.current_item >= nav->state.total_items) {
                    nav->state.current_item = 0;
                }
                msg_Dbg(nav->obj, "Navigation: Down (item %d)", nav->state.current_item);
            }
            break;
            
        case NAV_LEFT:
            if (nav->state.can_navigate_left) {
                msg_Dbg(nav->obj, "Navigation: Left");
            }
            break;
            
        case NAV_RIGHT:
            if (nav->state.can_navigate_right) {
                msg_Dbg(nav->obj, "Navigation: Right");
            }
            break;
            
        default:
            return -1;
    }
    
    // Update navigation state
    menu_navigation_update_navigation_state(nav);
    
    return 0;
}

int menu_navigation_select(menu_navigation_t *nav) {
    if (!nav || !nav->state.can_select) return -1;
    
    msg_Info(nav->obj, "Menu item selected: %d", nav->state.current_item);
    
    // Handle selection based on current menu
    if (nav->current_menu_id == "8kdvd_main") {
        return menu_navigation_handle_main_menu_selection(nav);
    } else if (nav->current_menu_id == "8kdvd_settings") {
        return menu_navigation_handle_settings_menu_selection(nav);
    } else if (nav->current_menu_id == "8kdvd_about") {
        return menu_navigation_handle_about_menu_selection(nav);
    }
    
    return 0;
}

int menu_navigation_go_back(menu_navigation_t *nav) {
    if (!nav || !nav->state.can_go_back) return -1;
    
    msg_Info(nav->obj, "Menu navigation: Go back");
    
    // Handle back navigation
    if (nav->current_menu_id == "8kdvd_settings" || nav->current_menu_id == "8kdvd_about") {
        // Return to main menu
        nav->current_menu_id = "8kdvd_main";
        nav->state.current_item = 0;
        menu_navigation_update_navigation_state(nav);
        
        if (nav->menu_system) {
            html5_menu_system_show_menu(nav->menu_system, "8kdvd_main");
        }
    } else if (nav->current_menu_id == "8kdvd_main") {
        // Exit menu system
        if (nav->menu_system) {
            html5_menu_system_hide_menu(nav->menu_system);
        }
    }
    
    return 0;
}

nav_state_t menu_navigation_get_state(menu_navigation_t *nav) {
    if (nav) {
        return nav->state;
    }
    
    nav_state_t empty_state = {0};
    return empty_state;
}

int menu_navigation_get_current_item(menu_navigation_t *nav) {
    return nav ? nav->state.current_item : -1;
}

int menu_navigation_get_total_items(menu_navigation_t *nav) {
    return nav ? nav->state.total_items : 0;
}

int menu_navigation_set_menu(menu_navigation_t *nav, const char *menu_id, int total_items) {
    if (!nav || !menu_id) return -1;
    
    nav->current_menu_id = menu_id;
    nav->state.total_items = total_items;
    nav->state.current_item = 0;
    
    menu_navigation_update_navigation_state(nav);
    
    msg_Info(nav->obj, "Menu navigation set: %s (%d items)", menu_id, total_items);
    return 0;
}

int menu_navigation_update_items(menu_navigation_t *nav, int total_items) {
    if (!nav) return -1;
    
    nav->state.total_items = total_items;
    
    // Ensure current item is within bounds
    if (nav->state.current_item >= total_items) {
        nav->state.current_item = total_items - 1;
    }
    if (nav->state.current_item < 0) {
        nav->state.current_item = 0;
    }
    
    menu_navigation_update_navigation_state(nav);
    
    msg_Dbg(nav->obj, "Menu navigation updated: %d items", total_items);
    return 0;
}

int menu_navigation_handle_8kdvd_input(menu_navigation_t *nav, const char *input) {
    if (!nav || !input) return -1;
    
    if (strcmp(input, "up") == 0) {
        return menu_navigation_navigate(nav, NAV_UP);
    } else if (strcmp(input, "down") == 0) {
        return menu_navigation_navigate(nav, NAV_DOWN);
    } else if (strcmp(input, "left") == 0) {
        return menu_navigation_navigate(nav, NAV_LEFT);
    } else if (strcmp(input, "right") == 0) {
        return menu_navigation_navigate(nav, NAV_RIGHT);
    } else if (strcmp(input, "select") == 0) {
        return menu_navigation_select(nav);
    } else if (strcmp(input, "back") == 0) {
        return menu_navigation_go_back(nav);
    }
    
    return -1;
}

int menu_navigation_set_8kdvd_mode(menu_navigation_t *nav, bool enable) {
    if (!nav) return -1;
    
    nav->kdvd_mode_enabled = enable;
    msg_Info(nav->obj, "8KDVD navigation mode %s", enable ? "enabled" : "disabled");
    return 0;
}

void menu_navigation_set_vlc_integration(menu_navigation_t *nav, bool enable) {
    if (nav) {
        nav->vlc_integration_enabled = enable;
        msg_Info(nav->obj, "VLC integration %s", enable ? "enabled" : "disabled");
    }
}

void menu_navigation_set_debug_output(menu_navigation_t *nav, bool enable) {
    if (nav) {
        nav->debug_output_enabled = enable;
        msg_Info(nav->obj, "Debug output %s", enable ? "enabled" : "disabled");
    }
}

// Helper functions
void menu_navigation_update_navigation_state(menu_navigation_t *nav) {
    if (!nav) return;
    
    // Update navigation capabilities based on current state
    nav->state.can_navigate_up = (nav->state.total_items > 1);
    nav->state.can_navigate_down = (nav->state.total_items > 1);
    nav->state.can_navigate_left = false; // Not used in current menu system
    nav->state.can_navigate_right = false; // Not used in current menu system
    nav->state.can_select = (nav->state.total_items > 0);
    nav->state.can_go_back = true; // Always allow going back
    
    if (nav->debug_output_enabled) {
        msg_Dbg(nav->obj, "Navigation state: item %d/%d, up=%s, down=%s, select=%s, back=%s",
               nav->state.current_item, nav->state.total_items,
               nav->state.can_navigate_up ? "true" : "false",
               nav->state.can_navigate_down ? "true" : "false",
               nav->state.can_select ? "true" : "false",
               nav->state.can_go_back ? "true" : "false");
    }
}

int menu_navigation_handle_main_menu_selection(menu_navigation_t *nav) {
    if (!nav) return -1;
    
    switch (nav->state.current_item) {
        case 0: // Play Title
            msg_Info(nav->obj, "8KDVD: Play title selected");
            // TODO: Implement play title action
            break;
            
        case 1: // Settings
            nav->current_menu_id = "8kdvd_settings";
            nav->state.current_item = 0;
            menu_navigation_update_navigation_state(nav);
            
            if (nav->menu_system) {
                html5_menu_system_show_menu(nav->menu_system, "8kdvd_settings");
            }
            break;
            
        case 2: // About
            nav->current_menu_id = "8kdvd_about";
            nav->state.current_item = 0;
            menu_navigation_update_navigation_state(nav);
            
            if (nav->menu_system) {
                html5_menu_system_show_menu(nav->menu_system, "8kdvd_about");
            }
            break;
            
        default:
            msg_Warn(nav->obj, "Unknown main menu selection: %d", nav->state.current_item);
            break;
    }
    
    return 0;
}

int menu_navigation_handle_settings_menu_selection(menu_navigation_t *nav) {
    if (!nav) return -1;
    
    switch (nav->state.current_item) {
        case 0: // Audio Settings
            msg_Info(nav->obj, "8KDVD: Audio settings selected");
            // TODO: Implement audio settings
            break;
            
        case 1: // Video Settings
            msg_Info(nav->obj, "8KDVD: Video settings selected");
            // TODO: Implement video settings
            break;
            
        case 2: // Back
            return menu_navigation_go_back(nav);
            
        default:
            msg_Warn(nav->obj, "Unknown settings menu selection: %d", nav->state.current_item);
            break;
    }
    
    return 0;
}

int menu_navigation_handle_about_menu_selection(menu_navigation_t *nav) {
    if (!nav) return -1;
    
    switch (nav->state.current_item) {
        case 0: // Disc Info
            msg_Info(nav->obj, "8KDVD: Disc info selected");
            // TODO: Implement disc info display
            break;
            
        case 1: // Back
            return menu_navigation_go_back(nav);
            
        default:
            msg_Warn(nav->obj, "Unknown about menu selection: %d", nav->state.current_item);
            break;
    }
    
    return 0;
}
