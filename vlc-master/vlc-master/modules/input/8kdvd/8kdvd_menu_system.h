#ifndef VLC_8KDVD_MENU_SYSTEM_H
#define VLC_8KDVD_MENU_SYSTEM_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Menu System for HTML5 Menu Integration
typedef struct kdvd_menu_system_t kdvd_menu_system_t;

// 8KDVD Menu Types
typedef enum {
    EIGHTKDVD_MENU_MAIN = 0,
    EIGHTKDVD_MENU_CHAPTER,
    EIGHTKDVD_MENU_AUDIO,
    EIGHTKDVD_MENU_SUBTITLE,
    EIGHTKDVD_MENU_SETTINGS,
    EIGHTKDVD_MENU_EXTRA,
    EIGHTKDVD_MENU_POPUP,
    EIGHTKDVD_MENU_OVERLAY
} kdvd_menu_type_t;

// 8KDVD Menu State
typedef enum {
    EIGHTKDVD_MENU_HIDDEN = 0,
    EIGHTKDVD_MENU_VISIBLE,
    EIGHTKDVD_MENU_ACTIVE,
    EIGHTKDVD_MENU_NAVIGATING,
    EIGHTKDVD_MENU_LOADING,
    EIGHTKDVD_MENU_ERROR
} kdvd_menu_state_t;

// 8KDVD Menu Item
typedef struct kdvd_menu_item_t {
    char id[64];                    // Menu item ID
    char title[256];                // Menu item title
    char description[512];          // Menu item description
    char action[128];               // Menu item action
    char target[128];               // Menu item target
    bool enabled;                   // Menu item enabled state
    bool visible;                   // Menu item visible state
    bool selected;                  // Menu item selected state
    uint32_t position_x;          // Menu item X position
    uint32_t position_y;            // Menu item Y position
    uint32_t width;                 // Menu item width
    uint32_t height;                // Menu item height
    char icon_path[512];           // Menu item icon path
    char sound_path[512];          // Menu item sound path
    uint32_t order;                // Menu item order
    void *user_data;                // User data pointer
} kdvd_menu_item_t;

// 8KDVD Menu Configuration
typedef struct kdvd_menu_config_t {
    bool enable_html5_menus;        // Enable HTML5 menu rendering
    bool enable_javascript;        // Enable JavaScript execution
    bool enable_css_animations;     // Enable CSS animations
    bool enable_sound_effects;      // Enable sound effects
    bool enable_transitions;        // Enable menu transitions
    uint32_t menu_timeout_ms;       // Menu timeout in milliseconds
    uint32_t animation_duration_ms; // Animation duration in milliseconds
    char default_theme[64];         // Default menu theme
    char default_language[8];       // Default language
    bool enable_keyboard_navigation; // Enable keyboard navigation
    bool enable_mouse_navigation;   // Enable mouse navigation
    bool enable_touch_navigation;   // Enable touch navigation
    uint32_t menu_width;            // Menu width
    uint32_t menu_height;           // Menu height
    bool enable_fullscreen_menus;   // Enable fullscreen menus
    bool enable_overlay_menus;      // Enable overlay menus
    char performance_mode[32];      // Performance mode
} kdvd_menu_config_t;

// 8KDVD Menu Statistics
typedef struct kdvd_menu_stats_t {
    uint64_t menus_loaded;          // Total menus loaded
    uint64_t menus_displayed;       // Total menus displayed
    uint64_t menu_interactions;     // Total menu interactions
    uint64_t menu_navigations;      // Total menu navigations
    uint64_t menu_selections;       // Total menu selections
    uint64_t menu_timeouts;         // Total menu timeouts
    uint64_t menu_errors;           // Total menu errors
    uint64_t javascript_executions; // Total JavaScript executions
    uint64_t css_animations;        // Total CSS animations
    uint64_t sound_effects;         // Total sound effects
    float average_menu_load_time;    // Average menu load time
    float average_menu_display_time; // Average menu display time
    float average_interaction_time;  // Average interaction time
    uint32_t current_menu_count;     // Current menu count
    uint32_t memory_usage_mb;       // Memory usage in MB
} kdvd_menu_stats_t;

