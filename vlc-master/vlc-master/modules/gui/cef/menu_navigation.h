#ifndef VLC_MENU_NAVIGATION_H
#define VLC_MENU_NAVIGATION_H

#include <vlc_common.h>
#include <string>
#include <vector>
#include <map>

// Menu Navigation System for 8KDVD
typedef struct menu_navigation_t menu_navigation_t;

// Navigation direction
typedef enum {
    NAV_UP,
    NAV_DOWN,
    NAV_LEFT,
    NAV_RIGHT,
    NAV_SELECT,
    NAV_BACK
} nav_direction_t;

// Navigation state
typedef struct nav_state_t {
    int current_item;
    int total_items;
    bool can_navigate_up;
    bool can_navigate_down;
    bool can_navigate_left;
    bool can_navigate_right;
    bool can_select;
    bool can_go_back;
} nav_state_t;

// Menu Navigation Functions
menu_navigation_t* menu_navigation_create(vlc_object_t *obj);
void menu_navigation_destroy(menu_navigation_t *nav);

// Navigation Control
int menu_navigation_navigate(menu_navigation_t *nav, nav_direction_t direction);
int menu_navigation_select(menu_navigation_t *nav);
int menu_navigation_go_back(menu_navigation_t *nav);

// State Management
nav_state_t menu_navigation_get_state(menu_navigation_t *nav);
int menu_navigation_get_current_item(menu_navigation_t *nav);
int menu_navigation_get_total_items(menu_navigation_t *nav);

// Menu Integration
int menu_navigation_set_menu(menu_navigation_t *nav, const char *menu_id, int total_items);
int menu_navigation_update_items(menu_navigation_t *nav, int total_items);

// 8KDVD Specific Navigation
int menu_navigation_handle_8kdvd_input(menu_navigation_t *nav, const char *input);
int menu_navigation_set_8kdvd_mode(menu_navigation_t *nav, bool enable);

// VLC Integration
void menu_navigation_set_vlc_integration(menu_navigation_t *nav, bool enable);
void menu_navigation_set_debug_output(menu_navigation_t *nav, bool enable);

#endif // VLC_MENU_NAVIGATION_H
