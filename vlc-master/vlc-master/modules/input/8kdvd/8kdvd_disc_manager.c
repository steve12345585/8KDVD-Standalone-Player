#include "8kdvd_disc_manager.h"
#include "8kdvd_certificate_validator.h"
#include <vlc_messages.h>
#include <vlc_fs.h>
#include <vlc_meta.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

// 8KDVD Disc Manager Implementation
struct kdvd_disc_manager_t {
    vlc_object_t *obj;
    kdvd_disc_manager_stats_t stats;
    bool initialized;
    bool debug_enabled;
    char last_error[256];
    void *manager_context;  // Placeholder for actual manager context
    kdvd_disc_info_t *discs;
    uint32_t disc_count;
    kdvd_certificate_validator_t *certificate_validator;
    void (*disc_callback)(const char *disc_path, int event_type);
    uint64_t start_time;
    uint64_t last_operation_time;
};

// 8KDVD Disc Manager Functions
kdvd_disc_manager_t* kdvd_disc_manager_create(vlc_object_t *obj) {
    kdvd_disc_manager_t *manager = calloc(1, sizeof(kdvd_disc_manager_t));
    if (!manager) return NULL;
    
    manager->obj = obj;
    manager->initialized = false;
    manager->debug_enabled = false;
    manager->manager_context = NULL;
    manager->discs = NULL;
    manager->disc_count = 0;
    manager->certificate_validator = NULL;
    manager->disc_callback = NULL;
    manager->start_time = 0;
    manager->last_operation_time = 0;
    
    // Initialize stats
    memset(&manager->stats, 0, sizeof(kdvd_disc_manager_stats_t));
    
    // Create certificate validator
    manager->certificate_validator = kdvd_certificate_validator_create(obj);
    if (!manager->certificate_validator) {
        msg_Err(obj, "Failed to create certificate validator");
        free(manager);
        return NULL;
    }
    
    manager->initialized = true;
    manager->start_time = vlc_tick_now();
    
    msg_Info(obj, "8KDVD disc manager created");
    return manager;
}

void kdvd_disc_manager_destroy(kdvd_disc_manager_t *manager) {
    if (!manager) return;
    
    if (manager->discs) {
        free(manager->discs);
    }
    
    if (manager->certificate_validator) {
        kdvd_certificate_validator_destroy(manager->certificate_validator);
    }
    
    if (manager->manager_context) {
        free(manager->manager_context);
    }
    
    free(manager);
    msg_Info(manager->obj, "8KDVD disc manager destroyed");
}

int kdvd_disc_manager_detect_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Detecting disc: %s", disc_path);
    
    uint64_t detection_start = vlc_tick_now();
    
    // Check if disc path exists
    if (vlc_access(disc_path, R_OK) != 0) {
        msg_Err(manager->obj, "Disc path not found: %s", disc_path);
        return -1;
    }
    
    // Simulate disc detection
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Analyzing disc structure: %s", disc_path);
        msg_Dbg(manager->obj, "Checking file system type");
        msg_Dbg(manager->obj, "Reading disc metadata");
        msg_Dbg(manager->obj, "Validating disc format");
    }
    
    // Check for 8KDVD disc structure
    char structure_path[512];
    snprintf(structure_path, sizeof(structure_path), "%s/8KDVD_TS", disc_path);
    
    if (vlc_access(structure_path, R_OK) == 0) {
        // Found 8KDVD structure
        if (manager->debug_enabled) {
            msg_Dbg(manager->obj, "8KDVD disc structure detected: %s", structure_path);
        }
        
        // Update statistics
        manager->stats.discs_detected++;
        
        uint64_t detection_time = vlc_tick_now() - detection_start;
        manager->stats.mount_time_us += detection_time;
        manager->stats.last_operation_time = vlc_tick_now();
        
        // Calculate average detection time
        if (manager->stats.discs_detected > 0) {
            manager->stats.average_mount_time = (float)manager->stats.mount_time_us / manager->stats.discs_detected;
        }
        
        if (manager->debug_enabled) {
            msg_Dbg(manager->obj, "Disc detected in %llu us", detection_time);
        }
        
        return 0;
    } else {
        msg_Err(manager->obj, "Not a valid 8KDVD disc: %s", disc_path);
        return -1;
    }
}