// 8KDVD Menu System Functions
kdvd_menu_system_t* kdvd_menu_system_create(vlc_object_t *obj);
void kdvd_menu_system_destroy(kdvd_menu_system_t *menu_system);

// Menu Management
int kdvd_menu_system_load_menu(kdvd_menu_system_t *menu_system, const char *menu_path, kdvd_menu_type_t menu_type);
int kdvd_menu_system_display_menu(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);
int kdvd_menu_system_hide_menu(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);
int kdvd_menu_system_unload_menu(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);

// Menu Navigation
int kdvd_menu_system_navigate_up(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_navigate_down(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_navigate_left(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_navigate_right(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_select_item(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_go_back(kdvd_menu_system_t *menu_system);

// Menu Items
int kdvd_menu_system_add_menu_item(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const kdvd_menu_item_t *item);
int kdvd_menu_system_remove_menu_item(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const char *item_id);
int kdvd_menu_system_update_menu_item(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const char *item_id, const kdvd_menu_item_t *item);
kdvd_menu_item_t kdvd_menu_system_get_menu_item(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const char *item_id);

// Menu State
kdvd_menu_state_t kdvd_menu_system_get_menu_state(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);
int kdvd_menu_system_set_menu_state(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, kdvd_menu_state_t state);
bool kdvd_menu_system_is_menu_visible(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);
bool kdvd_menu_system_is_menu_active(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);

// Menu Configuration
int kdvd_menu_system_set_config(kdvd_menu_system_t *menu_system, const kdvd_menu_config_t *config);
kdvd_menu_config_t kdvd_menu_system_get_config(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_reset_config(kdvd_menu_system_t *menu_system);

// HTML5 Menu Rendering
int kdvd_menu_system_render_html5_menu(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const char *html_content);
int kdvd_menu_system_execute_javascript(kdvd_menu_system_t *menu_system, const char *javascript_code);
int kdvd_menu_system_load_css(kdvd_menu_system_t *menu_system, const char *css_content);
int kdvd_menu_system_apply_theme(kdvd_menu_system_t *menu_system, const char *theme_name);

// Menu Events
int kdvd_menu_system_set_event_callback(kdvd_menu_system_t *menu_system, void (*callback)(kdvd_menu_type_t menu_type, const char *event_name, const char *event_data));
int kdvd_menu_system_trigger_event(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const char *event_name, const char *event_data);

// Menu Audio
int kdvd_menu_system_play_sound(kdvd_menu_system_t *menu_system, const char *sound_path);
int kdvd_menu_system_stop_sound(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_set_volume(kdvd_menu_system_t *menu_system, float volume);

// Menu Animations
int kdvd_menu_system_start_animation(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type, const char *animation_name);
int kdvd_menu_system_stop_animation(kdvd_menu_system_t *menu_system, kdvd_menu_type_t menu_type);
int kdvd_menu_system_set_animation_duration(kdvd_menu_system_t *menu_system, uint32_t duration_ms);

// Performance and Statistics
kdvd_menu_stats_t kdvd_menu_system_get_stats(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_reset_stats(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_set_performance_mode(kdvd_menu_system_t *menu_system, const char *mode);

// Memory Management
int kdvd_menu_system_allocate_buffers(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_free_buffers(kdvd_menu_system_t *menu_system);
int kdvd_menu_system_get_memory_usage(kdvd_menu_system_t *menu_system, uint32_t *usage_mb);

// Error Handling
int kdvd_menu_system_get_last_error(kdvd_menu_system_t *menu_system, char *error_buffer, size_t buffer_size);
int kdvd_menu_system_clear_errors(kdvd_menu_system_t *menu_system);

// Debug and Logging
void kdvd_menu_system_set_debug(kdvd_menu_system_t *menu_system, bool enable);
void kdvd_menu_system_log_info(kdvd_menu_system_t *menu_system);
void kdvd_menu_system_log_stats(kdvd_menu_system_t *menu_system);

#endif // VLC_8KDVD_MENU_SYSTEM_H
