#include "8k_video_renderer.h"
#include <vlc_messages.h>
#include <vlc_picture.h>
#include <vlc_fourcc.h>
#include <vlc_window.h>
#include <vlc_opengl.h>
#include <vlc_gl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// 8K Video Renderer Implementation
struct kdvd_8k_renderer_t {
    vlc_object_t *obj;
    kdvd_8k_render_config_t config;
    kdvd_8k_render_stats_t stats;
    bool initialized;
    bool hardware_acceleration;
    bool hdr_enabled;
    bool dolby_vision_enabled;
    bool debug_enabled;
    char last_error[256];
    void *render_context;  // Placeholder for actual render context
    void *gpu_context;      // Placeholder for GPU context
    uint8_t *frame_buffer;
    size_t frame_buffer_size;
    uint32_t current_width;
    uint32_t current_height;
    uint32_t current_bit_depth;
    uint64_t start_time;
    uint64_t last_frame_time;
    bool vsync_active;
    bool adaptive_sync_active;
};

// 8K Video Renderer Functions
kdvd_8k_renderer_t* kdvd_8k_renderer_create(vlc_object_t *obj) {
    kdvd_8k_renderer_t *renderer = calloc(1, sizeof(kdvd_8k_renderer_t));
    if (!renderer) return NULL;
    
    renderer->obj = obj;
    renderer->initialized = false;
    renderer->hardware_acceleration = false;
    renderer->hdr_enabled = false;
    renderer->dolby_vision_enabled = false;
    renderer->debug_enabled = false;
    renderer->render_context = NULL;
    renderer->gpu_context = NULL;
    renderer->frame_buffer = NULL;
    renderer->frame_buffer_size = 0;
    renderer->current_width = 0;
    renderer->current_height = 0;
    renderer->current_bit_depth = 0;
    renderer->start_time = 0;
    renderer->last_frame_time = 0;
    renderer->vsync_active = false;
    renderer->adaptive_sync_active = false;
    
    // Initialize stats
    memset(&renderer->stats, 0, sizeof(kdvd_8k_render_stats_t));
    
    // Initialize config with 8K defaults
    renderer->config.width = 7680;
    renderer->config.height = 4320;
    renderer->config.bit_depth = 10;
    renderer->config.frame_rate = 60;
    renderer->config.hdr_enabled = false;
    renderer->config.dolby_vision_enabled = false;
    renderer->config.hardware_acceleration = false;
    renderer->config.color_space = 1;  // BT.2020
    renderer->config.color_range = 1;  // Full range
    renderer->config.chroma_subsampling = 1;  // 4:2:0
    renderer->config.vsync_enabled = true;
    renderer->config.triple_buffering = true;
    renderer->config.max_fps = 60;
    renderer->config.adaptive_sync = true;
    
    msg_Info(obj, "8K video renderer created");
    return renderer;
}

void kdvd_8k_renderer_destroy(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return;
    
    if (renderer->frame_buffer) {
        free(renderer->frame_buffer);
    }
    
    if (renderer->render_context) {
        // Clean up render context
        free(renderer->render_context);
    }
    
    if (renderer->gpu_context) {
        // Clean up GPU context
        free(renderer->gpu_context);
    }
    
    free(renderer);
    msg_Info(renderer->obj, "8K video renderer destroyed");
}