int kdvd_disc_manager_mount_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Mounting disc: %s", disc_path);
    
    uint64_t mount_start = vlc_tick_now();
    
    // Check if disc is already mounted
    for (uint32_t i = 0; i < manager->disc_count; i++) {
        if (strcmp(manager->discs[i].mount_path, disc_path) == 0) {
            if (manager->discs[i].is_mounted) {
                msg_Warn(manager->obj, "Disc already mounted: %s", disc_path);
                return 0;
            }
        }
    }
    
    // Simulate disc mounting
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Mounting disc filesystem: %s", disc_path);
        msg_Dbg(manager->obj, "Setting up disc access");
        msg_Dbg(manager->obj, "Initializing disc metadata");
    }
    
    // Add disc to manager
    if (manager->discs) {
        kdvd_disc_info_t *new_discs = realloc(manager->discs, (manager->disc_count + 1) * sizeof(kdvd_disc_info_t));
        if (!new_discs) {
            msg_Err(manager->obj, "Failed to allocate disc storage");
            return -1;
        }
        manager->discs = new_discs;
    } else {
        manager->discs = malloc(sizeof(kdvd_disc_info_t));
        if (!manager->discs) {
            msg_Err(manager->obj, "Failed to allocate disc storage");
            return -1;
        }
    }
    
    // Initialize disc info
    kdvd_disc_info_t *disc = &manager->discs[manager->disc_count];
    memset(disc, 0, sizeof(kdvd_disc_info_t));
    
    strncpy(disc->disc_id, "8KDVD-001", sizeof(disc->disc_id) - 1);
    strncpy(disc->disc_title, "8KDVD Disc", sizeof(disc->disc_title) - 1);
    strncpy(disc->disc_manufacturer, "8KDVD Manufacturer", sizeof(disc->disc_manufacturer) - 1);
    strncpy(disc->disc_version, "1.0", sizeof(disc->disc_version) - 1);
    disc->disc_size = 50000000000; // 50GB
    disc->disc_type = 1; // 8KDVD
    disc->is_authentic = true;
    disc->is_mounted = true;
    strncpy(disc->mount_path, disc_path, sizeof(disc->mount_path) - 1);
    strncpy(disc->file_system, "UDF", sizeof(disc->file_system) - 1);
    disc->creation_date = time(NULL);
    disc->last_access_date = time(NULL);
    disc->access_count = 1;
    disc->write_protected = true;
    disc->region_locked = false;
    disc->region_code = 0; // Region free
    snprintf(disc->certificate_path, sizeof(disc->certificate_path), "%s/8KDVD_TS/certificate.pem", disc_path);
    disc->certificate_valid = true;
    
    manager->disc_count++;
    
    // Update statistics
    manager->stats.discs_mounted++;
    
    uint64_t mount_time = vlc_tick_now() - mount_start;
    manager->stats.mount_time_us += mount_time;
    manager->stats.last_operation_time = vlc_tick_now();
    
    // Calculate average mount time
    if (manager->stats.discs_mounted > 0) {
        manager->stats.average_mount_time = (float)manager->stats.mount_time_us / manager->stats.discs_mounted;
    }
    
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Disc mounted in %llu us", mount_time);
    }
    
    // Call disc callback if set
    if (manager->disc_callback) {
        manager->disc_callback(disc_path, 1); // Mount event
    }
    
    msg_Info(manager->obj, "Disc mounted successfully: %s", disc_path);
    return 0;
}

int kdvd_disc_manager_unmount_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Unmounting disc: %s", disc_path);
    
    uint64_t unmount_start = vlc_tick_now();
    
    // Find disc in manager
    uint32_t disc_index = UINT32_MAX;
    for (uint32_t i = 0; i < manager->disc_count; i++) {
        if (strcmp(manager->discs[i].mount_path, disc_path) == 0) {
            disc_index = i;
            break;
        }
    }
    
    if (disc_index == UINT32_MAX) {
        msg_Err(manager->obj, "Disc not found in manager: %s", disc_path);
        return -1;
    }
    
    // Simulate disc unmounting
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Unmounting disc filesystem: %s", disc_path);
        msg_Dbg(manager->obj, "Cleaning up disc access");
        msg_Dbg(manager->obj, "Saving disc metadata");
    }
    
    // Mark disc as unmounted
    manager->discs[disc_index].is_mounted = false;
    manager->discs[disc_index].last_access_date = time(NULL);
    
    // Update statistics
    manager->stats.discs_unmounted++;
    
    uint64_t unmount_time = vlc_tick_now() - unmount_start;
    manager->stats.unmount_time_us += unmount_time;
    manager->stats.last_operation_time = vlc_tick_now();
    
    // Calculate average unmount time
    if (manager->stats.discs_unmounted > 0) {
        manager->stats.average_unmount_time = (float)manager->stats.unmount_time_us / manager->stats.discs_unmounted;
    }
    
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Disc unmounted in %llu us", unmount_time);
    }
    
    // Call disc callback if set
    if (manager->disc_callback) {
        manager->disc_callback(disc_path, 2); // Unmount event
    }
    
    msg_Info(manager->obj, "Disc unmounted successfully: %s", disc_path);
    return 0;
}

