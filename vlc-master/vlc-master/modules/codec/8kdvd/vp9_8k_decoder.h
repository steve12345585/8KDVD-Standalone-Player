#ifndef VLC_VP9_8K_DECODER_H
#define VLC_VP9_8K_DECODER_H

#include <vlc_common.h>
#include <vlc_codec.h>
#include <vlc_fourcc.h>
#include <vlc_picture.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <stdint.h>
#include <stdbool.h>

// VP9 8K Decoder for 8KDVD
typedef struct vp9_8k_decoder_t vp9_8k_decoder_t;

// VP9 8K Decoder Configuration
typedef struct vp9_8k_config_t {
    uint32_t width;                   // Video width (7680 for 8K)
    uint32_t height;                  // Video height (4320 for 8K)
    uint32_t bit_depth;               // Bit depth (10-bit for HDR)
    uint32_t frame_rate;              // Frame rate (60 FPS)
    bool hdr_enabled;                 // HDR support
    bool dolby_vision_enabled;        // Dolby Vision support
    bool hardware_acceleration;       // Hardware acceleration
    uint32_t profile;                 // VP9 profile (0-3)
    uint32_t level;                   // VP9 level
    uint32_t color_space;             // Color space
    uint32_t color_range;             // Color range
    uint32_t chroma_subsampling;      // Chroma subsampling
} vp9_8k_config_t;

// VP9 8K Decoder Statistics
typedef struct vp9_8k_stats_t {
    uint64_t frames_decoded;          // Total frames decoded
    uint64_t bytes_processed;         // Total bytes processed
    uint64_t decode_time_us;          // Total decode time in microseconds
    uint64_t dropped_frames;          // Dropped frames
    float average_fps;                 // Average FPS
    float average_decode_time;        // Average decode time per frame
    uint32_t current_frame_rate;      // Current frame rate
    uint32_t memory_usage_mb;         // Memory usage in MB
} vp9_8k_stats_t;

// VP9 8K Decoder Functions
vp9_8k_decoder_t* vp9_8k_decoder_create(vlc_object_t *obj);
void vp9_8k_decoder_destroy(vp9_8k_decoder_t *decoder);

// Decoder Configuration
int vp9_8k_decoder_configure(vp9_8k_decoder_t *decoder, const vp9_8k_config_t *config);
int vp9_8k_decoder_set_hardware_acceleration(vp9_8k_decoder_t *decoder, bool enable);
int vp9_8k_decoder_set_hdr_mode(vp9_8k_decoder_t *decoder, bool hdr_enabled, bool dolby_vision_enabled);

// Decoding Functions
int vp9_8k_decoder_decode_frame(vp9_8k_decoder_t *decoder, block_t *input_block, picture_t **output_picture);
int vp9_8k_decoder_flush(vp9_8k_decoder_t *decoder);
int vp9_8k_decoder_reset(vp9_8k_decoder_t *decoder);

// 8K Specific Functions
int vp9_8k_decoder_enable_8k_mode(vp9_8k_decoder_t *decoder, bool enable);
int vp9_8k_decoder_set_8k_resolution(vp9_8k_decoder_t *decoder, uint32_t width, uint32_t height);
int vp9_8k_decoder_optimize_for_8k(vp9_8k_decoder_t *decoder);

// HDR/Dolby Vision Functions
int vp9_8k_decoder_enable_hdr(vp9_8k_decoder_t *decoder, bool enable);
int vp9_8k_decoder_enable_dolby_vision(vp9_8k_decoder_t *decoder, bool enable);
int vp9_8k_decoder_set_color_space(vp9_8k_decoder_t *decoder, uint32_t color_space);
int vp9_8k_decoder_set_color_range(vp9_8k_decoder_t *decoder, uint32_t color_range);

// Hardware Acceleration
int vp9_8k_decoder_detect_hardware_support(vp9_8k_decoder_t *decoder);
int vp9_8k_decoder_enable_nvenc(vp9_8k_decoder_t *decoder, bool enable);
int vp9_8k_decoder_enable_quicksync(vp9_8k_decoder_t *decoder, bool enable);
int vp9_8k_decoder_enable_amf(vp9_8k_decoder_t *decoder, bool enable);

// Performance and Statistics
vp9_8k_stats_t vp9_8k_decoder_get_stats(vp9_8k_decoder_t *decoder);
int vp9_8k_decoder_reset_stats(vp9_8k_decoder_t *decoder);
int vp9_8k_decoder_set_performance_mode(vp9_8k_decoder_t *decoder, const char *mode);

// Memory Management
int vp9_8k_decoder_allocate_buffers(vp9_8k_decoder_t *decoder);
int vp9_8k_decoder_free_buffers(vp9_8k_decoder_t *decoder);
int vp9_8k_decoder_get_memory_usage(vp9_8k_decoder_t *decoder, uint32_t *usage_mb);

// Error Handling
int vp9_8k_decoder_get_last_error(vp9_8k_decoder_t *decoder, char *error_buffer, size_t buffer_size);
int vp9_8k_decoder_clear_errors(vp9_8k_decoder_t *decoder);

// Debug and Logging
void vp9_8k_decoder_set_debug(vp9_8k_decoder_t *decoder, bool enable);
void vp9_8k_decoder_log_info(vp9_8k_decoder_t *decoder);
void vp9_8k_decoder_log_stats(vp9_8k_decoder_t *decoder);

#endif // VLC_VP9_8K_DECODER_H
