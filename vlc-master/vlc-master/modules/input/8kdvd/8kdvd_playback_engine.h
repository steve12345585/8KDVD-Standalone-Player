#ifndef VLC_8KDVD_PLAYBACK_ENGINE_H
#define VLC_8KDVD_PLAYBACK_ENGINE_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Playback Engine for Seamless 8K Video Playback
typedef struct kdvd_playback_engine_t kdvd_playback_engine_t;

// 8KDVD Playback State
typedef enum {
    EIGHTKDVD_PLAYBACK_STOPPED = 0,
    EIGHTKDVD_PLAYBACK_PLAYING,
    EIGHTKDVD_PLAYBACK_PAUSED,
    EIGHTKDVD_PLAYBACK_SEEKING,
    EIGHTKDVD_PLAYBACK_BUFFERING,
    EIGHTKDVD_PLAYBACK_ERROR
} kdvd_playback_state_t;

// 8KDVD Playback Configuration
typedef struct kdvd_playback_config_t {
    bool enable_hardware_acceleration;  // Hardware acceleration
    bool enable_hdr_processing;         // HDR processing
    bool enable_spatial_audio;          // Spatial audio processing
    bool enable_adaptive_bitrate;       // Adaptive bitrate streaming
    uint32_t buffer_size_mb;            // Buffer size in MB
    uint32_t max_bitrate_mbps;          // Maximum bitrate in Mbps
    uint32_t target_framerate;          // Target framerate
    bool enable_vsync;                  // V-Sync support
    bool enable_adaptive_sync;          // Adaptive Sync support
    uint32_t audio_channels;            // Audio channel count
    uint32_t audio_sample_rate;         // Audio sample rate
    uint32_t video_width;               // Video width
    uint32_t video_height;              // Video height
    uint32_t video_bit_depth;           // Video bit depth
    bool enable_dolby_vision;           // Dolby Vision support
    bool enable_hdr10_plus;             // HDR10+ support
    char performance_mode[32];          // Performance mode
} kdvd_playback_config_t;

// 8KDVD Playback Statistics
typedef struct kdvd_playback_stats_t {
    uint64_t frames_rendered;           // Total frames rendered
    uint64_t frames_dropped;             // Total frames dropped
    uint64_t audio_samples_processed;    // Total audio samples processed
    uint64_t bytes_processed;            // Total bytes processed
    uint64_t playback_time_ms;           // Total playback time in ms
    float average_framerate;              // Average framerate
    float current_framerate;             // Current framerate
    float average_bitrate_mbps;          // Average bitrate in Mbps
    float current_bitrate_mbps;          // Current bitrate in Mbps
    uint32_t buffer_usage_percent;       // Buffer usage percentage
    uint32_t cpu_usage_percent;          // CPU usage percentage
    uint32_t gpu_usage_percent;          // GPU usage percentage
    uint32_t memory_usage_mb;            // Memory usage in MB
    uint64_t seek_operations;            // Total seek operations
    uint64_t pause_operations;           // Total pause operations
    uint64_t error_count;                // Total error count
    float average_seek_time_ms;          // Average seek time in ms
    float average_pause_time_ms;         // Average pause time in ms
} kdvd_playback_stats_t;

// 8KDVD Playback Engine Functions
kdvd_playback_engine_t* kdvd_playback_engine_create(vlc_object_t *obj);
void kdvd_playback_engine_destroy(kdvd_playback_engine_t *engine);

// Playback Control
int kdvd_playback_engine_play(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_pause(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_stop(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_seek(kdvd_playback_engine_t *engine, uint64_t position_ms);

// Playback State
kdvd_playback_state_t kdvd_playback_engine_get_state(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_set_state(kdvd_playback_engine_t *engine, kdvd_playback_state_t state);
bool kdvd_playback_engine_is_playing(kdvd_playback_engine_t *engine);
bool kdvd_playback_engine_is_paused(kdvd_playback_engine_t *engine);

// Playback Configuration
int kdvd_playback_engine_set_config(kdvd_playback_engine_t *engine, const kdvd_playback_config_t *config);
kdvd_playback_config_t kdvd_playback_engine_get_config(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_reset_config(kdvd_playback_engine_t *engine);

// 8K Video Processing
int kdvd_playback_engine_process_video_frame(kdvd_playback_engine_t *engine, const uint8_t *frame_data, size_t frame_size);
int kdvd_playback_engine_render_video_frame(kdvd_playback_engine_t *engine, uint8_t *output_buffer, size_t buffer_size);
int kdvd_playback_engine_set_video_resolution(kdvd_playback_engine_t *engine, uint32_t width, uint32_t height);
int kdvd_playback_engine_set_video_bitrate(kdvd_playback_engine_t *engine, uint32_t bitrate_mbps);

// 8K Audio Processing
int kdvd_playback_engine_process_audio_frame(kdvd_playback_engine_t *engine, const uint8_t *audio_data, size_t audio_size);
int kdvd_playback_engine_render_audio_frame(kdvd_playback_engine_t *engine, float *output_buffer, size_t buffer_size);
int kdvd_playback_engine_set_audio_channels(kdvd_playback_engine_t *engine, uint32_t channels);
int kdvd_playback_engine_set_audio_sample_rate(kdvd_playback_engine_t *engine, uint32_t sample_rate);

// HDR and Dolby Vision
int kdvd_playback_engine_set_hdr_mode(kdvd_playback_engine_t *engine, const char *hdr_mode);
int kdvd_playback_engine_set_dolby_vision(kdvd_playback_engine_t *engine, bool enable);
int kdvd_playback_engine_set_hdr10_plus(kdvd_playback_engine_t *engine, bool enable);
int kdvd_playback_engine_process_hdr_metadata(kdvd_playback_engine_t *engine, const uint8_t *metadata, size_t metadata_size);

// Hardware Acceleration
int kdvd_playback_engine_enable_hardware_acceleration(kdvd_playback_engine_t *engine, bool enable);
int kdvd_playback_engine_set_gpu_device(kdvd_playback_engine_t *engine, const char *device_name);
int kdvd_playback_engine_get_gpu_info(kdvd_playback_engine_t *engine, char *info_buffer, size_t buffer_size);

// Performance and Statistics
kdvd_playback_stats_t kdvd_playback_engine_get_stats(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_reset_stats(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_set_performance_mode(kdvd_playback_engine_t *engine, const char *mode);

// Buffer Management
int kdvd_playback_engine_allocate_buffers(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_free_buffers(kdvd_playback_engine_t *engine);
int kdvd_playback_engine_get_buffer_usage(kdvd_playback_engine_t *engine, uint32_t *usage_percent);

// Error Handling
int kdvd_playback_engine_get_last_error(kdvd_playback_engine_t *engine, char *error_buffer, size_t buffer_size);
int kdvd_playback_engine_clear_errors(kdvd_playback_engine_t *engine);

// Debug and Logging
void kdvd_playback_engine_set_debug(kdvd_playback_engine_t *engine, bool enable);
void kdvd_playback_engine_log_info(kdvd_playback_engine_t *engine);
void kdvd_playback_engine_log_stats(kdvd_playback_engine_t *engine);

#endif // VLC_8KDVD_PLAYBACK_ENGINE_H
