#include "html5_menu_system.h"
#include "cef_wrapper.h"
#include <vlc_messages.h>
#include <fstream>
#include <sstream>
#include <filesystem>

// HTML5 Menu System Implementation
struct html5_menu_system_t {
    vlc_object_t *obj;
    cef_wrapper_t *cef_wrapper;
    std::map<std::string, menu_t> menus;
    std::string current_menu_id;
    int selected_item_index;
    bool vlc_integration_enabled;
    bool kdvd_mode_enabled;
    bool menu_visible;
};

html5_menu_system_t* html5_menu_system_create(vlc_object_t *obj) {
    html5_menu_system_t *system = new html5_menu_system_t();
    if (!system) return nullptr;
    
    system->obj = obj;
    system->cef_wrapper = cef_wrapper_create(obj);
    system->selected_item_index = 0;
    system->vlc_integration_enabled = true;
    system->kdvd_mode_enabled = false;
    system->menu_visible = false;
    
    if (!system->cef_wrapper) {
        delete system;
        return nullptr;
    }
    
    if (cef_wrapper_initialize(system->cef_wrapper) != 0) {
        cef_wrapper_destroy(system->cef_wrapper);
        delete system;
        return nullptr;
    }
    
    msg_Info(obj, "HTML5 Menu System created");
    return system;
}

void html5_menu_system_destroy(html5_menu_system_t *system) {
    if (!system) return;
    
    if (system->cef_wrapper) {
        cef_wrapper_shutdown(system->cef_wrapper);
        cef_wrapper_destroy(system->cef_wrapper);
    }
    
    system->menus.clear();
    delete system;
    msg_Info(system->obj, "HTML5 Menu System destroyed");
}

int html5_menu_system_load_menu(html5_menu_system_t *system, const char *menu_path) {
    if (!system || !menu_path) return -1;
    
    std::string path = menu_path;
    msg_Info(system->obj, "Loading HTML5 menu: %s", path.c_str());
    
    // Load menu via CEF wrapper
    int result = cef_wrapper_load_menu(system->cef_wrapper, menu_path);
    if (result == 0) {
        system->menu_visible = true;
        msg_Info(system->obj, "HTML5 menu loaded successfully");
    } else {
        msg_Err(system->obj, "Failed to load HTML5 menu: %s", path.c_str());
    }
    
    return result;
}

int html5_menu_system_create_menu(html5_menu_system_t *system, const menu_t *menu) {
    if (!system || !menu) return -1;
    
    // Store menu in system
    system->menus[menu->id] = *menu;
    msg_Info(system->obj, "HTML5 menu created: %s", menu->id.c_str());
    
    return 0;
}

int html5_menu_system_show_menu(html5_menu_system_t *system, const char *menu_id) {
    if (!system || !menu_id) return -1;
    
    auto it = system->menus.find(menu_id);
    if (it == system->menus.end()) {
        msg_Err(system->obj, "Menu not found: %s", menu_id);
        return -1;
    }
    
    const menu_t &menu = it->second;
    std::string menu_url = "file://" + menu.template_path;
    
    int result = cef_wrapper_load_menu(system->cef_wrapper, menu_url.c_str());
    if (result == 0) {
        system->current_menu_id = menu_id;
        system->menu_visible = true;
        system->selected_item_index = 0;
        msg_Info(system->obj, "HTML5 menu shown: %s", menu_id);
    }
    
    return result;
}

int html5_menu_system_hide_menu(html5_menu_system_t *system) {
    if (!system) return -1;
    
    system->menu_visible = false;
    system->current_menu_id.clear();
    system->selected_item_index = 0;
    
    msg_Info(system->obj, "HTML5 menu hidden");
    return 0;
}

// 8KDVD Specific Functions
int html5_menu_system_load_8kdvd_menu(html5_menu_system_t *system, const char *disc_path) {
    if (!system || !disc_path) return -1;
    
    system->kdvd_mode_enabled = true;
    msg_Info(system->obj, "Loading 8KDVD menu from: %s", disc_path);
    
    int result = cef_wrapper_load_8kdvd_menu(system->cef_wrapper, disc_path);
    if (result == 0) {
        system->menu_visible = true;
        msg_Info(system->obj, "8KDVD menu loaded successfully");
    }
    
    return result;
}

