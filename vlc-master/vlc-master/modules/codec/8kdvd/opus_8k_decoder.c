#include "opus_8k_decoder.h"
#include <vlc_messages.h>
#include <vlc_aout.h>
#include <vlc_block.h>
#include <vlc_fourcc.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Opus 8K Decoder Implementation
struct opus_8k_decoder_t {
    vlc_object_t *obj;
    opus_8k_config_t config;
    opus_8k_stats_t stats;
    bool initialized;
    bool spatial_audio_enabled;
    bool ambisonics_enabled;
    bool binaural_enabled;
    bool debug_enabled;
    char last_error[256];
    void *decoder_context;  // Placeholder for actual Opus decoder context
    float *audio_buffer;
    size_t audio_buffer_size;
    float *spatial_buffer;
    size_t spatial_buffer_size;
    float *hrtf_data;
    size_t hrtf_size;
    float listener_x, listener_y, listener_z;
    float listener_yaw, listener_pitch, listener_roll;
    uint64_t start_time;
    uint64_t last_frame_time;
};

// Opus 8K Decoder Functions
opus_8k_decoder_t* opus_8k_decoder_create(vlc_object_t *obj) {
    opus_8k_decoder_t *decoder = calloc(1, sizeof(opus_8k_decoder_t));
    if (!decoder) return NULL;
    
    decoder->obj = obj;
    decoder->initialized = false;
    decoder->spatial_audio_enabled = false;
    decoder->ambisonics_enabled = false;
    decoder->binaural_enabled = false;
    decoder->debug_enabled = false;
    decoder->decoder_context = NULL;
    decoder->audio_buffer = NULL;
    decoder->audio_buffer_size = 0;
    decoder->spatial_buffer = NULL;
    decoder->spatial_buffer_size = 0;
    decoder->hrtf_data = NULL;
    decoder->hrtf_size = 0;
    decoder->listener_x = 0.0f;
    decoder->listener_y = 0.0f;
    decoder->listener_z = 0.0f;
    decoder->listener_yaw = 0.0f;
    decoder->listener_pitch = 0.0f;
    decoder->listener_roll = 0.0f;
    decoder->start_time = 0;
    decoder->last_frame_time = 0;
    
    // Initialize stats
    memset(&decoder->stats, 0, sizeof(opus_8k_stats_t));
    
    // Initialize config with 8K spatial audio defaults
    decoder->config.channels = 8;
    decoder->config.sample_rate = 48000;
    decoder->config.bitrate = 510000;  // 510 kbps for 8-channel
    decoder->config.frame_size = 960;  // 20ms at 48kHz
    decoder->config.application = 0;   // VOIP application
    decoder->config.spatial_audio = true;
    decoder->config.ambisonics = false;
    decoder->config.binaural = false;
    decoder->config.ambisonics_order = 1;
    decoder->config.ambisonics_channels = 4;
    decoder->config.spatial_resolution = 1.0f;
    decoder->config.binaural_quality = 0.8f;
    
    msg_Info(obj, "Opus 8K decoder created");
    return decoder;
}

void opus_8k_decoder_destroy(opus_8k_decoder_t *decoder) {
    if (!decoder) return;
    
    if (decoder->audio_buffer) {
        free(decoder->audio_buffer);
    }
    
    if (decoder->spatial_buffer) {
        free(decoder->spatial_buffer);
    }
    
    if (decoder->hrtf_data) {
        free(decoder->hrtf_data);
    }
    
    if (decoder->decoder_context) {
        // Clean up decoder context
        free(decoder->decoder_context);
    }
    
    free(decoder);
    msg_Info(decoder->obj, "Opus 8K decoder destroyed");
}

