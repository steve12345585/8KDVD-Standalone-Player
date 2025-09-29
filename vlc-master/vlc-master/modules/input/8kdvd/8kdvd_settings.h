#ifndef VLC_8KDVD_SETTINGS_H
#define VLC_8KDVD_SETTINGS_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Settings Management for User Preferences and Configuration
typedef struct kdvd_settings_t kdvd_settings_t;

// 8KDVD Settings Categories
typedef enum {
    EIGHTKDVD_SETTINGS_GENERAL = 0,
    EIGHTKDVD_SETTINGS_VIDEO,
    EIGHTKDVD_SETTINGS_AUDIO,
    EIGHTKDVD_SETTINGS_MENU,
    EIGHTKDVD_SETTINGS_PLAYBACK,
    EIGHTKDVD_SETTINGS_SECURITY,
    EIGHTKDVD_SETTINGS_PERFORMANCE,
    EIGHTKDVD_SETTINGS_ADVANCED
} kdvd_settings_category_t;

// 8KDVD Settings Types
typedef enum {
    EIGHTKDVD_SETTING_BOOLEAN = 0,
    EIGHTKDVD_SETTING_INTEGER,
    EIGHTKDVD_SETTING_FLOAT,
    EIGHTKDVD_SETTING_STRING,
    EIGHTKDVD_SETTING_ENUM,
    EIGHTKDVD_SETTING_COLOR,
    EIGHTKDVD_SETTING_PATH,
    EIGHTKDVD_SETTING_PASSWORD
} kdvd_setting_type_t;

// 8KDVD Setting Value
typedef union {
    bool boolean_value;
    int32_t integer_value;
    float float_value;
    char string_value[256];
    uint32_t enum_value;
    uint32_t color_value;
    char path_value[512];
    char password_value[64];
} kdvd_setting_value_t;

// 8KDVD Setting Definition
typedef struct kdvd_setting_t {
    char name[64];                  // Setting name
    char display_name[128];         // Display name
    char description[256];          // Setting description
    kdvd_setting_type_t type;       // Setting type
    kdvd_settings_category_t category; // Setting category
    kdvd_setting_value_t default_value; // Default value
    kdvd_setting_value_t current_value; // Current value
    kdvd_setting_value_t min_value;     // Minimum value (for numeric types)
    kdvd_setting_value_t max_value;     // Maximum value (for numeric types)
    char enum_options[8][64];       // Enum options (for enum type)
    uint32_t enum_count;            // Enum option count
    bool readonly;                  // Read-only setting
    bool hidden;                    // Hidden setting
    bool advanced;                  // Advanced setting
    char unit[16];                  // Value unit (e.g., "ms", "Hz", "Mbps")
    char help_text[512];            // Help text
    void *user_data;                // User data pointer
} kdvd_setting_t;

// 8KDVD Settings Profile
typedef struct kdvd_settings_profile_t {
    char name[64];                  // Profile name
    char description[256];          // Profile description
    bool is_default;               // Default profile
    bool is_custom;                // Custom profile
    uint64_t created_date;         // Creation date
    uint64_t modified_date;        // Last modification date
    uint32_t setting_count;        // Setting count
    kdvd_setting_t *settings;       // Settings array
} kdvd_settings_profile_t;

// 8KDVD Settings Statistics
typedef struct kdvd_settings_stats_t {
    uint64_t settings_loaded;       // Total settings loaded
    uint64_t settings_saved;       // Total settings saved
    uint64_t settings_reset;       // Total settings reset
    uint64_t profiles_created;     // Total profiles created
    uint64_t profiles_deleted;     // Total profiles deleted
    uint64_t settings_imported;    // Total settings imported
    uint64_t settings_exported;    // Total settings exported
    uint64_t settings_validated;  // Total settings validated
    uint64_t settings_errors;      // Total settings errors
    float average_load_time;        // Average load time
    float average_save_time;        // Average save time
    uint32_t current_setting_count; // Current setting count
    uint32_t current_profile_count; // Current profile count
    uint32_t memory_usage_mb;       // Memory usage in MB
} kdvd_settings_stats_t;

// 8KDVD Settings Functions
kdvd_settings_t* kdvd_settings_create(vlc_object_t *obj);
void kdvd_settings_destroy(kdvd_settings_t *settings);

// Settings Management
int kdvd_settings_load(kdvd_settings_t *settings, const char *config_path);
int kdvd_settings_save(kdvd_settings_t *settings, const char *config_path);
int kdvd_settings_reset(kdvd_settings_t *settings);
int kdvd_settings_validate(kdvd_settings_t *settings);