int kdvd_8k_renderer_configure(kdvd_8k_renderer_t *renderer, const kdvd_8k_render_config_t *config) {
    if (!renderer || !config) return -1;
    
    msg_Info(renderer->obj, "Configuring 8K video renderer: %ux%u %u-bit %u FPS", 
             config->width, config->height, config->bit_depth, config->frame_rate);
    
    // Copy configuration
    memcpy(&renderer->config, config, sizeof(kdvd_8k_render_config_t));
    
    // Validate 8K configuration
    if (config->width != 7680 || config->height != 4320) {
        msg_Err(renderer->obj, "Invalid 8K resolution: %ux%u (expected 7680x4320)", 
                config->width, config->height);
        return -1;
    }
    
    if (config->bit_depth != 10 && config->bit_depth != 12) {
        msg_Err(renderer->obj, "Invalid bit depth: %u (expected 10 or 12 for HDR)", config->bit_depth);
        return -1;
    }
    
    if (config->frame_rate != 60) {
        msg_Err(renderer->obj, "Invalid frame rate: %u (expected 60 FPS)", config->frame_rate);
        return -1;
    }
    
    // Allocate frame buffer for 8K
    size_t buffer_size = config->width * config->height * 4; // RGBA for 10/12-bit
    if (renderer->frame_buffer_size < buffer_size) {
        if (renderer->frame_buffer) {
            free(renderer->frame_buffer);
        }
        renderer->frame_buffer = malloc(buffer_size);
        if (!renderer->frame_buffer) {
            msg_Err(renderer->obj, "Failed to allocate 8K frame buffer");
            return -1;
        }
        renderer->frame_buffer_size = buffer_size;
    }
    
    renderer->current_width = config->width;
    renderer->current_height = config->height;
    renderer->current_bit_depth = config->bit_depth;
    
    // Initialize render context
    if (renderer->render_context) {
        free(renderer->render_context);
    }
    renderer->render_context = malloc(2048); // Placeholder for actual render context
    
    // Initialize GPU context
    if (renderer->gpu_context) {
        free(renderer->gpu_context);
    }
    renderer->gpu_context = malloc(1024); // Placeholder for actual GPU context
    
    renderer->initialized = true;
    renderer->start_time = vlc_tick_now();
    
    msg_Info(renderer->obj, "8K video renderer configured successfully");
    return 0;
}

int kdvd_8k_renderer_set_8k_mode(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    if (enable) {
        renderer->config.width = 7680;
        renderer->config.height = 4320;
        renderer->config.bit_depth = 10;
        renderer->config.frame_rate = 60;
        renderer->config.hdr_enabled = true;
        renderer->config.color_space = 1;  // BT.2020
        renderer->config.color_range = 1;  // Full range
        msg_Info(renderer->obj, "8K mode enabled: 7680x4320 10-bit HDR 60 FPS");
    } else {
        renderer->config.width = 1920;
        renderer->config.height = 1080;
        renderer->config.bit_depth = 8;
        renderer->config.frame_rate = 30;
        renderer->config.hdr_enabled = false;
        renderer->config.color_space = 0;  // BT.709
        renderer->config.color_range = 0;  // Limited range
        msg_Info(renderer->obj, "8K mode disabled: 1920x1080 8-bit 30 FPS");
    }
    
    return 0;
}

int kdvd_8k_renderer_set_hdr_mode(kdvd_8k_renderer_t *renderer, bool hdr_enabled, bool dolby_vision_enabled) {
    if (!renderer) return -1;
    
    renderer->hdr_enabled = hdr_enabled;
    renderer->dolby_vision_enabled = dolby_vision_enabled;
    renderer->config.hdr_enabled = hdr_enabled;
    renderer->config.dolby_vision_enabled = dolby_vision_enabled;
    
    if (hdr_enabled) {
        renderer->config.bit_depth = 10;
        renderer->config.color_space = 1;  // BT.2020
        renderer->config.color_range = 1;  // Full range
        msg_Info(renderer->obj, "HDR enabled for 8K renderer");
    }
    
    if (dolby_vision_enabled) {
        renderer->config.bit_depth = 12;
        renderer->config.color_space = 1;  // BT.2020
        renderer->config.color_range = 1;  // Full range
        msg_Info(renderer->obj, "Dolby Vision enabled for 8K renderer");
    }
    
    return 0;
}

int kdvd_8k_renderer_set_hardware_acceleration(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    renderer->hardware_acceleration = enable;
    renderer->config.hardware_acceleration = enable;
    
    msg_Info(renderer->obj, "8K hardware acceleration %s", enable ? "enabled" : "disabled");
    return 0;
}