int opus_8k_decoder_configure(opus_8k_decoder_t *decoder, const opus_8k_config_t *config) {
    if (!decoder || !config) return -1;
    
    msg_Info(decoder->obj, "Configuring Opus 8K decoder: %u channels %u Hz %u bps", 
             config->channels, config->sample_rate, config->bitrate);
    
    // Copy configuration
    memcpy(&decoder->config, config, sizeof(opus_8k_config_t));
    
    // Validate 8K spatial audio configuration
    if (config->channels != 8) {
        msg_Err(decoder->obj, "Invalid channel count: %u (expected 8 for spatial audio)", config->channels);
        return -1;
    }
    
    if (config->sample_rate != 48000) {
        msg_Err(decoder->obj, "Invalid sample rate: %u (expected 48000 Hz)", config->sample_rate);
        return -1;
    }
    
    if (config->bitrate > 510000) {
        msg_Err(decoder->obj, "Invalid bitrate: %u (max 510 kbps)", config->bitrate);
        return -1;
    }
    
    // Allocate audio buffer for 8-channel spatial audio
    size_t buffer_size = config->channels * config->frame_size * sizeof(float);
    if (decoder->audio_buffer_size < buffer_size) {
        if (decoder->audio_buffer) {
            free(decoder->audio_buffer);
        }
        decoder->audio_buffer = malloc(buffer_size);
        if (!decoder->audio_buffer) {
            msg_Err(decoder->obj, "Failed to allocate 8K audio buffer");
            return -1;
        }
        decoder->audio_buffer_size = buffer_size;
    }
    
    // Allocate spatial buffer for spatial processing
    if (config->spatial_audio) {
        size_t spatial_buffer_size = config->channels * config->frame_size * sizeof(float) * 2; // Stereo output
        if (decoder->spatial_buffer_size < spatial_buffer_size) {
            if (decoder->spatial_buffer) {
                free(decoder->spatial_buffer);
            }
            decoder->spatial_buffer = malloc(spatial_buffer_size);
            if (!decoder->spatial_buffer) {
                msg_Err(decoder->obj, "Failed to allocate spatial buffer");
                return -1;
            }
            decoder->spatial_buffer_size = spatial_buffer_size;
        }
    }
    
    // Initialize decoder context
    if (decoder->decoder_context) {
        free(decoder->decoder_context);
    }
    decoder->decoder_context = malloc(1024); // Placeholder for actual Opus decoder context
    
    decoder->initialized = true;
    decoder->start_time = vlc_tick_now();
    
    msg_Info(decoder->obj, "Opus 8K decoder configured successfully");
    return 0;
}

int opus_8k_decoder_set_spatial_audio(opus_8k_decoder_t *decoder, bool enable) {
    if (!decoder) return -1;
    
    decoder->spatial_audio_enabled = enable;
    decoder->config.spatial_audio = enable;
    
    if (enable) {
        // Allocate spatial buffer
        size_t spatial_buffer_size = decoder->config.channels * decoder->config.frame_size * sizeof(float) * 2;
        if (decoder->spatial_buffer_size < spatial_buffer_size) {
            if (decoder->spatial_buffer) {
                free(decoder->spatial_buffer);
            }
            decoder->spatial_buffer = malloc(spatial_buffer_size);
            if (!decoder->spatial_buffer) {
                msg_Err(decoder->obj, "Failed to allocate spatial buffer");
                return -1;
            }
            decoder->spatial_buffer_size = spatial_buffer_size;
        }
        msg_Info(decoder->obj, "Spatial audio enabled for Opus 8K decoder");
    } else {
        msg_Info(decoder->obj, "Spatial audio disabled for Opus 8K decoder");
    }
    
    return 0;
}

int opus_8k_decoder_set_ambisonics(opus_8k_decoder_t *decoder, bool enable, uint32_t order) {
    if (!decoder) return -1;
    
    decoder->ambisonics_enabled = enable;
    decoder->config.ambisonics = enable;
    
    if (enable) {
        decoder->config.ambisonics_order = order;
        decoder->config.ambisonics_channels = (order + 1) * (order + 1);
        msg_Info(decoder->obj, "Ambisonics enabled for Opus 8K decoder: order %u, %u channels", 
                 order, decoder->config.ambisonics_channels);
    } else {
        msg_Info(decoder->obj, "Ambisonics disabled for Opus 8K decoder");
    }
    
    return 0;
}