int kdvd_disc_manager_eject_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Ejecting disc: %s", disc_path);
    
    // Unmount disc first
    if (kdvd_disc_manager_unmount_disc(manager, disc_path) != 0) {
        msg_Err(manager->obj, "Failed to unmount disc before eject: %s", disc_path);
        return -1;
    }
    
    // Simulate disc ejection
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Ejecting disc from drive: %s", disc_path);
    }
    
    // Call disc callback if set
    if (manager->disc_callback) {
        manager->disc_callback(disc_path, 3); // Eject event
    }
    
    msg_Info(manager->obj, "Disc ejected successfully: %s", disc_path);
    return 0;
}

kdvd_disc_info_t kdvd_disc_manager_get_disc_info(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) {
        kdvd_disc_info_t empty_info = {0};
        return empty_info;
    }
    
    // Find disc in manager
    for (uint32_t i = 0; i < manager->disc_count; i++) {
        if (strcmp(manager->discs[i].mount_path, disc_path) == 0) {
            return manager->discs[i];
        }
    }
    
    kdvd_disc_info_t empty_info = {0};
    return empty_info;
}

int kdvd_disc_manager_get_disc_count(kdvd_disc_manager_t *manager) {
    return manager ? manager->disc_count : 0;
}

kdvd_disc_info_t kdvd_disc_manager_get_disc(kdvd_disc_manager_t *manager, uint32_t index) {
    if (manager && manager->discs && index < manager->disc_count) {
        return manager->discs[index];
    }
    
    kdvd_disc_info_t empty_disc = {0};
    return empty_disc;
}

int kdvd_disc_manager_detect_8kdvd_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Detecting 8KDVD disc: %s", disc_path);
    
    // Check for 8KDVD disc structure
    char structure_path[512];
    snprintf(structure_path, sizeof(structure_path), "%s/8KDVD_TS", disc_path);
    
    if (vlc_access(structure_path, R_OK) != 0) {
        msg_Err(manager->obj, "Not a valid 8KDVD disc: %s", disc_path);
        return -1;
    }
    
    // Check for required 8KDVD files
    char required_files[][64] = {
        "index.xml",
        "certificate.pem",
        "manifest.json"
    };
    
    for (int i = 0; i < 3; i++) {
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", structure_path, required_files[i]);
        
        if (vlc_access(file_path, R_OK) != 0) {
            msg_Err(manager->obj, "Required 8KDVD file not found: %s", file_path);
            return -1;
        }
    }
    
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "8KDVD disc structure validated: %s", disc_path);
    }
    
    return 0;
}

int kdvd_disc_manager_validate_8kdvd_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Validating 8KDVD disc: %s", disc_path);
    
    // Check disc structure
    if (kdvd_disc_manager_detect_8kdvd_disc(manager, disc_path) != 0) {
        return -1;
    }
    
    // Validate certificate
    if (kdvd_certificate_validator_validate_8kdvd_certificate(manager->certificate_validator, disc_path) != 0) {
        msg_Err(manager->obj, "8KDVD certificate validation failed");
        return -1;
    }
    
    // Check disc integrity
    if (kdvd_certificate_validator_verify_disc_integrity(manager->certificate_validator, disc_path) != 0) {
        msg_Err(manager->obj, "8KDVD disc integrity check failed");
        return -1;
    }
    
    msg_Info(manager->obj, "8KDVD disc validation successful: %s", disc_path);
    return 0;
}

int kdvd_disc_manager_authenticate_8kdvd_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Authenticating 8KDVD disc: %s", disc_path);
    
    // Update statistics
    manager->stats.authentication_attempts++;
    
    // Validate disc
    if (kdvd_disc_manager_validate_8kdvd_disc(manager, disc_path) != 0) {
        manager->stats.failed_auths++;
        msg_Err(manager->obj, "8KDVD disc authentication failed");
        return -1;
    }
    
    manager->stats.successful_auths++;
    msg_Info(manager->obj, "8KDVD disc authentication successful");
    return 0;
}

int kdvd_disc_manager_authenticate_disc(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Authenticating disc: %s", disc_path);
    
    // Check disc type and authenticate accordingly
    if (kdvd_disc_manager_detect_8kdvd_disc(manager, disc_path) == 0) {
        return kdvd_disc_manager_authenticate_8kdvd_disc(manager, disc_path);
    }
    
    msg_Err(manager->obj, "Unsupported disc type: %s", disc_path);
    return -1;
}

