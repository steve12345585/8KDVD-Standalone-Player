#ifndef VLC_8K_AUDIO_PROCESSOR_H
#define VLC_8K_AUDIO_PROCESSOR_H

#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <stdint.h>
#include <stdbool.h>

// 8K Audio Processor for 8KDVD Spatial Audio
typedef struct kdvd_8k_audio_processor_t kdvd_8k_audio_processor_t;

// 8K Audio Processor Configuration
typedef struct kdvd_8k_audio_config_t {
    uint32_t channels;                // Number of channels (8 for spatial)
    uint32_t sample_rate;            // Sample rate (48000 Hz)
    uint32_t bitrate;                // Bitrate (up to 510 kbps)
    uint32_t frame_size;             // Frame size in samples
    bool spatial_audio;               // Spatial audio support
    bool ambisonics;                 // Ambisonics support
    bool binaural;                   // Binaural rendering
    uint32_t ambisonics_order;       // Ambisonics order (1-3)
    uint32_t ambisonics_channels;    // Ambisonics channel count
    float spatial_resolution;         // Spatial resolution
    float binaural_quality;           // Binaural rendering quality
    bool hrtf_enabled;               // HRTF support
    bool room_simulation;            // Room simulation
    float room_size;                 // Room size
    float room_damping;              // Room damping
    bool doppler_effect;             // Doppler effect
    bool distance_attenuation;       // Distance attenuation
    bool occlusion_simulation;       // Occlusion simulation
} kdvd_8k_audio_config_t;

// 8K Audio Processor Statistics
typedef struct kdvd_8k_audio_stats_t {
    uint64_t frames_processed;        // Total frames processed
    uint64_t samples_processed;      // Total samples processed
    uint64_t bytes_processed;        // Total bytes processed
    uint64_t process_time_us;        // Total process time in microseconds
    uint64_t dropped_frames;         // Dropped frames
    float average_fps;                // Average FPS
    float average_process_time;       // Average process time per frame
    uint32_t current_sample_rate;     // Current sample rate
    uint32_t current_channels;        // Current channel count
    float spatial_accuracy;          // Spatial accuracy
    float binaural_quality;          // Binaural rendering quality
    float room_simulation_quality;    // Room simulation quality
    uint32_t memory_usage_mb;        // Memory usage in MB
} kdvd_8k_audio_stats_t;

// 8K Audio Processor Functions
kdvd_8k_audio_processor_t* kdvd_8k_audio_processor_create(vlc_object_t *obj);
void kdvd_8k_audio_processor_destroy(kdvd_8k_audio_processor_t *processor);

// Processor Configuration
int kdvd_8k_audio_processor_configure(kdvd_8k_audio_processor_t *processor, const kdvd_8k_audio_config_t *config);
int kdvd_8k_audio_processor_set_spatial_audio(kdvd_8k_audio_processor_t *processor, bool enable);
int kdvd_8k_audio_processor_set_ambisonics(kdvd_8k_audio_processor_t *processor, bool enable, uint32_t order);
int kdvd_8k_audio_processor_set_binaural(kdvd_8k_audio_processor_t *processor, bool enable, float quality);

// Audio Processing Functions
int kdvd_8k_audio_processor_process_frame(kdvd_8k_audio_processor_t *processor, block_t *input_block, block_t **output_block);
int kdvd_8k_audio_processor_flush(kdvd_8k_audio_processor_t *processor);
int kdvd_8k_audio_processor_reset(kdvd_8k_audio_processor_t *processor);

// 8K Spatial Audio Functions
int kdvd_8k_audio_processor_enable_8k_spatial(kdvd_8k_audio_processor_t *processor, bool enable);
int kdvd_8k_audio_processor_set_8k_channels(kdvd_8k_audio_processor_t *processor, uint32_t channels);
int kdvd_8k_audio_processor_optimize_for_8k(kdvd_8k_audio_processor_t *processor);

// Spatial Audio Processing
int kdvd_8k_audio_processor_process_spatial(kdvd_8k_audio_processor_t *processor, float *input, float *output, uint32_t samples);
int kdvd_8k_audio_processor_process_ambisonics(kdvd_8k_audio_processor_t *processor, float *input, float *output, uint32_t samples);
int kdvd_8k_audio_processor_process_binaural(kdvd_8k_audio_processor_t *processor, float *input, float *output, uint32_t samples);

// HRTF and Spatial Processing
int kdvd_8k_audio_processor_load_hrtf(kdvd_8k_audio_processor_t *processor, const char *hrtf_file);
int kdvd_8k_audio_processor_set_spatial_resolution(kdvd_8k_audio_processor_t *processor, float resolution);
int kdvd_8k_audio_processor_set_listener_position(kdvd_8k_audio_processor_t *processor, float x, float y, float z);
int kdvd_8k_audio_processor_set_listener_orientation(kdvd_8k_audio_processor_t *processor, float yaw, float pitch, float roll);

// Room Simulation
int kdvd_8k_audio_processor_enable_room_simulation(kdvd_8k_audio_processor_t *processor, bool enable);
int kdvd_8k_audio_processor_set_room_size(kdvd_8k_audio_processor_t *processor, float size);
int kdvd_8k_audio_processor_set_room_damping(kdvd_8k_audio_processor_t *processor, float damping);
int kdvd_8k_audio_processor_set_room_acoustics(kdvd_8k_audio_processor_t *processor, float size, float damping);

// Advanced Audio Effects
int kdvd_8k_audio_processor_enable_doppler_effect(kdvd_8k_audio_processor_t *processor, bool enable);
int kdvd_8k_audio_processor_enable_distance_attenuation(kdvd_8k_audio_processor_t *processor, bool enable);
int kdvd_8k_audio_processor_enable_occlusion_simulation(kdvd_8k_audio_processor_t *processor, bool enable);
int kdvd_8k_audio_processor_set_audio_effects(kdvd_8k_audio_processor_t *processor, bool doppler, bool distance, bool occlusion);

// Performance and Statistics
kdvd_8k_audio_stats_t kdvd_8k_audio_processor_get_stats(kdvd_8k_audio_processor_t *processor);
int kdvd_8k_audio_processor_reset_stats(kdvd_8k_audio_processor_t *processor);
int kdvd_8k_audio_processor_set_performance_mode(kdvd_8k_audio_processor_t *processor, const char *mode);

// Memory Management
int kdvd_8k_audio_processor_allocate_buffers(kdvd_8k_audio_processor_t *processor);
int kdvd_8k_audio_processor_free_buffers(kdvd_8k_audio_processor_t *processor);
int kdvd_8k_audio_processor_get_memory_usage(kdvd_8k_audio_processor_t *processor, uint32_t *usage_mb);

// Error Handling
int kdvd_8k_audio_processor_get_last_error(kdvd_8k_audio_processor_t *processor, char *error_buffer, size_t buffer_size);
int kdvd_8k_audio_processor_clear_errors(kdvd_8k_audio_processor_t *processor);

// Debug and Logging
void kdvd_8k_audio_processor_set_debug(kdvd_8k_audio_processor_t *processor, bool enable);
void kdvd_8k_audio_processor_log_info(kdvd_8k_audio_processor_t *processor);
void kdvd_8k_audio_processor_log_stats(kdvd_8k_audio_processor_t *processor);

#endif // VLC_8K_AUDIO_PROCESSOR_H