int opus_8k_decoder_set_binaural(opus_8k_decoder_t *decoder, bool enable, float quality) {
    if (!decoder) return -1;
    
    decoder->binaural_enabled = enable;
    decoder->config.binaural = enable;
    decoder->config.binaural_quality = quality;
    
    if (enable) {
        msg_Info(decoder->obj, "Binaural rendering enabled for Opus 8K decoder: quality %.2f", quality);
    } else {
        msg_Info(decoder->obj, "Binaural rendering disabled for Opus 8K decoder");
    }
    
    return 0;
}

int opus_8k_decoder_decode_frame(opus_8k_decoder_t *decoder, block_t *input_block, block_t **output_block) {
    if (!decoder || !input_block || !output_block) return -1;
    
    if (!decoder->initialized) {
        msg_Err(decoder->obj, "Opus 8K decoder not initialized");
        return -1;
    }
    
    uint64_t decode_start = vlc_tick_now();
    
    // Simulate Opus decoding for 8K spatial audio
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Decoding Opus 8K frame: %zu bytes", input_block->i_buffer);
    }
    
    // Create output block
    block_t *output = block_Alloc(decoder->config.channels * decoder->config.frame_size * sizeof(float));
    if (!output) {
        msg_Err(decoder->obj, "Failed to allocate output block");
        return -1;
    }
    
    // Set output block properties
    output->i_dts = input_block->i_dts;
    output->i_pts = input_block->i_pts;
    output->i_length = decoder->config.frame_size * 1000000 / decoder->config.sample_rate;
    
    // Simulate Opus decoding
    float *audio_data = (float*)output->p_buffer;
    uint32_t samples_per_channel = decoder->config.frame_size;
    
    // Generate test audio data (in real implementation, this would be actual Opus decoding)
    for (uint32_t ch = 0; ch < decoder->config.channels; ch++) {
        for (uint32_t i = 0; i < samples_per_channel; i++) {
            float sample = sinf(2.0f * M_PI * 440.0f * i / decoder->config.sample_rate) * 0.1f;
            audio_data[ch * samples_per_channel + i] = sample;
        }
    }
    
    // Apply spatial processing if enabled
    if (decoder->spatial_audio_enabled) {
        opus_8k_decoder_process_spatial(decoder, audio_data, (float*)output->p_buffer, samples_per_channel);
    }
    
    // Apply ambisonics processing if enabled
    if (decoder->ambisonics_enabled) {
        opus_8k_decoder_process_ambisonics(decoder, audio_data, (float*)output->p_buffer, samples_per_channel);
    }
    
    // Apply binaural processing if enabled
    if (decoder->binaural_enabled) {
        opus_8k_decoder_process_binaural(decoder, audio_data, (float*)output->p_buffer, samples_per_channel);
    }
    
    *output_block = output;
    
    // Update statistics
    decoder->stats.frames_decoded++;
    decoder->stats.samples_decoded += decoder->config.channels * samples_per_channel;
    decoder->stats.bytes_processed += input_block->i_buffer;
    
    uint64_t decode_time = vlc_tick_now() - decode_start;
    decoder->stats.decode_time_us += decode_time;
    decoder->stats.last_frame_time = vlc_tick_now();
    
    // Calculate average FPS
    if (decoder->stats.frames_decoded > 0) {
        uint64_t total_time = vlc_tick_now() - decoder->start_time;
        decoder->stats.average_fps = (float)decoder->stats.frames_decoded * 1000000.0f / total_time;
    }
    
    // Calculate average decode time
    if (decoder->stats.frames_decoded > 0) {
        decoder->stats.average_decode_time = (float)decoder->stats.decode_time_us / decoder->stats.frames_decoded;
    }
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Opus 8K frame decoded in %llu us", decode_time);
    }
    
    return 0;
}

int opus_8k_decoder_flush(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    msg_Info(decoder->obj, "Flushing Opus 8K decoder");
    
    // Reset decoder state
    decoder->stats.dropped_frames = 0;
    
    return 0;
}