int kdvd_8k_renderer_render_frame(kdvd_8k_renderer_t *renderer, picture_t *picture) {
    if (!renderer || !picture) return -1;
    
    if (!renderer->initialized) {
        msg_Err(renderer->obj, "8K renderer not initialized");
        return -1;
    }
    
    uint64_t render_start = vlc_tick_now();
    
    // Simulate 8K frame rendering
    if (renderer->debug_enabled) {
        msg_Dbg(renderer->obj, "Rendering 8K frame: %ux%u %u-bit", 
               picture->format.i_width, picture->format.i_height, picture->format.i_bits_per_pixel);
    }
    
    // Validate picture format
    if (picture->format.i_width != renderer->current_width || 
        picture->format.i_height != renderer->current_height) {
        msg_Err(renderer->obj, "Picture resolution mismatch: %ux%u != %ux%u", 
               picture->format.i_width, picture->format.i_height,
               renderer->current_width, renderer->current_height);
        return -1;
    }
    
    // Simulate hardware-accelerated rendering
    if (renderer->hardware_acceleration) {
        // GPU-accelerated rendering
        if (renderer->debug_enabled) {
            msg_Dbg(renderer->obj, "Using hardware acceleration for 8K rendering");
        }
        
        // Simulate GPU processing
        if (renderer->hdr_enabled) {
            // HDR tone mapping
            if (renderer->debug_enabled) {
                msg_Dbg(renderer->obj, "Applying HDR tone mapping");
            }
        }
        
        if (renderer->dolby_vision_enabled) {
            // Dolby Vision processing
            if (renderer->debug_enabled) {
                msg_Dbg(renderer->obj, "Applying Dolby Vision processing");
            }
        }
        
    } else {
        // Software rendering
        if (renderer->debug_enabled) {
            msg_Dbg(renderer->obj, "Using software rendering for 8K");
        }
    }
    
    // Simulate frame buffer processing
    size_t buffer_size = renderer->current_width * renderer->current_height * 4;
    if (renderer->frame_buffer_size < buffer_size) {
        if (renderer->frame_buffer) {
            free(renderer->frame_buffer);
        }
        renderer->frame_buffer = malloc(buffer_size);
        if (!renderer->frame_buffer) {
            msg_Err(renderer->obj, "Failed to allocate 8K frame buffer");
            return -1;
        }
        renderer->frame_buffer_size = buffer_size;
    }
    
    // Copy picture data to frame buffer (simplified)
    // In real implementation, this would involve proper format conversion
    memset(renderer->frame_buffer, 0, buffer_size);
    
    // Update statistics
    renderer->stats.frames_rendered++;
    
    uint64_t render_time = vlc_tick_now() - render_start;
    renderer->stats.render_time_us += render_time;
    renderer->stats.last_frame_time = vlc_tick_now();
    
    // Calculate average FPS
    if (renderer->stats.frames_rendered > 0) {
        uint64_t total_time = vlc_tick_now() - renderer->start_time;
        renderer->stats.average_fps = (float)renderer->stats.frames_rendered * 1000000.0f / total_time;
    }
    
    // Calculate average render time
    if (renderer->stats.frames_rendered > 0) {
        renderer->stats.average_render_time = (float)renderer->stats.render_time_us / renderer->stats.frames_rendered;
    }
    
    // Simulate GPU usage
    renderer->stats.gpu_usage_percent = 85.0f + (rand() % 15); // 85-100%
    
    // Simulate memory usage
    renderer->stats.memory_usage_mb = buffer_size / (1024 * 1024);
    
    if (renderer->debug_enabled) {
        msg_Dbg(renderer->obj, "8K frame rendered in %llu us", render_time);
    }
    
    return 0;
}

int kdvd_8k_renderer_present_frame(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    if (!renderer->initialized) {
        msg_Err(renderer->obj, "8K renderer not initialized");
        return -1;
    }
    
    // Simulate frame presentation
    if (renderer->debug_enabled) {
        msg_Dbg(renderer->obj, "Presenting 8K frame to display");
    }
    
    // Simulate V-Sync
    if (renderer->vsync_active) {
        if (renderer->debug_enabled) {
            msg_Dbg(renderer->obj, "V-Sync active");
        }
    }
    
    // Simulate Adaptive Sync
    if (renderer->adaptive_sync_active) {
        if (renderer->debug_enabled) {
            msg_Dbg(renderer->obj, "Adaptive sync active");
        }
    }
    
    return 0;
}