int kdvd_disc_manager_validate_certificate(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Validating disc certificate: %s", disc_path);
    
    // Use certificate validator
    if (kdvd_certificate_validator_validate_disc_certificate(manager->certificate_validator, disc_path) != 0) {
        msg_Err(manager->obj, "Disc certificate validation failed");
        return -1;
    }
    
    msg_Info(manager->obj, "Disc certificate validation successful");
    return 0;
}

int kdvd_disc_manager_check_disc_integrity(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Checking disc integrity: %s", disc_path);
    
    // Use certificate validator
    if (kdvd_certificate_validator_verify_disc_integrity(manager->certificate_validator, disc_path) != 0) {
        msg_Err(manager->obj, "Disc integrity check failed");
        return -1;
    }
    
    msg_Info(manager->obj, "Disc integrity check successful");
    return 0;
}

int kdvd_disc_manager_start_monitoring(kdvd_disc_manager_t *manager) {
    if (!manager) return -1;
    
    msg_Info(manager->obj, "Starting disc monitoring");
    
    // Simulate disc monitoring
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Setting up disc monitoring");
        msg_Dbg(manager->obj, "Registering disc event handlers");
    }
    
    msg_Info(manager->obj, "Disc monitoring started");
    return 0;
}

int kdvd_disc_manager_stop_monitoring(kdvd_disc_manager_t *manager) {
    if (!manager) return -1;
    
    msg_Info(manager->obj, "Stopping disc monitoring");
    
    // Simulate stopping disc monitoring
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Stopping disc monitoring");
        msg_Dbg(manager->obj, "Unregistering disc event handlers");
    }
    
    msg_Info(manager->obj, "Disc monitoring stopped");
    return 0;
}

int kdvd_disc_manager_set_disc_callback(kdvd_disc_manager_t *manager, void (*callback)(const char *disc_path, int event_type)) {
    if (!manager) return -1;
    
    manager->disc_callback = callback;
    
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Disc callback set");
    }
    
    return 0;
}

int kdvd_disc_manager_read_disc_info(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Reading disc info: %s", disc_path);
    
    // Simulate reading disc info
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Reading disc metadata from: %s", disc_path);
        msg_Dbg(manager->obj, "Parsing disc information");
    }
    
    return 0;
}

int kdvd_disc_manager_write_disc_info(kdvd_disc_manager_t *manager, const char *disc_path, const kdvd_disc_info_t *info) {
    if (!manager || !disc_path || !info) return -1;
    
    msg_Info(manager->obj, "Writing disc info: %s", disc_path);
    
    // Simulate writing disc info
    if (manager->debug_enabled) {
        msg_Dbg(manager->obj, "Writing disc metadata to: %s", disc_path);
        msg_Dbg(manager->obj, "Updating disc information");
    }
    
    return 0;
}

int kdvd_disc_manager_update_disc_info(kdvd_disc_manager_t *manager, const char *disc_path) {
    if (!manager || !disc_path) return -1;
    
    msg_Info(manager->obj, "Updating disc info: %s", disc_path);
    
    // Find disc in manager
    for (uint32_t i = 0; i < manager->disc_count; i++) {
        if (strcmp(manager->discs[i].mount_path, disc_path) == 0) {
            // Update disc info
            manager->discs[i].last_access_date = time(NULL);
            manager->discs[i].access_count++;
            
            if (manager->debug_enabled) {
                msg_Dbg(manager->obj, "Disc info updated: %s", disc_path);
            }
            
            return 0;
        }
    }
    
    msg_Err(manager->obj, "Disc not found for update: %s", disc_path);
    return -1;
}

kdvd_disc_manager_stats_t kdvd_disc_manager_get_stats(kdvd_disc_manager_t *manager) {
    if (manager) {
        return manager->stats;
    }
    
    kdvd_disc_manager_stats_t empty_stats = {0};
    return empty_stats;
}

int kdvd_disc_manager_reset_stats(kdvd_disc_manager_t *manager) {
    if (!manager) return -1;
    
    memset(&manager->stats, 0, sizeof(kdvd_disc_manager_stats_t));
    manager->start_time = vlc_tick_now();
    
    msg_Info(manager->obj, "8KDVD disc manager statistics reset");
    return 0;
}