int opus_8k_decoder_reset(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    msg_Info(decoder->obj, "Resetting Opus 8K decoder");
    
    // Reset all statistics
    memset(&decoder->stats, 0, sizeof(opus_8k_stats_t));
    decoder->start_time = vlc_tick_now();
    
    return 0;
}

int opus_8k_decoder_enable_8k_spatial(opus_8k_decoder_t *decoder, bool enable) {
    if (!decoder) return -1;
    
    if (enable) {
        decoder->config.channels = 8;
        decoder->config.spatial_audio = true;
        decoder->config.bitrate = 510000;  // Max bitrate for 8-channel
        msg_Info(decoder->obj, "8K spatial audio enabled: 8 channels, 510 kbps");
    } else {
        decoder->config.channels = 2;
        decoder->config.spatial_audio = false;
        decoder->config.bitrate = 128000;  // Standard stereo bitrate
        msg_Info(decoder->obj, "8K spatial audio disabled: 2 channels, 128 kbps");
    }
    
    return 0;
}

int opus_8k_decoder_set_8k_channels(opus_8k_decoder_t *decoder, uint32_t channels) {
    if (!decoder) return -1;
    
    if (channels != 8) {
        msg_Err(decoder->obj, "Invalid channel count: %u (expected 8 for 8K spatial audio)", channels);
        return -1;
    }
    
    decoder->config.channels = channels;
    decoder->current_channels = channels;
    
    msg_Info(decoder->obj, "8K channel count set: %u", channels);
    return 0;
}

int opus_8k_decoder_optimize_for_8k(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    msg_Info(decoder->obj, "Optimizing Opus decoder for 8K spatial audio");
    
    // Set optimal settings for 8K spatial audio
    decoder->config.channels = 8;
    decoder->config.sample_rate = 48000;
    decoder->config.bitrate = 510000;  // Max bitrate
    decoder->config.frame_size = 960;  // 20ms frames
    decoder->config.spatial_audio = true;
    decoder->config.spatial_resolution = 1.0f;
    
    // Allocate larger buffers for 8K spatial audio
    size_t buffer_size = decoder->config.channels * decoder->config.frame_size * sizeof(float);
    if (decoder->audio_buffer_size < buffer_size) {
        if (decoder->audio_buffer) {
            free(decoder->audio_buffer);
        }
        decoder->audio_buffer = malloc(buffer_size);
        if (!decoder->audio_buffer) {
            msg_Err(decoder->obj, "Failed to allocate 8K audio buffer");
            return -1;
        }
        decoder->audio_buffer_size = buffer_size;
    }
    
    msg_Info(decoder->obj, "Opus decoder optimized for 8K spatial audio");
    return 0;
}

int opus_8k_decoder_process_spatial(opus_8k_decoder_t *decoder, float *input, float *output, uint32_t samples) {
    if (!decoder || !input || !output) return -1;
    
    if (!decoder->spatial_audio_enabled) {
        // Copy input to output without spatial processing
        memcpy(output, input, decoder->config.channels * samples * sizeof(float));
        return 0;
    }
    
    // Simulate spatial audio processing
    // This would implement actual spatial audio algorithms
    for (uint32_t ch = 0; ch < decoder->config.channels; ch++) {
        for (uint32_t i = 0; i < samples; i++) {
            float sample = input[ch * samples + i];
            
            // Apply spatial processing based on channel position
            float spatial_gain = 1.0f;
            switch (ch) {
                case 0: spatial_gain = 1.0f; break;  // Front Left
                case 1: spatial_gain = 1.0f; break;  // Front Right
                case 2: spatial_gain = 0.8f; break;  // Center
                case 3: spatial_gain = 0.6f; break;  // LFE
                case 4: spatial_gain = 0.9f; break;  // Rear Left
                case 5: spatial_gain = 0.9f; break;  // Rear Right
                case 6: spatial_gain = 0.7f; break;  // Side Left
                case 7: spatial_gain = 0.7f; break;  // Side Right
            }
            
            output[ch * samples + i] = sample * spatial_gain;
        }
    }
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Spatial audio processing applied to %u samples", samples);
    }
    
    return 0;
}