int kdvd_8k_renderer_clear_screen(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    if (renderer->frame_buffer) {
        memset(renderer->frame_buffer, 0, renderer->frame_buffer_size);
    }
    
    if (renderer->debug_enabled) {
        msg_Dbg(renderer->obj, "8K screen cleared");
    }
    
    return 0;
}

int kdvd_8k_renderer_flush(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    msg_Info(renderer->obj, "Flushing 8K renderer");
    
    // Reset render state
    renderer->stats.dropped_frames = 0;
    
    return 0;
}

int kdvd_8k_renderer_enable_8k_rendering(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    if (enable) {
        renderer->config.width = 7680;
        renderer->config.height = 4320;
        renderer->config.bit_depth = 10;
        renderer->config.frame_rate = 60;
        renderer->config.hdr_enabled = true;
        renderer->config.color_space = 1;  // BT.2020
        renderer->config.color_range = 1;  // Full range
        msg_Info(renderer->obj, "8K rendering enabled: 7680x4320 10-bit HDR 60 FPS");
    } else {
        renderer->config.width = 1920;
        renderer->config.height = 1080;
        renderer->config.bit_depth = 8;
        renderer->config.frame_rate = 30;
        renderer->config.hdr_enabled = false;
        renderer->config.color_space = 0;  // BT.709
        renderer->config.color_range = 0;  // Limited range
        msg_Info(renderer->obj, "8K rendering disabled: 1920x1080 8-bit 30 FPS");
    }
    
    return 0;
}

int kdvd_8k_renderer_set_8k_resolution(kdvd_8k_renderer_t *renderer, uint32_t width, uint32_t height) {
    if (!renderer) return -1;
    
    if (width != 7680 || height != 4320) {
        msg_Err(renderer->obj, "Invalid 8K resolution: %ux%u (expected 7680x4320)", width, height);
        return -1;
    }
    
    renderer->config.width = width;
    renderer->config.height = height;
    renderer->current_width = width;
    renderer->current_height = height;
    
    msg_Info(renderer->obj, "8K resolution set: %ux%u", width, height);
    return 0;
}

int kdvd_8k_renderer_optimize_for_8k(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    msg_Info(renderer->obj, "Optimizing renderer for 8K");
    
    // Enable hardware acceleration for 8K
    renderer->hardware_acceleration = true;
    renderer->config.hardware_acceleration = true;
    
    // Set optimal settings for 8K
    renderer->config.width = 7680;
    renderer->config.height = 4320;
    renderer->config.bit_depth = 10;
    renderer->config.frame_rate = 60;
    renderer->config.hdr_enabled = true;
    renderer->config.color_space = 1;  // BT.2020
    renderer->config.color_range = 1;  // Full range
    renderer->config.vsync_enabled = true;
    renderer->config.triple_buffering = true;
    renderer->config.adaptive_sync = true;
    
    // Allocate larger buffers for 8K
    size_t buffer_size = renderer->config.width * renderer->config.height * 4;
    if (renderer->frame_buffer_size < buffer_size) {
        if (renderer->frame_buffer) {
            free(renderer->frame_buffer);
        }
        renderer->frame_buffer = malloc(buffer_size);
        if (!renderer->frame_buffer) {
            msg_Err(renderer->obj, "Failed to allocate 8K frame buffer");
            return -1;
        }
        renderer->frame_buffer_size = buffer_size;
    }
    
    msg_Info(renderer->obj, "Renderer optimized for 8K");
    return 0;
}

