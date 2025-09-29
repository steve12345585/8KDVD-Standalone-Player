#ifndef VLC_8K_VIDEO_RENDERER_H
#define VLC_8K_VIDEO_RENDERER_H

#include <vlc_common.h>
#include <vlc_vout.h>
#include <vlc_picture.h>
#include <vlc_fourcc.h>
#include <vlc_window.h>
#include <vlc_opengl.h>
#include <vlc_gl.h>
#include <stdint.h>
#include <stdbool.h>

// 8K Video Renderer for 8KDVD
typedef struct kdvd_8k_renderer_t kdvd_8k_renderer_t;

// 8K Video Renderer Configuration
typedef struct kdvd_8k_render_config_t {
    uint32_t width;                   // Video width (7680 for 8K)
    uint32_t height;                  // Video height (4320 for 8K)
    uint32_t bit_depth;               // Bit depth (10-bit for HDR)
    uint32_t frame_rate;              // Frame rate (60 FPS)
    bool hdr_enabled;                 // HDR support
    bool dolby_vision_enabled;        // Dolby Vision support
    bool hardware_acceleration;       // Hardware acceleration
    uint32_t color_space;             // Color space (BT.2020 for HDR)
    uint32_t color_range;             // Color range (Full for HDR)
    uint32_t chroma_subsampling;      // Chroma subsampling (4:2:0)
    bool vsync_enabled;               // V-Sync support
    bool triple_buffering;            // Triple buffering
    uint32_t max_fps;                 // Maximum FPS
    bool adaptive_sync;               // Adaptive sync (G-Sync/FreeSync)
} kdvd_8k_render_config_t;

// 8K Video Renderer Statistics
typedef struct kdvd_8k_render_stats_t {
    uint64_t frames_rendered;         // Total frames rendered
    uint64_t render_time_us;          // Total render time in microseconds
    uint64_t dropped_frames;          // Dropped frames
    float average_fps;                 // Average FPS
    float average_render_time;        // Average render time per frame
    float gpu_usage_percent;          // GPU usage percentage
    float memory_usage_mb;            // Memory usage in MB
    uint32_t current_fps;             // Current FPS
    bool vsync_active;                // V-Sync status
    bool adaptive_sync_active;        // Adaptive sync status
} kdvd_8k_render_stats_t;

// 8K Video Renderer Functions
kdvd_8k_renderer_t* kdvd_8k_renderer_create(vlc_object_t *obj);
void kdvd_8k_renderer_destroy(kdvd_8k_renderer_t *renderer);

// Renderer Configuration
int kdvd_8k_renderer_configure(kdvd_8k_renderer_t *renderer, const kdvd_8k_render_config_t *config);
int kdvd_8k_renderer_set_8k_mode(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_set_hdr_mode(kdvd_8k_renderer_t *renderer, bool hdr_enabled, bool dolby_vision_enabled);
int kdvd_8k_renderer_set_hardware_acceleration(kdvd_8k_renderer_t *renderer, bool enable);

// Rendering Functions
int kdvd_8k_renderer_render_frame(kdvd_8k_renderer_t *renderer, picture_t *picture);
int kdvd_8k_renderer_present_frame(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_clear_screen(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_flush(kdvd_8k_renderer_t *renderer);

// 8K Specific Functions
int kdvd_8k_renderer_enable_8k_rendering(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_set_8k_resolution(kdvd_8k_renderer_t *renderer, uint32_t width, uint32_t height);
int kdvd_8k_renderer_optimize_for_8k(kdvd_8k_renderer_t *renderer);

// HDR/Dolby Vision Functions
int kdvd_8k_renderer_enable_hdr(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_enable_dolby_vision(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_set_color_space(kdvd_8k_renderer_t *renderer, uint32_t color_space);
int kdvd_8k_renderer_set_color_range(kdvd_8k_renderer_t *renderer, uint32_t color_range);

// Hardware Acceleration
int kdvd_8k_renderer_detect_hardware_support(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_enable_nvenc(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_enable_quicksync(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_enable_amf(kdvd_8k_renderer_t *renderer, bool enable);

// V-Sync and Adaptive Sync
int kdvd_8k_renderer_enable_vsync(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_enable_adaptive_sync(kdvd_8k_renderer_t *renderer, bool enable);
int kdvd_8k_renderer_set_max_fps(kdvd_8k_renderer_t *renderer, uint32_t max_fps);

// Performance and Statistics
kdvd_8k_render_stats_t kdvd_8k_renderer_get_stats(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_reset_stats(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_set_performance_mode(kdvd_8k_renderer_t *renderer, const char *mode);

// Memory Management
int kdvd_8k_renderer_allocate_buffers(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_free_buffers(kdvd_8k_renderer_t *renderer);
int kdvd_8k_renderer_get_memory_usage(kdvd_8k_renderer_t *renderer, uint32_t *usage_mb);

// Error Handling
int kdvd_8k_renderer_get_last_error(kdvd_8k_renderer_t *renderer, char *error_buffer, size_t buffer_size);
int kdvd_8k_renderer_clear_errors(kdvd_8k_renderer_t *renderer);

// Debug and Logging
void kdvd_8k_renderer_set_debug(kdvd_8k_renderer_t *renderer, bool enable);
void kdvd_8k_renderer_log_info(kdvd_8k_renderer_t *renderer);
void kdvd_8k_renderer_log_stats(kdvd_8k_renderer_t *renderer);

#endif // VLC_8K_VIDEO_RENDERER_H