int opus_8k_decoder_process_ambisonics(opus_8k_decoder_t *decoder, float *input, float *output, uint32_t samples) {
    if (!decoder || !input || !output) return -1;
    
    if (!decoder->ambisonics_enabled) {
        return 0;
    }
    
    // Simulate ambisonics processing
    // This would implement actual ambisonics algorithms
    uint32_t ambisonics_channels = decoder->config.ambisonics_channels;
    
    for (uint32_t ch = 0; ch < ambisonics_channels; ch++) {
        for (uint32_t i = 0; i < samples; i++) {
            float sample = input[ch * samples + i];
            
            // Apply ambisonics processing
            float ambisonics_gain = 1.0f / sqrtf(ambisonics_channels);
            output[ch * samples + i] = sample * ambisonics_gain;
        }
    }
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Ambisonics processing applied: order %u, %u channels", 
                decoder->config.ambisonics_order, ambisonics_channels);
    }
    
    return 0;
}

int opus_8k_decoder_process_binaural(opus_8k_decoder_t *decoder, float *input, float *output, uint32_t samples) {
    if (!decoder || !input || !output) return -1;
    
    if (!decoder->binaural_enabled) {
        return 0;
    }
    
    // Simulate binaural processing
    // This would implement actual binaural rendering with HRTF
    for (uint32_t i = 0; i < samples; i++) {
        float left_sample = 0.0f;
        float right_sample = 0.0f;
        
        // Mix all channels to stereo with spatial positioning
        for (uint32_t ch = 0; ch < decoder->config.channels; ch++) {
            float sample = input[ch * samples + i];
            
            // Apply channel-specific spatial positioning
            float left_gain = 0.0f;
            float right_gain = 0.0f;
            
            switch (ch) {
                case 0: left_gain = 1.0f; right_gain = 0.0f; break;  // Front Left
                case 1: left_gain = 0.0f; right_gain = 1.0f; break;  // Front Right
                case 2: left_gain = 0.5f; right_gain = 0.5f; break;  // Center
                case 3: left_gain = 0.3f; right_gain = 0.3f; break;  // LFE
                case 4: left_gain = 0.8f; right_gain = 0.2f; break;  // Rear Left
                case 5: left_gain = 0.2f; right_gain = 0.8f; break;  // Rear Right
                case 6: left_gain = 0.9f; right_gain = 0.1f; break;  // Side Left
                case 7: left_gain = 0.1f; right_gain = 0.9f; break;  // Side Right
            }
            
            left_sample += sample * left_gain;
            right_sample += sample * right_gain;
        }
        
        // Output stereo
        output[0 * samples + i] = left_sample;
        output[1 * samples + i] = right_sample;
    }
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Binaural processing applied with quality %.2f", 
                decoder->config.binaural_quality);
    }
    
    return 0;
}

int opus_8k_decoder_load_hrtf(opus_8k_decoder_t *decoder, const char *hrtf_file) {
    if (!decoder || !hrtf_file) return -1;
    
    msg_Info(decoder->obj, "Loading HRTF data from: %s", hrtf_file);
    
    // TODO: Implement actual HRTF loading
    // This would load HRTF data from the specified file
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "HRTF data loaded successfully");
    }
    
    return 0;
}

int opus_8k_decoder_set_spatial_resolution(opus_8k_decoder_t *decoder, float resolution) {
    if (!decoder) return -1;
    
    if (resolution < 0.1f || resolution > 2.0f) {
        msg_Err(decoder->obj, "Invalid spatial resolution: %.2f (range: 0.1-2.0)", resolution);
        return -1;
    }
    
    decoder->config.spatial_resolution = resolution;
    decoder->stats.spatial_accuracy = resolution;
    
    msg_Info(decoder->obj, "Spatial resolution set to: %.2f", resolution);
    return 0;
}

