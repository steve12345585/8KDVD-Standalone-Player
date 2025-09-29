#ifndef VLC_OPUS_8K_DECODER_H
#define VLC_OPUS_8K_DECODER_H

#include <vlc_common.h>
#include <vlc_codec.h>
#include <vlc_fourcc.h>
#include <vlc_aout.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <stdint.h>
#include <stdbool.h>

// Opus 8K Decoder for 8KDVD Spatial Audio
typedef struct opus_8k_decoder_t opus_8k_decoder_t;

// Opus 8K Decoder Configuration
typedef struct opus_8k_config_t {
    uint32_t channels;                // Number of channels (8 for spatial)
    uint32_t sample_rate;            // Sample rate (48000 Hz)
    uint32_t bitrate;                // Bitrate (up to 510 kbps)
    uint32_t frame_size;             // Frame size in samples
    uint32_t application;             // Opus application type
    bool spatial_audio;               // Spatial audio support
    bool ambisonics;                 // Ambisonics support
    bool binaural;                   // Binaural rendering
    uint32_t ambisonics_order;       // Ambisonics order (1-3)
    uint32_t ambisonics_channels;    // Ambisonics channel count
    float spatial_resolution;         // Spatial resolution
    float binaural_quality;           // Binaural rendering quality
} opus_8k_config_t;

// Opus 8K Decoder Statistics
typedef struct opus_8k_stats_t {
    uint64_t frames_decoded;          // Total frames decoded
    uint64_t samples_decoded;         // Total samples decoded
    uint64_t bytes_processed;         // Total bytes processed
    uint64_t decode_time_us;          // Total decode time in microseconds
    uint64_t dropped_frames;          // Dropped frames
    float average_fps;                 // Average FPS
    float average_decode_time;        // Average decode time per frame
    uint32_t current_sample_rate;     // Current sample rate
    uint32_t current_channels;        // Current channel count
    uint32_t memory_usage_mb;         // Memory usage in MB
    float spatial_accuracy;           // Spatial accuracy
    float binaural_quality;           // Binaural rendering quality
} opus_8k_stats_t;

// Opus 8K Decoder Functions
opus_8k_decoder_t* opus_8k_decoder_create(vlc_object_t *obj);
void opus_8k_decoder_destroy(opus_8k_decoder_t *decoder);

// Decoder Configuration
int opus_8k_decoder_configure(opus_8k_decoder_t *decoder, const opus_8k_config_t *config);
int opus_8k_decoder_set_spatial_audio(opus_8k_decoder_t *decoder, bool enable);
int opus_8k_decoder_set_ambisonics(opus_8k_decoder_t *decoder, bool enable, uint32_t order);
int opus_8k_decoder_set_binaural(opus_8k_decoder_t *decoder, bool enable, float quality);

// Decoding Functions
int opus_8k_decoder_decode_frame(opus_8k_decoder_t *decoder, block_t *input_block, block_t **output_block);
int opus_8k_decoder_flush(opus_8k_decoder_t *decoder);
int opus_8k_decoder_reset(opus_8k_decoder_t *decoder);

// 8K Spatial Audio Functions
int opus_8k_decoder_enable_8k_spatial(opus_8k_decoder_t *decoder, bool enable);
int opus_8k_decoder_set_8k_channels(opus_8k_decoder_t *decoder, uint32_t channels);
int opus_8k_decoder_optimize_for_8k(opus_8k_decoder_t *decoder);

// Spatial Audio Processing
int opus_8k_decoder_process_spatial(opus_8k_decoder_t *decoder, float *input, float *output, uint32_t samples);
int opus_8k_decoder_process_ambisonics(opus_8k_decoder_t *decoder, float *input, float *output, uint32_t samples);
int opus_8k_decoder_process_binaural(opus_8k_decoder_t *decoder, float *input, float *output, uint32_t samples);

// HRTF and Spatial Processing
int opus_8k_decoder_load_hrtf(opus_8k_decoder_t *decoder, const char *hrtf_file);
int opus_8k_decoder_set_spatial_resolution(opus_8k_decoder_t *decoder, float resolution);
int opus_8k_decoder_set_listener_position(opus_8k_decoder_t *decoder, float x, float y, float z);
int opus_8k_decoder_set_listener_orientation(opus_8k_decoder_t *decoder, float yaw, float pitch, float roll);

// Performance and Statistics
opus_8k_stats_t opus_8k_decoder_get_stats(opus_8k_decoder_t *decoder);
int opus_8k_decoder_reset_stats(opus_8k_decoder_t *decoder);
int opus_8k_decoder_set_performance_mode(opus_8k_decoder_t *decoder, const char *mode);

// Memory Management
int opus_8k_decoder_allocate_buffers(opus_8k_decoder_t *decoder);
int opus_8k_decoder_free_buffers(opus_8k_decoder_t *decoder);
int opus_8k_decoder_get_memory_usage(opus_8k_decoder_t *decoder, uint32_t *usage_mb);

// Error Handling
int opus_8k_decoder_get_last_error(opus_8k_decoder_t *decoder, char *error_buffer, size_t buffer_size);
int opus_8k_decoder_clear_errors(opus_8k_decoder_t *decoder);

// Debug and Logging
void opus_8k_decoder_set_debug(opus_8k_decoder_t *decoder, bool enable);
void opus_8k_decoder_log_info(opus_8k_decoder_t *decoder);
void opus_8k_decoder_log_stats(opus_8k_decoder_t *decoder);

#endif // VLC_OPUS_8K_DECODER_H