int html5_menu_system_create_8kdvd_main_menu(html5_menu_system_t *system, const char *disc_path) {
    if (!system || !disc_path) return -1;
    
    menu_t main_menu;
    main_menu.id = "8kdvd_main";
    main_menu.title = "8KDVD Main Menu";
    main_menu.template_path = std::string(disc_path) + "/8KDVD_TS/index.html";
    main_menu.background_color = "#1a1a1a";
    main_menu.theme = "8kdvd";
    main_menu.width = 7680;  // 8K width
    main_menu.height = 4320; // 8K height
    main_menu.fullscreen = true;
    
    // Add menu items
    menu_item_t play_item;
    play_item.id = "play_title";
    play_item.title = "Play Title";
    play_item.description = "Start playing the 8KDVD";
    play_item.action = "play_title";
    play_item.enabled = true;
    play_item.visible = true;
    main_menu.items.push_back(play_item);
    
    menu_item_t settings_item;
    settings_item.id = "settings";
    settings_item.title = "Settings";
    settings_item.description = "Configure playback settings";
    settings_item.action = "show_settings";
    settings_item.enabled = true;
    settings_item.visible = true;
    main_menu.items.push_back(settings_item);
    
    menu_item_t about_item;
    about_item.id = "about";
    about_item.title = "About";
    about_item.description = "About this 8KDVD";
    about_item.action = "show_about";
    about_item.enabled = true;
    about_item.visible = true;
    main_menu.items.push_back(about_item);
    
    return html5_menu_system_create_menu(system, &main_menu);
}

int html5_menu_system_create_8kdvd_settings_menu(html5_menu_system_t *system) {
    if (!system) return -1;
    
    menu_t settings_menu;
    settings_menu.id = "8kdvd_settings";
    settings_menu.title = "8KDVD Settings";
    settings_menu.background_color = "#2a2a2a";
    settings_menu.theme = "8kdvd";
    settings_menu.width = 7680;
    settings_menu.height = 4320;
    settings_menu.fullscreen = true;
    
    // Add settings items
    menu_item_t audio_item;
    audio_item.id = "audio_settings";
    audio_item.title = "Audio Settings";
    audio_item.description = "Configure audio options";
    audio_item.action = "audio_settings";
    audio_item.enabled = true;
    audio_item.visible = true;
    settings_menu.items.push_back(audio_item);
    
    menu_item_t video_item;
    video_item.id = "video_settings";
    video_item.title = "Video Settings";
    video_item.description = "Configure video options";
    video_item.action = "video_settings";
    video_item.enabled = true;
    video_item.visible = true;
    settings_menu.items.push_back(video_item);
    
    menu_item_t back_item;
    back_item.id = "back";
    back_item.title = "Back";
    back_item.description = "Return to main menu";
    back_item.action = "go_back";
    back_item.enabled = true;
    back_item.visible = true;
    settings_menu.items.push_back(back_item);
    
    return html5_menu_system_create_menu(system, &settings_menu);
}

int html5_menu_system_create_8kdvd_about_menu(html5_menu_system_t *system) {
    if (!system) return -1;
    
    menu_t about_menu;
    about_menu.id = "8kdvd_about";
    about_menu.title = "About 8KDVD";
    about_menu.background_color = "#3a3a3a";
    about_menu.theme = "8kdvd";
    about_menu.width = 7680;
    about_menu.height = 4320;
    about_menu.fullscreen = true;
    
    // Add about items
    menu_item_t info_item;
    info_item.id = "disc_info";
    info_item.title = "Disc Information";
    info_item.description = "View disc details";
    info_item.action = "show_disc_info";
    info_item.enabled = true;
    info_item.visible = true;
    about_menu.items.push_back(info_item);
    
    menu_item_t back_item;
    back_item.id = "back";
    back_item.title = "Back";
    back_item.description = "Return to main menu";
    back_item.action = "go_back";
    back_item.enabled = true;
    back_item.visible = true;
    about_menu.items.push_back(back_item);
    
    return html5_menu_system_create_menu(system, &about_menu);
}