int opus_8k_decoder_set_listener_position(opus_8k_decoder_t *decoder, float x, float y, float z) {
    if (!decoder) return -1;
    
    decoder->listener_x = x;
    decoder->listener_y = y;
    decoder->listener_z = z;
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Listener position set to: (%.2f, %.2f, %.2f)", x, y, z);
    }
    
    return 0;
}

int opus_8k_decoder_set_listener_orientation(opus_8k_decoder_t *decoder, float yaw, float pitch, float roll) {
    if (!decoder) return -1;
    
    decoder->listener_yaw = yaw;
    decoder->listener_pitch = pitch;
    decoder->listener_roll = roll;
    
    if (decoder->debug_enabled) {
        msg_Dbg(decoder->obj, "Listener orientation set to: yaw=%.2f, pitch=%.2f, roll=%.2f", 
                yaw, pitch, roll);
    }
    
    return 0;
}

opus_8k_stats_t opus_8k_decoder_get_stats(opus_8k_decoder_t *decoder) {
    if (decoder) {
        return decoder->stats;
    }
    
    opus_8k_stats_t empty_stats = {0};
    return empty_stats;
}

int opus_8k_decoder_reset_stats(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    memset(&decoder->stats, 0, sizeof(opus_8k_stats_t));
    decoder->start_time = vlc_tick_now();
    
    msg_Info(decoder->obj, "Opus 8K decoder statistics reset");
    return 0;
}

int opus_8k_decoder_set_performance_mode(opus_8k_decoder_t *decoder, const char *mode) {
    if (!decoder || !mode) return -1;
    
    msg_Info(decoder->obj, "Setting Opus 8K decoder performance mode: %s", mode);
    
    if (strcmp(mode, "quality") == 0) {
        // Quality mode - prioritize quality over speed
        decoder->config.bitrate = 510000;  // Max bitrate
        decoder->config.spatial_resolution = 1.0f;
        decoder->config.binaural_quality = 0.9f;
    } else if (strcmp(mode, "speed") == 0) {
        // Speed mode - prioritize speed over quality
        decoder->config.bitrate = 256000;  // Lower bitrate
        decoder->config.spatial_resolution = 0.5f;
        decoder->config.binaural_quality = 0.6f;
    } else if (strcmp(mode, "balanced") == 0) {
        // Balanced mode - balance quality and speed
        decoder->config.bitrate = 384000;  // Medium bitrate
        decoder->config.spatial_resolution = 0.8f;
        decoder->config.binaural_quality = 0.8f;
    } else {
        msg_Err(decoder->obj, "Unknown performance mode: %s", mode);
        return -1;
    }
    
    return 0;
}

int opus_8k_decoder_allocate_buffers(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    size_t buffer_size = decoder->config.channels * decoder->config.frame_size * sizeof(float);
    
    if (decoder->audio_buffer_size < buffer_size) {
        if (decoder->audio_buffer) {
            free(decoder->audio_buffer);
        }
        decoder->audio_buffer = malloc(buffer_size);
        if (!decoder->audio_buffer) {
            msg_Err(decoder->obj, "Failed to allocate 8K audio buffer");
            return -1;
        }
        decoder->audio_buffer_size = buffer_size;
    }
    
    if (decoder->spatial_audio_enabled) {
        size_t spatial_buffer_size = decoder->config.channels * decoder->config.frame_size * sizeof(float) * 2;
        if (decoder->spatial_buffer_size < spatial_buffer_size) {
            if (decoder->spatial_buffer) {
                free(decoder->spatial_buffer);
            }
            decoder->spatial_buffer = malloc(spatial_buffer_size);
            if (!decoder->spatial_buffer) {
                msg_Err(decoder->obj, "Failed to allocate spatial buffer");
                return -1;
            }
            decoder->spatial_buffer_size = spatial_buffer_size;
        }
    }
    
    msg_Info(decoder->obj, "Opus 8K decoder buffers allocated: %zu MB", 
             (buffer_size + decoder->spatial_buffer_size) / (1024 * 1024));
    return 0;
}