int kdvd_8k_renderer_enable_hdr(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    renderer->hdr_enabled = enable;
    renderer->config.hdr_enabled = enable;
    
    if (enable) {
        renderer->config.bit_depth = 10;
        renderer->config.color_space = 1;  // BT.2020
        renderer->config.color_range = 1;  // Full range
        msg_Info(renderer->obj, "HDR enabled for 8K renderer");
    } else {
        renderer->config.bit_depth = 8;
        renderer->config.color_space = 0;  // BT.709
        renderer->config.color_range = 0;  // Limited range
        msg_Info(renderer->obj, "HDR disabled for 8K renderer");
    }
    
    return 0;
}

int kdvd_8k_renderer_enable_dolby_vision(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    renderer->dolby_vision_enabled = enable;
    renderer->config.dolby_vision_enabled = enable;
    
    if (enable) {
        renderer->config.bit_depth = 12;  // Dolby Vision requires 12-bit
        renderer->config.color_space = 1;  // BT.2020
        renderer->config.color_range = 1;  // Full range
        msg_Info(renderer->obj, "Dolby Vision enabled for 8K renderer");
    } else {
        renderer->config.bit_depth = 10;  // Back to 10-bit HDR
        msg_Info(renderer->obj, "Dolby Vision disabled for 8K renderer");
    }
    
    return 0;
}

int kdvd_8k_renderer_set_color_space(kdvd_8k_renderer_t *renderer, uint32_t color_space) {
    if (!renderer) return -1;
    
    renderer->config.color_space = color_space;
    
    const char *color_space_name = "Unknown";
    switch (color_space) {
        case 0: color_space_name = "BT.709"; break;
        case 1: color_space_name = "BT.2020"; break;
        case 2: color_space_name = "BT.601"; break;
        case 3: color_space_name = "SMPTE-240M"; break;
        case 4: color_space_name = "SMPTE-2085"; break;
        case 5: color_space_name = "SMPTE-432"; break;
    }
    
    msg_Info(renderer->obj, "Color space set to %s", color_space_name);
    return 0;
}

int kdvd_8k_renderer_set_color_range(kdvd_8k_renderer_t *renderer, uint32_t color_range) {
    if (!renderer) return -1;
    
    renderer->config.color_range = color_range;
    
    const char *range_name = (color_range == 0) ? "Limited" : "Full";
    msg_Info(renderer->obj, "Color range set to %s", range_name);
    return 0;
}

int kdvd_8k_renderer_detect_hardware_support(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    msg_Info(renderer->obj, "Detecting hardware support for 8K rendering");
    
    // Simulate hardware detection
    bool nvenc_support = false;
    bool quicksync_support = false;
    bool amf_support = false;
    
    // TODO: Implement actual hardware detection
    // This would check for NVIDIA, Intel, and AMD hardware support
    
    if (nvenc_support) {
        msg_Info(renderer->obj, "NVIDIA NVENC support detected");
    }
    if (quicksync_support) {
        msg_Info(renderer->obj, "Intel Quick Sync support detected");
    }
    if (amf_support) {
        msg_Info(renderer->obj, "AMD AMF support detected");
    }
    
    if (!nvenc_support && !quicksync_support && !amf_support) {
        msg_Warn(renderer->obj, "No hardware acceleration support detected for 8K rendering");
    }
    
    return 0;
}

int kdvd_8k_renderer_enable_nvenc(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    if (enable) {
        renderer->hardware_acceleration = true;
        msg_Info(renderer->obj, "NVIDIA NVENC enabled for 8K rendering");
    } else {
        renderer->hardware_acceleration = false;
        msg_Info(renderer->obj, "NVIDIA NVENC disabled for 8K rendering");
    }
    
    return 0;
}

int kdvd_8k_renderer_enable_quicksync(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    if (enable) {
        renderer->hardware_acceleration = true;
        msg_Info(renderer->obj, "Intel Quick Sync enabled for 8K rendering");
    } else {
        renderer->hardware_acceleration = false;
        msg_Info(renderer->obj, "Intel Quick Sync disabled for 8K rendering");
    }
    
    return 0;
}

int kdvd_8k_renderer_enable_amf(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    if (enable) {
        renderer->hardware_acceleration = true;
        msg_Info(renderer->obj, "AMD AMF enabled for 8K rendering");
    } else {
        renderer->hardware_acceleration = false;
        msg_Info(renderer->obj, "AMD AMF disabled for 8K rendering");
    }
    
    return 0;
}