// Menu Navigation
int html5_menu_system_navigate_up(html5_menu_system_t *system) {
    if (!system || !system->menu_visible) return -1;
    
    auto it = system->menus.find(system->current_menu_id);
    if (it == system->menus.end()) return -1;
    
    const menu_t &menu = it->second;
    if (system->selected_item_index > 0) {
        system->selected_item_index--;
        msg_Dbg(system->obj, "Navigation: Up (item %d)", system->selected_item_index);
    }
    
    return 0;
}

int html5_menu_system_navigate_down(html5_menu_system_t *system) {
    if (!system || !system->menu_visible) return -1;
    
    auto it = system->menus.find(system->current_menu_id);
    if (it == system->menus.end()) return -1;
    
    const menu_t &menu = it->second;
    if (system->selected_item_index < (int)menu.items.size() - 1) {
        system->selected_item_index++;
        msg_Dbg(system->obj, "Navigation: Down (item %d)", system->selected_item_index);
    }
    
    return 0;
}

int html5_menu_system_navigate_left(html5_menu_system_t *system) {
    if (!system || !system->menu_visible) return -1;
    
    msg_Dbg(system->obj, "Navigation: Left");
    return 0;
}

int html5_menu_system_navigate_right(html5_menu_system_t *system) {
    if (!system || !system->menu_visible) return -1;
    
    msg_Dbg(system->obj, "Navigation: Right");
    return 0;
}

int html5_menu_system_select_item(html5_menu_system_t *system) {
    if (!system || !system->menu_visible) return -1;
    
    auto it = system->menus.find(system->current_menu_id);
    if (it == system->menus.end()) return -1;
    
    const menu_t &menu = it->second;
    if (system->selected_item_index >= 0 && system->selected_item_index < (int)menu.items.size()) {
        const menu_item_t &item = menu.items[system->selected_item_index];
        msg_Info(system->obj, "Menu item selected: %s (%s)", item.title.c_str(), item.action.c_str());
        
        // Handle menu item action
        if (item.action == "play_title") {
            // TODO: Implement play title action
        } else if (item.action == "show_settings") {
            html5_menu_system_show_menu(system, "8kdvd_settings");
        } else if (item.action == "show_about") {
            html5_menu_system_show_menu(system, "8kdvd_about");
        } else if (item.action == "go_back") {
            html5_menu_system_show_menu(system, "8kdvd_main");
        }
    }
    
    return 0;
}

int html5_menu_system_go_back(html5_menu_system_t *system) {
    if (!system || !system->menu_visible) return -1;
    
    msg_Info(system->obj, "Menu navigation: Go back");
    
    // Simple back navigation - return to main menu
    if (system->current_menu_id != "8kdvd_main") {
        return html5_menu_system_show_menu(system, "8kdvd_main");
    }
    
    return 0;
}

// Menu State
bool html5_menu_system_is_menu_visible(html5_menu_system_t *system) {
    return system ? system->menu_visible : false;
}

const char* html5_menu_system_get_current_menu(html5_menu_system_t *system) {
    return system ? system->current_menu_id.c_str() : nullptr;
}

int html5_menu_system_get_selected_item(html5_menu_system_t *system) {
    return system ? system->selected_item_index : -1;
}

// VLC Integration
void html5_menu_system_set_vlc_integration(html5_menu_system_t *system, bool enable) {
    if (system) {
        system->vlc_integration_enabled = enable;
        msg_Info(system->obj, "VLC integration %s", enable ? "enabled" : "disabled");
    }
}

void html5_menu_system_set_8kdvd_mode(html5_menu_system_t *system, bool enable) {
    if (system) {
        system->kdvd_mode_enabled = enable;
        msg_Info(system->obj, "8KDVD mode %s", enable ? "enabled" : "disabled");
    }
}