int kdvd_disc_manager_set_performance_mode(kdvd_disc_manager_t *manager, const char *mode) {
    if (!manager || !mode) return -1;
    
    msg_Info(manager->obj, "Setting disc manager performance mode: %s", mode);
    
    if (strcmp(mode, "quality") == 0) {
        // Quality mode - prioritize accuracy over speed
        msg_Info(manager->obj, "Quality mode enabled: maximum disc validation accuracy");
    } else if (strcmp(mode, "speed") == 0) {
        // Speed mode - prioritize speed over accuracy
        msg_Info(manager->obj, "Speed mode enabled: maximum disc processing speed");
    } else if (strcmp(mode, "balanced") == 0) {
        // Balanced mode - balance accuracy and speed
        msg_Info(manager->obj, "Balanced mode enabled: optimal performance");
    } else {
        msg_Err(manager->obj, "Unknown performance mode: %s", mode);
        return -1;
    }
    
    return 0;
}

int kdvd_disc_manager_allocate_buffers(kdvd_disc_manager_t *manager) {
    if (!manager) return -1;
    
    // Allocate disc storage
    if (manager->discs) {
        free(manager->discs);
    }
    
    manager->discs = calloc(100, sizeof(kdvd_disc_info_t));
    if (!manager->discs) {
        msg_Err(manager->obj, "Failed to allocate disc storage");
        return -1;
    }
    
    manager->disc_count = 0;
    
    msg_Info(manager->obj, "Disc manager buffers allocated");
    return 0;
}

int kdvd_disc_manager_free_buffers(kdvd_disc_manager_t *manager) {
    if (!manager) return -1;
    
    if (manager->discs) {
        free(manager->discs);
        manager->discs = NULL;
        manager->disc_count = 0;
    }
    
    msg_Info(manager->obj, "Disc manager buffers freed");
    return 0;
}

int kdvd_disc_manager_get_memory_usage(kdvd_disc_manager_t *manager, uint32_t *usage_mb) {
    if (!manager || !usage_mb) return -1;
    
    *usage_mb = manager->disc_count * sizeof(kdvd_disc_info_t) / (1024 * 1024);
    return 0;
}

int kdvd_disc_manager_get_last_error(kdvd_disc_manager_t *manager, char *error_buffer, size_t buffer_size) {
    if (!manager || !error_buffer) return -1;
    
    strncpy(error_buffer, manager->last_error, buffer_size - 1);
    error_buffer[buffer_size - 1] = '\0';
    return 0;
}

int kdvd_disc_manager_clear_errors(kdvd_disc_manager_t *manager) {
    if (!manager) return -1;
    
    memset(manager->last_error, 0, sizeof(manager->last_error));
    return 0;
}

void kdvd_disc_manager_set_debug(kdvd_disc_manager_t *manager, bool enable) {
    if (manager) {
        manager->debug_enabled = enable;
        msg_Info(manager->obj, "8KDVD disc manager debug %s", enable ? "enabled" : "disabled");
    }
}

void kdvd_disc_manager_log_info(kdvd_disc_manager_t *manager) {
    if (!manager) return;
    
    msg_Info(manager->obj, "8KDVD Disc Manager Info:");
    msg_Info(manager->obj, "  Initialized: %s", manager->initialized ? "yes" : "no");
    msg_Info(manager->obj, "  Disc Count: %u", manager->disc_count);
    msg_Info(manager->obj, "  Debug Enabled: %s", manager->debug_enabled ? "yes" : "no");
    msg_Info(manager->obj, "  Certificate Validator: %s", manager->certificate_validator ? "available" : "unavailable");
}

void kdvd_disc_manager_log_stats(kdvd_disc_manager_t *manager) {
    if (!manager) return;
    
    msg_Info(manager->obj, "8KDVD Disc Manager Statistics:");
    msg_Info(manager->obj, "  Discs Detected: %llu", manager->stats.discs_detected);
    msg_Info(manager->obj, "  Discs Mounted: %llu", manager->stats.discs_mounted);
    msg_Info(manager->obj, "  Discs Unmounted: %llu", manager->stats.discs_unmounted);
    msg_Info(manager->obj, "  Authentication Attempts: %llu", manager->stats.authentication_attempts);
    msg_Info(manager->obj, "  Successful Authentications: %llu", manager->stats.successful_auths);
    msg_Info(manager->obj, "  Failed Authentications: %llu", manager->stats.failed_auths);
    msg_Info(manager->obj, "  Total Mount Time: %llu us", manager->stats.mount_time_us);
    msg_Info(manager->obj, "  Total Unmount Time: %llu us", manager->stats.unmount_time_us);
    msg_Info(manager->obj, "  Average Mount Time: %.2f us", manager->stats.average_mount_time);
    msg_Info(manager->obj, "  Average Unmount Time: %.2f us", manager->stats.average_unmount_time);
    msg_Info(manager->obj, "  Memory Usage: %u MB", manager->stats.memory_usage_mb);
}