int kdvd_8k_renderer_enable_vsync(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    renderer->vsync_active = enable;
    renderer->config.vsync_enabled = enable;
    
    msg_Info(renderer->obj, "V-Sync %s for 8K rendering", enable ? "enabled" : "disabled");
    return 0;
}

int kdvd_8k_renderer_enable_adaptive_sync(kdvd_8k_renderer_t *renderer, bool enable) {
    if (!renderer) return -1;
    
    renderer->adaptive_sync_active = enable;
    renderer->config.adaptive_sync = enable;
    
    msg_Info(renderer->obj, "Adaptive sync %s for 8K rendering", enable ? "enabled" : "disabled");
    return 0;
}

int kdvd_8k_renderer_set_max_fps(kdvd_8k_renderer_t *renderer, uint32_t max_fps) {
    if (!renderer) return -1;
    
    if (max_fps > 120) {
        msg_Err(renderer->obj, "Invalid max FPS: %u (max 120)", max_fps);
        return -1;
    }
    
    renderer->config.max_fps = max_fps;
    renderer->stats.current_fps = max_fps;
    
    msg_Info(renderer->obj, "Max FPS set to: %u", max_fps);
    return 0;
}

kdvd_8k_render_stats_t kdvd_8k_renderer_get_stats(kdvd_8k_renderer_t *renderer) {
    if (renderer) {
        return renderer->stats;
    }
    
    kdvd_8k_render_stats_t empty_stats = {0};
    return empty_stats;
}

int kdvd_8k_renderer_reset_stats(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    memset(&renderer->stats, 0, sizeof(kdvd_8k_render_stats_t));
    renderer->start_time = vlc_tick_now();
    
    msg_Info(renderer->obj, "8K renderer statistics reset");
    return 0;
}

int kdvd_8k_renderer_set_performance_mode(kdvd_8k_renderer_t *renderer, const char *mode) {
    if (!renderer || !mode) return -1;
    
    msg_Info(renderer->obj, "Setting 8K renderer performance mode: %s", mode);
    
    if (strcmp(mode, "quality") == 0) {
        // Quality mode - prioritize quality over speed
        renderer->config.bit_depth = 12;  // 12-bit for maximum quality
        renderer->config.hdr_enabled = true;
        renderer->config.dolby_vision_enabled = true;
        renderer->config.vsync_enabled = true;
        renderer->config.adaptive_sync = true;
    } else if (strcmp(mode, "speed") == 0) {
        // Speed mode - prioritize speed over quality
        renderer->config.bit_depth = 10;  // 10-bit for speed
        renderer->config.hdr_enabled = true;
        renderer->config.dolby_vision_enabled = false;
        renderer->config.vsync_enabled = false;
        renderer->config.adaptive_sync = false;
    } else if (strcmp(mode, "balanced") == 0) {
        // Balanced mode - balance quality and speed
        renderer->config.bit_depth = 10;  // 10-bit HDR
        renderer->config.hdr_enabled = true;
        renderer->config.dolby_vision_enabled = false;
        renderer->config.vsync_enabled = true;
        renderer->config.adaptive_sync = true;
    } else {
        msg_Err(renderer->obj, "Unknown performance mode: %s", mode);
        return -1;
    }
    
    return 0;
}

int kdvd_8k_renderer_allocate_buffers(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    size_t buffer_size = renderer->config.width * renderer->config.height * 4;
    
    if (renderer->frame_buffer_size < buffer_size) {
        if (renderer->frame_buffer) {
            free(renderer->frame_buffer);
        }
        renderer->frame_buffer = malloc(buffer_size);
        if (!renderer->frame_buffer) {
            msg_Err(renderer->obj, "Failed to allocate 8K frame buffer");
            return -1;
        }
        renderer->frame_buffer_size = buffer_size;
    }
    
    msg_Info(renderer->obj, "8K renderer buffers allocated: %zu MB", buffer_size / (1024 * 1024));
    return 0;
}

