#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_input.h>
#include <vlc_access.h>
#include <vlc_fs.h>
#include <vlc_messages.h>
#include "8kdvd_disc_manager.h"

// 8KDVD Disc Input Module
static int Open(vlc_object_t *obj);
static void Close(vlc_object_t *obj);
static int Control(input_thread_t *input, int query, va_list args);

// VLC Module Descriptor
vlc_module_begin()
    set_shortname("8KDVD Disc")
    set_description("8KDVD Disc Input Module")
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_ACCESS)
    set_capability("access", 0)
    set_callbacks(Open, Close)
    add_shortcut("8kdvd-disc")
vlc_module_end()

// 8KDVD Disc Input Context
typedef struct {
    kdvd_disc_manager_t *disc_manager;
    kdvd_disc_info_t disc_info;
    bool disc_mounted;
    char disc_path[512];
    uint64_t disc_size;
    uint64_t current_position;
    bool is_authenticated;
} kdvd_disc_input_t;

static int Open(vlc_object_t *obj) {
    access_t *access = (access_t *)obj;
    kdvd_disc_input_t *ctx = calloc(1, sizeof(kdvd_disc_input_t));
    if (!ctx) return VLC_ENOMEM;
    
    access->p_sys = ctx;
    
    // Get disc path
    strncpy(ctx->disc_path, access->psz_location, sizeof(ctx->disc_path) - 1);
    ctx->disc_path[sizeof(ctx->disc_path) - 1] = '\0';
    
    msg_Info(access, "Opening 8KDVD disc: %s", ctx->disc_path);
    
    // Create disc manager
    ctx->disc_manager = kdvd_disc_manager_create(VLC_OBJECT(access));
    if (!ctx->disc_manager) {
        msg_Err(access, "Failed to create 8KDVD disc manager");
        free(ctx);
        return VLC_EGENERIC;
    }
    
    // Detect disc
    if (kdvd_disc_manager_detect_disc(ctx->disc_manager, ctx->disc_path) != 0) {
        msg_Err(access, "Failed to detect 8KDVD disc: %s", ctx->disc_path);
        kdvd_disc_manager_destroy(ctx->disc_manager);
        free(ctx);
        return VLC_EGENERIC;
    }
    
    // Mount disc
    if (kdvd_disc_manager_mount_disc(ctx->disc_manager, ctx->disc_path) != 0) {
        msg_Err(access, "Failed to mount 8KDVD disc: %s", ctx->disc_path);
        kdvd_disc_manager_destroy(ctx->disc_manager);
        free(ctx);
        return VLC_EGENERIC;
    }
    
    ctx->disc_mounted = true;
    
    // Get disc info
    ctx->disc_info = kdvd_disc_manager_get_disc_info(ctx->disc_manager, ctx->disc_path);
    ctx->disc_size = ctx->disc_info.disc_size;
    ctx->current_position = 0;
    
    // Authenticate disc
    if (kdvd_disc_manager_authenticate_disc(ctx->disc_manager, ctx->disc_path) != 0) {
        msg_Err(access, "Failed to authenticate 8KDVD disc: %s", ctx->disc_path);
        kdvd_disc_manager_unmount_disc(ctx->disc_manager, ctx->disc_path);
        kdvd_disc_manager_destroy(ctx->disc_manager);
        free(ctx);
        return VLC_EGENERIC;
    }
    
    ctx->is_authenticated = true;
    
    // Set up access callbacks
    access->pf_read = NULL; // No direct reading
    access->pf_seek = NULL; // No direct seeking
    access->pf_control = Control;
    
    msg_Info(access, "8KDVD disc opened successfully: %s", ctx->disc_path);
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj) {
    access_t *access = (access_t *)obj;
    kdvd_disc_input_t *ctx = access->p_sys;
    
    if (!ctx) return;
    
    msg_Info(access, "Closing 8KDVD disc: %s", ctx->disc_path);
    
    // Unmount disc if mounted
    if (ctx->disc_mounted && ctx->disc_manager) {
        kdvd_disc_manager_unmount_disc(ctx->disc_manager, ctx->disc_path);
    }
    
    // Destroy disc manager
    if (ctx->disc_manager) {
        kdvd_disc_manager_destroy(ctx->disc_manager);
    }
    
    free(ctx);
    access->p_sys = NULL;
    
    msg_Info(access, "8KDVD disc closed");
}

static int Control(input_thread_t *input, int query, va_list args) {
    access_t *access = input->p_access;
    kdvd_disc_input_t *ctx = access->p_sys;
    
    if (!ctx) return VLC_EGENERIC;
    
    switch (query) {
        case ACCESS_CAN_SEEK:
            *va_arg(args, bool *) = true;
            break;
            
        case ACCESS_CAN_FASTSEEK:
            *va_arg(args, bool *) = true;
            break;
            
        case ACCESS_CAN_PAUSE:
            *va_arg(args, bool *) = true;
            break;
            
        case ACCESS_CAN_CONTROL_PACE:
            *va_arg(args, bool *) = true;
            break;
            
        case ACCESS_GET_SIZE:
            *va_arg(args, uint64_t *) = ctx->disc_size;
            break;
            
        case ACCESS_GET_PTS_DELAY:
            *va_arg(args, int64_t *) = 0;
            break;
            
        case ACCESS_SET_PAUSE_STATE:
            // Handle pause state
            break;
            
        case ACCESS_GET_TITLE_INFO:
            // Return disc title info
            break;
            
        case ACCESS_GET_CHAPTERS_COUNT:
            // Return chapters count
            break;
            
        default:
            return VLC_EGENERIC;
    }
    
    return VLC_SUCCESS;
}