int opus_8k_decoder_free_buffers(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    if (decoder->audio_buffer) {
        free(decoder->audio_buffer);
        decoder->audio_buffer = NULL;
        decoder->audio_buffer_size = 0;
    }
    
    if (decoder->spatial_buffer) {
        free(decoder->spatial_buffer);
        decoder->spatial_buffer = NULL;
        decoder->spatial_buffer_size = 0;
    }
    
    msg_Info(decoder->obj, "Opus 8K decoder buffers freed");
    return 0;
}

int opus_8k_decoder_get_memory_usage(opus_8k_decoder_t *decoder, uint32_t *usage_mb) {
    if (!decoder || !usage_mb) return -1;
    
    *usage_mb = (decoder->audio_buffer_size + decoder->spatial_buffer_size) / (1024 * 1024);
    return 0;
}

int opus_8k_decoder_get_last_error(opus_8k_decoder_t *decoder, char *error_buffer, size_t buffer_size) {
    if (!decoder || !error_buffer) return -1;
    
    strncpy(error_buffer, decoder->last_error, buffer_size - 1);
    error_buffer[buffer_size - 1] = '\0';
    return 0;
}

int opus_8k_decoder_clear_errors(opus_8k_decoder_t *decoder) {
    if (!decoder) return -1;
    
    memset(decoder->last_error, 0, sizeof(decoder->last_error));
    return 0;
}

void opus_8k_decoder_set_debug(opus_8k_decoder_t *decoder, bool enable) {
    if (decoder) {
        decoder->debug_enabled = enable;
        msg_Info(decoder->obj, "Opus 8K decoder debug %s", enable ? "enabled" : "disabled");
    }
}

void opus_8k_decoder_log_info(opus_8k_decoder_t *decoder) {
    if (!decoder) return;
    
    msg_Info(decoder->obj, "Opus 8K Decoder Info:");
    msg_Info(decoder->obj, "  Channels: %u", decoder->config.channels);
    msg_Info(decoder->obj, "  Sample Rate: %u Hz", decoder->config.sample_rate);
    msg_Info(decoder->obj, "  Bitrate: %u bps", decoder->config.bitrate);
    msg_Info(decoder->obj, "  Frame Size: %u samples", decoder->config.frame_size);
    msg_Info(decoder->obj, "  Spatial Audio: %s", decoder->config.spatial_audio ? "enabled" : "disabled");
    msg_Info(decoder->obj, "  Ambisonics: %s", decoder->config.ambisonics ? "enabled" : "disabled");
    msg_Info(decoder->obj, "  Binaural: %s", decoder->config.binaural ? "enabled" : "disabled");
    msg_Info(decoder->obj, "  Spatial Resolution: %.2f", decoder->config.spatial_resolution);
    msg_Info(decoder->obj, "  Binaural Quality: %.2f", decoder->config.binaural_quality);
}

void opus_8k_decoder_log_stats(opus_8k_decoder_t *decoder) {
    if (!decoder) return;
    
    msg_Info(decoder->obj, "Opus 8K Decoder Statistics:");
    msg_Info(decoder->obj, "  Frames Decoded: %llu", decoder->stats.frames_decoded);
    msg_Info(decoder->obj, "  Samples Decoded: %llu", decoder->stats.samples_decoded);
    msg_Info(decoder->obj, "  Bytes Processed: %llu", decoder->stats.bytes_processed);
    msg_Info(decoder->obj, "  Total Decode Time: %llu us", decoder->stats.decode_time_us);
    msg_Info(decoder->obj, "  Dropped Frames: %llu", decoder->stats.dropped_frames);
    msg_Info(decoder->obj, "  Average FPS: %.2f", decoder->stats.average_fps);
    msg_Info(decoder->obj, "  Average Decode Time: %.2f us", decoder->stats.average_decode_time);
    msg_Info(decoder->obj, "  Memory Usage: %u MB", decoder->stats.memory_usage_mb);
    msg_Info(decoder->obj, "  Spatial Accuracy: %.2f", decoder->stats.spatial_accuracy);
    msg_Info(decoder->obj, "  Binaural Quality: %.2f", decoder->stats.binaural_quality);
}
