#ifndef VLC_8KDVD_DISC_MANAGER_H
#define VLC_8KDVD_DISC_MANAGER_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Disc Manager for Disc Detection and Management
typedef struct kdvd_disc_manager_t kdvd_disc_manager_t;

// 8KDVD Disc Information
typedef struct kdvd_disc_info_t {
    char disc_id[64];               // Disc ID
    char disc_title[256];           // Disc title
    char disc_manufacturer[128];    // Disc manufacturer
    char disc_version[32];          // Disc version
    uint64_t disc_size;             // Disc size in bytes
    uint32_t disc_type;             // Disc type (8KDVD, HD-DVD, etc.)
    bool is_authentic;              // Disc authenticity
    bool is_mounted;                // Disc mount status
    char mount_path[512];           // Mount path
    char file_system[32];           // File system type
    uint64_t creation_date;         // Disc creation date
    uint64_t last_access_date;      // Last access date
    uint32_t access_count;          // Access count
    bool write_protected;           // Write protection status
    bool region_locked;             // Region lock status
    uint32_t region_code;           // Region code
    char certificate_path[512];     // Certificate file path
    bool certificate_valid;         // Certificate validity
} kdvd_disc_info_t;

// 8KDVD Disc Manager Statistics
typedef struct kdvd_disc_manager_stats_t {
    uint64_t discs_detected;        // Total discs detected
    uint64_t discs_mounted;         // Total discs mounted
    uint64_t discs_unmounted;       // Total discs unmounted
    uint64_t authentication_attempts; // Authentication attempts
    uint64_t successful_auths;      // Successful authentications
    uint64_t failed_auths;          // Failed authentications
    uint64_t mount_time_us;         // Total mount time in microseconds
    uint64_t unmount_time_us;       // Total unmount time in microseconds
    float average_mount_time;       // Average mount time per disc
    float average_unmount_time;     // Average unmount time per disc
    uint32_t current_disc_count;    // Current disc count
    uint32_t memory_usage_mb;       // Memory usage in MB
} kdvd_disc_manager_stats_t;

// 8KDVD Disc Manager Functions
kdvd_disc_manager_t* kdvd_disc_manager_create(vlc_object_t *obj);
void kdvd_disc_manager_destroy(kdvd_disc_manager_t *manager);

// Disc Detection and Management
int kdvd_disc_manager_detect_disc(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_mount_disc(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_unmount_disc(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_eject_disc(kdvd_disc_manager_t *manager, const char *disc_path);

// Disc Information
kdvd_disc_info_t kdvd_disc_manager_get_disc_info(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_get_disc_count(kdvd_disc_manager_t *manager);
kdvd_disc_info_t kdvd_disc_manager_get_disc(kdvd_disc_manager_t *manager, uint32_t index);

// 8KDVD Specific Functions
int kdvd_disc_manager_detect_8kdvd_disc(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_validate_8kdvd_disc(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_authenticate_8kdvd_disc(kdvd_disc_manager_t *manager, const char *disc_path);

// Disc Authentication
int kdvd_disc_manager_authenticate_disc(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_validate_certificate(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_check_disc_integrity(kdvd_disc_manager_t *manager, const char *disc_path);

// Disc Monitoring
int kdvd_disc_manager_start_monitoring(kdvd_disc_manager_t *manager);
int kdvd_disc_manager_stop_monitoring(kdvd_disc_manager_t *manager);
int kdvd_disc_manager_set_disc_callback(kdvd_disc_manager_t *manager, void (*callback)(const char *disc_path, int event_type));

// Disc Operations
int kdvd_disc_manager_read_disc_info(kdvd_disc_manager_t *manager, const char *disc_path);
int kdvd_disc_manager_write_disc_info(kdvd_disc_manager_t *manager, const char *disc_path, const kdvd_disc_info_t *info);
int kdvd_disc_manager_update_disc_info(kdvd_disc_manager_t *manager, const char *disc_path);

// Performance and Statistics
kdvd_disc_manager_stats_t kdvd_disc_manager_get_stats(kdvd_disc_manager_t *manager);
int kdvd_disc_manager_reset_stats(kdvd_disc_manager_t *manager);
int kdvd_disc_manager_set_performance_mode(kdvd_disc_manager_t *manager, const char *mode);

// Memory Management
int kdvd_disc_manager_allocate_buffers(kdvd_disc_manager_t *manager);
int kdvd_disc_manager_free_buffers(kdvd_disc_manager_t *manager);
int kdvd_disc_manager_get_memory_usage(kdvd_disc_manager_t *manager, uint32_t *usage_mb);

// Error Handling
int kdvd_disc_manager_get_last_error(kdvd_disc_manager_t *manager, char *error_buffer, size_t buffer_size);
int kdvd_disc_manager_clear_errors(kdvd_disc_manager_t *manager);

// Debug and Logging
void kdvd_disc_manager_set_debug(kdvd_disc_manager_t *manager, bool enable);
void kdvd_disc_manager_log_info(kdvd_disc_manager_t *manager);
void kdvd_disc_manager_log_stats(kdvd_disc_manager_t *manager);

#endif // VLC_8KDVD_DISC_MANAGER_H


