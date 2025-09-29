#ifndef VLC_HTML5_MENU_SYSTEM_H
#define VLC_HTML5_MENU_SYSTEM_H

#include <vlc_common.h>
#include <string>
#include <vector>
#include <map>

// HTML5 Menu System for 8KDVD
typedef struct html5_menu_system_t html5_menu_system_t;

// Menu item structure
typedef struct menu_item_t {
    std::string id;
    std::string title;
    std::string description;
    std::string icon;
    std::string action;
    std::map<std::string, std::string> properties;
    bool enabled;
    bool visible;
} menu_item_t;

// Menu structure
typedef struct menu_t {
    std::string id;
    std::string title;
    std::string template_path;
    std::vector<menu_item_t> items;
    std::string background_image;
    std::string background_color;
    std::string theme;
    int width;
    int height;
    bool fullscreen;
} menu_t;

// HTML5 Menu System Functions
html5_menu_system_t* html5_menu_system_create(vlc_object_t *obj);
void html5_menu_system_destroy(html5_menu_system_t *system);

// Menu Management
int html5_menu_system_load_menu(html5_menu_system_t *system, const char *menu_path);
int html5_menu_system_create_menu(html5_menu_system_t *system, const menu_t *menu);
int html5_menu_system_show_menu(html5_menu_system_t *system, const char *menu_id);
int html5_menu_system_hide_menu(html5_menu_system_t *system);

// 8KDVD Specific Functions
int html5_menu_system_load_8kdvd_menu(html5_menu_system_t *system, const char *disc_path);
int html5_menu_system_create_8kdvd_main_menu(html5_menu_system_t *system, const char *disc_path);
int html5_menu_system_create_8kdvd_settings_menu(html5_menu_system_t *system);
int html5_menu_system_create_8kdvd_about_menu(html5_menu_system_t *system);

// Menu Navigation
int html5_menu_system_navigate_up(html5_menu_system_t *system);
int html5_menu_system_navigate_down(html5_menu_system_t *system);
int html5_menu_system_navigate_left(html5_menu_system_t *system);
int html5_menu_system_navigate_right(html5_menu_system_t *system);
int html5_menu_system_select_item(html5_menu_system_t *system);
int html5_menu_system_go_back(html5_menu_system_t *system);

// Menu State
bool html5_menu_system_is_menu_visible(html5_menu_system_t *system);
const char* html5_menu_system_get_current_menu(html5_menu_system_t *system);
int html5_menu_system_get_selected_item(html5_menu_system_t *system);

// VLC Integration
void html5_menu_system_set_vlc_integration(html5_menu_system_t *system, bool enable);
void html5_menu_system_set_8kdvd_mode(html5_menu_system_t *system, bool enable);

#endif // VLC_HTML5_MENU_SYSTEM_H