int kdvd_8k_renderer_free_buffers(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    if (renderer->frame_buffer) {
        free(renderer->frame_buffer);
        renderer->frame_buffer = NULL;
        renderer->frame_buffer_size = 0;
    }
    
    msg_Info(renderer->obj, "8K renderer buffers freed");
    return 0;
}

int kdvd_8k_renderer_get_memory_usage(kdvd_8k_renderer_t *renderer, uint32_t *usage_mb) {
    if (!renderer || !usage_mb) return -1;
    
    *usage_mb = renderer->frame_buffer_size / (1024 * 1024);
    return 0;
}

int kdvd_8k_renderer_get_last_error(kdvd_8k_renderer_t *renderer, char *error_buffer, size_t buffer_size) {
    if (!renderer || !error_buffer) return -1;
    
    strncpy(error_buffer, renderer->last_error, buffer_size - 1);
    error_buffer[buffer_size - 1] = '\0';
    return 0;
}

int kdvd_8k_renderer_clear_errors(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return -1;
    
    memset(renderer->last_error, 0, sizeof(renderer->last_error));
    return 0;
}

void kdvd_8k_renderer_set_debug(kdvd_8k_renderer_t *renderer, bool enable) {
    if (renderer) {
        renderer->debug_enabled = enable;
        msg_Info(renderer->obj, "8K renderer debug %s", enable ? "enabled" : "disabled");
    }
}

void kdvd_8k_renderer_log_info(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return;
    
    msg_Info(renderer->obj, "8K Video Renderer Info:");
    msg_Info(renderer->obj, "  Resolution: %ux%u", renderer->config.width, renderer->config.height);
    msg_Info(renderer->obj, "  Bit Depth: %u-bit", renderer->config.bit_depth);
    msg_Info(renderer->obj, "  Frame Rate: %u FPS", renderer->config.frame_rate);
    msg_Info(renderer->obj, "  HDR: %s", renderer->config.hdr_enabled ? "enabled" : "disabled");
    msg_Info(renderer->obj, "  Dolby Vision: %s", renderer->config.dolby_vision_enabled ? "enabled" : "disabled");
    msg_Info(renderer->obj, "  Hardware Acceleration: %s", renderer->config.hardware_acceleration ? "enabled" : "disabled");
    msg_Info(renderer->obj, "  V-Sync: %s", renderer->config.vsync_enabled ? "enabled" : "disabled");
    msg_Info(renderer->obj, "  Adaptive Sync: %s", renderer->config.adaptive_sync ? "enabled" : "disabled");
    msg_Info(renderer->obj, "  Max FPS: %u", renderer->config.max_fps);
}

void kdvd_8k_renderer_log_stats(kdvd_8k_renderer_t *renderer) {
    if (!renderer) return;
    
    msg_Info(renderer->obj, "8K Video Renderer Statistics:");
    msg_Info(renderer->obj, "  Frames Rendered: %llu", renderer->stats.frames_rendered);
    msg_Info(renderer->obj, "  Total Render Time: %llu us", renderer->stats.render_time_us);
    msg_Info(renderer->obj, "  Dropped Frames: %llu", renderer->stats.dropped_frames);
    msg_Info(renderer->obj, "  Average FPS: %.2f", renderer->stats.average_fps);
    msg_Info(renderer->obj, "  Average Render Time: %.2f us", renderer->stats.average_render_time);
    msg_Info(renderer->obj, "  GPU Usage: %.1f%%", renderer->stats.gpu_usage_percent);
    msg_Info(renderer->obj, "  Memory Usage: %.1f MB", renderer->stats.memory_usage_mb);
    msg_Info(renderer->obj, "  Current FPS: %u", renderer->stats.current_fps);
    msg_Info(renderer->obj, "  V-Sync Active: %s", renderer->stats.vsync_active ? "yes" : "no");
    msg_Info(renderer->obj, "  Adaptive Sync Active: %s", renderer->stats.adaptive_sync_active ? "yes" : "no");
}