// Setting Operations
int kdvd_settings_set_boolean(kdvd_settings_t *settings, const char *name, bool value);
int kdvd_settings_set_integer(kdvd_settings_t *settings, const char *name, int32_t value);
int kdvd_settings_set_float(kdvd_settings_t *settings, const char *name, float value);
int kdvd_settings_set_string(kdvd_settings_t *settings, const char *name, const char *value);
int kdvd_settings_set_enum(kdvd_settings_t *settings, const char *name, uint32_t value);
int kdvd_settings_set_color(kdvd_settings_t *settings, const char *name, uint32_t value);
int kdvd_settings_set_path(kdvd_settings_t *settings, const char *name, const char *value);
int kdvd_settings_set_password(kdvd_settings_t *settings, const char *name, const char *value);

bool kdvd_settings_get_boolean(kdvd_settings_t *settings, const char *name);
int32_t kdvd_settings_get_integer(kdvd_settings_t *settings, const char *name);
float kdvd_settings_get_float(kdvd_settings_t *settings, const char *name);
const char* kdvd_settings_get_string(kdvd_settings_t *settings, const char *name);
uint32_t kdvd_settings_get_enum(kdvd_settings_t *settings, const char *name);
uint32_t kdvd_settings_get_color(kdvd_settings_t *settings, const char *name);
const char* kdvd_settings_get_path(kdvd_settings_t *settings, const char *name);
const char* kdvd_settings_get_password(kdvd_settings_t *settings, const char *name);

// Setting Information
kdvd_setting_t kdvd_settings_get_setting(kdvd_settings_t *settings, const char *name);
int kdvd_settings_get_setting_count(kdvd_settings_t *settings);
int kdvd_settings_get_setting_count_by_category(kdvd_settings_t *settings, kdvd_settings_category_t category);
kdvd_setting_t* kdvd_settings_get_settings_by_category(kdvd_settings_t *settings, kdvd_settings_category_t category);

// Settings Profiles
int kdvd_settings_create_profile(kdvd_settings_t *settings, const char *profile_name, const char *description);
int kdvd_settings_delete_profile(kdvd_settings_t *settings, const char *profile_name);
int kdvd_settings_load_profile(kdvd_settings_t *settings, const char *profile_name);
int kdvd_settings_save_profile(kdvd_settings_t *settings, const char *profile_name);
int kdvd_settings_get_profile_count(kdvd_settings_t *settings);
kdvd_settings_profile_t kdvd_settings_get_profile(kdvd_settings_t *settings, const char *profile_name);
kdvd_settings_profile_t* kdvd_settings_get_all_profiles(kdvd_settings_t *settings);

// Settings Import/Export
int kdvd_settings_import(kdvd_settings_t *settings, const char *import_path);
int kdvd_settings_export(kdvd_settings_t *settings, const char *export_path);
int kdvd_settings_import_from_string(kdvd_settings_t *settings, const char *settings_string);
int kdvd_settings_export_to_string(kdvd_settings_t *settings, char *settings_string, size_t buffer_size);

// Settings Validation
int kdvd_settings_validate_setting(kdvd_settings_t *settings, const char *name, const kdvd_setting_value_t *value);
int kdvd_settings_validate_all(kdvd_settings_t *settings);
int kdvd_settings_get_validation_errors(kdvd_settings_t *settings, char *error_buffer, size_t buffer_size);

// Settings Categories
int kdvd_settings_get_category_count(kdvd_settings_t *settings);
const char* kdvd_settings_get_category_name(kdvd_settings_t *settings, kdvd_settings_category_t category);
const char* kdvd_settings_get_category_description(kdvd_settings_t *settings, kdvd_settings_category_t category);

// Performance and Statistics
kdvd_settings_stats_t kdvd_settings_get_stats(kdvd_settings_t *settings);
int kdvd_settings_reset_stats(kdvd_settings_t *settings);
int kdvd_settings_set_performance_mode(kdvd_settings_t *settings, const char *mode);

// Memory Management
int kdvd_settings_allocate_buffers(kdvd_settings_t *settings);
int kdvd_settings_free_buffers(kdvd_settings_t *settings);
int kdvd_settings_get_memory_usage(kdvd_settings_t *settings, uint32_t *usage_mb);

// Error Handling
int kdvd_settings_get_last_error(kdvd_settings_t *settings, char *error_buffer, size_t buffer_size);
int kdvd_settings_clear_errors(kdvd_settings_t *settings);

// Debug and Logging
void kdvd_settings_set_debug(kdvd_settings_t *settings, bool enable);
void kdvd_settings_log_info(kdvd_settings_t *settings);
void kdvd_settings_log_stats(kdvd_settings_t *settings);

#endif // VLC_8KDVD_SETTINGS_H
