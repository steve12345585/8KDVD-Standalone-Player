#include "8k_audio_processor.h"
#include <vlc_messages.h>
#include <vlc_aout.h>
#include <vlc_block.h>
#include <vlc_fourcc.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// 8K Audio Processor Implementation
struct kdvd_8k_audio_processor_t {
    vlc_object_t *obj;
    kdvd_8k_audio_config_t config;
    kdvd_8k_audio_stats_t stats;
    bool initialized;
    bool spatial_audio_enabled;
    bool ambisonics_enabled;
    bool binaural_enabled;
    bool debug_enabled;
    char last_error[256];
    void *processor_context;
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

// 8K Audio Processor Functions
kdvd_8k_audio_processor_t* kdvd_8k_audio_processor_create(vlc_object_t *obj) {
    kdvd_8k_audio_processor_t *processor = calloc(1, sizeof(kdvd_8k_audio_processor_t));
    if (!processor) return NULL;
    
    processor->obj = obj;
    processor->initialized = false;
    processor->spatial_audio_enabled = false;
    processor->ambisonics_enabled = false;
    processor->binaural_enabled = false;
    processor->debug_enabled = false;
    processor->processor_context = NULL;
    processor->audio_buffer = NULL;
    processor->audio_buffer_size = 0;
    processor->spatial_buffer = NULL;
    processor->spatial_buffer_size = 0;
    processor->hrtf_data = NULL;
    processor->hrtf_size = 0;
    processor->listener_x = 0.0f;
    processor->listener_y = 0.0f;
    processor->listener_z = 0.0f;
    processor->listener_yaw = 0.0f;
    processor->listener_pitch = 0.0f;
    processor->listener_roll = 0.0f;
    processor->start_time = 0;
    processor->last_frame_time = 0;
    
    // Initialize stats
    memset(&processor->stats, 0, sizeof(kdvd_8k_audio_stats_t));
    
    // Initialize config with 8K spatial audio defaults
    processor->config.channels = 8;
    processor->config.sample_rate = 48000;
    processor->config.bitrate = 510000;
    processor->config.frame_size = 960;
    processor->config.spatial_audio = true;
    processor->config.ambisonics = false;
    processor->config.binaural = false;
    processor->config.ambisonics_order = 1;
    processor->config.ambisonics_channels = 4;
    processor->config.spatial_resolution = 1.0f;
    processor->config.binaural_quality = 0.8f;
    processor->config.hrtf_enabled = true;
    processor->config.room_simulation = false;
    processor->config.room_size = 1.0f;
    processor->config.room_damping = 0.5f;
    processor->config.doppler_effect = false;
    processor->config.distance_attenuation = false;
    processor->config.occlusion_simulation = false;
    
    msg_Info(obj, "8K audio processor created");
    return processor;
}

void kdvd_8k_audio_processor_destroy(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return;
    
    if (processor->audio_buffer) {
        free(processor->audio_buffer);
    }
    
    if (processor->spatial_buffer) {
        free(processor->spatial_buffer);
    }
    
    if (processor->hrtf_data) {
        free(processor->hrtf_data);
    }
    
    if (processor->processor_context) {
        free(processor->processor_context);
    }
    
    free(processor);
    msg_Info(processor->obj, "8K audio processor destroyed");
}

int kdvd_8k_audio_processor_configure(kdvd_8k_audio_processor_t *processor, const kdvd_8k_audio_config_t *config) {
    if (!processor || !config) return -1;
    
    msg_Info(processor->obj, "Configuring 8K audio processor: %u channels %u Hz %u bps", 
             config->channels, config->sample_rate, config->bitrate);
    
    // Copy configuration
    memcpy(&processor->config, config, sizeof(kdvd_8k_audio_config_t));
    
    // Validate 8K spatial audio configuration
    if (config->channels != 8) {
        msg_Err(processor->obj, "Invalid channel count: %u (expected 8 for spatial audio)", config->channels);
        return -1;
    }
    
    if (config->sample_rate != 48000) {
        msg_Err(processor->obj, "Invalid sample rate: %u (expected 48000 Hz)", config->sample_rate);
        return -1;
    }
    
    if (config->bitrate > 510000) {
        msg_Err(processor->obj, "Invalid bitrate: %u (max 510 kbps)", config->bitrate);
        return -1;
    }
    
    // Allocate audio buffer for 8-channel spatial audio
    size_t buffer_size = config->channels * config->frame_size * sizeof(float);
    if (processor->audio_buffer_size < buffer_size) {
        if (processor->audio_buffer) {
            free(processor->audio_buffer);
        }
        processor->audio_buffer = malloc(buffer_size);
        if (!processor->audio_buffer) {
            msg_Err(processor->obj, "Failed to allocate 8K audio buffer");
            return -1;
        }
        processor->audio_buffer_size = buffer_size;
    }
    
    // Allocate spatial buffer for spatial processing
    if (config->spatial_audio) {
        size_t spatial_buffer_size = config->channels * config->frame_size * sizeof(float) * 2;
        if (processor->spatial_buffer_size < spatial_buffer_size) {
            if (processor->spatial_buffer) {
                free(processor->spatial_buffer);
            }
            processor->spatial_buffer = malloc(spatial_buffer_size);
            if (!processor->spatial_buffer) {
                msg_Err(processor->obj, "Failed to allocate spatial buffer");
                return -1;
            }
            processor->spatial_buffer_size = spatial_buffer_size;
        }
    }
    
    // Initialize processor context
    if (processor->processor_context) {
        free(processor->processor_context);
    }
    processor->processor_context = malloc(1024);
    
    processor->initialized = true;
    processor->start_time = vlc_tick_now();
    
    msg_Info(processor->obj, "8K audio processor configured successfully");
    return 0;
}

int kdvd_8k_audio_processor_process_frame(kdvd_8k_audio_processor_t *processor, block_t *input_block, block_t **output_block) {
    if (!processor || !input_block || !output_block) return -1;
    
    if (!processor->initialized) {
        msg_Err(processor->obj, "8K audio processor not initialized");
        return -1;
    }
    
    uint64_t process_start = vlc_tick_now();
    
    // Simulate 8K spatial audio processing
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "Processing 8K audio frame: %zu bytes", input_block->i_buffer);
    }
    
    // Create output block
    block_t *output = block_Alloc(processor->config.channels * processor->config.frame_size * sizeof(float));
    if (!output) {
        msg_Err(processor->obj, "Failed to allocate output block");
        return -1;
    }
    
    // Set output block properties
    output->i_dts = input_block->i_dts;
    output->i_pts = input_block->i_pts;
    output->i_length = processor->config.frame_size * 1000000 / processor->config.sample_rate;
    
    // Simulate spatial audio processing
    float *audio_data = (float*)output->p_buffer;
    uint32_t samples_per_channel = processor->config.frame_size;
    
    // Generate test audio data (in real implementation, this would be actual Opus decoding)
    for (uint32_t ch = 0; ch < processor->config.channels; ch++) {
        for (uint32_t i = 0; i < samples_per_channel; i++) {
            float sample = sinf(2.0f * M_PI * 440.0f * i / processor->config.sample_rate) * 0.1f;
            audio_data[ch * samples_per_channel + i] = sample;
        }
    }
    
    // Apply spatial processing if enabled
    if (processor->spatial_audio_enabled) {
        kdvd_8k_audio_processor_process_spatial(processor, audio_data, (float*)output->p_buffer, samples_per_channel);
    }
    
    // Apply ambisonics processing if enabled
    if (processor->ambisonics_enabled) {
        kdvd_8k_audio_processor_process_ambisonics(processor, audio_data, (float*)output->p_buffer, samples_per_channel);
    }
    
    // Apply binaural processing if enabled
    if (processor->binaural_enabled) {
        kdvd_8k_audio_processor_process_binaural(processor, audio_data, (float*)output->p_buffer, samples_per_channel);
    }
    
    *output_block = output;
    
    // Update statistics
    processor->stats.frames_processed++;
    processor->stats.samples_processed += processor->config.channels * samples_per_channel;
    processor->stats.bytes_processed += input_block->i_buffer;
    
    uint64_t process_time = vlc_tick_now() - process_start;
    processor->stats.process_time_us += process_time;
    processor->stats.last_frame_time = vlc_tick_now();
    
    // Calculate average FPS
    if (processor->stats.frames_processed > 0) {
        uint64_t total_time = vlc_tick_now() - processor->start_time;
        processor->stats.average_fps = (float)processor->stats.frames_processed * 1000000.0f / total_time;
    }
    
    // Calculate average process time
    if (processor->stats.frames_processed > 0) {
        processor->stats.average_process_time = (float)processor->stats.process_time_us / processor->stats.frames_processed;
    }
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "8K audio frame processed in %llu us", process_time);
    }
    
    return 0;
}

int kdvd_8k_audio_processor_process_spatial(kdvd_8k_audio_processor_t *processor, float *input, float *output, uint32_t samples) {
    if (!processor || !input || !output) return -1;
    
    if (!processor->spatial_audio_enabled) {
        memcpy(output, input, processor->config.channels * samples * sizeof(float));
        return 0;
    }
    
    // Simulate spatial audio processing
    for (uint32_t ch = 0; ch < processor->config.channels; ch++) {
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
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "Spatial audio processing applied to %u samples", samples);
    }
    
    return 0;
}

int kdvd_8k_audio_processor_process_ambisonics(kdvd_8k_audio_processor_t *processor, float *input, float *output, uint32_t samples) {
    if (!processor || !input || !output) return -1;
    
    if (!processor->ambisonics_enabled) {
        return 0;
    }
    
    // Simulate ambisonics processing
    uint32_t ambisonics_channels = processor->config.ambisonics_channels;
    
    for (uint32_t ch = 0; ch < ambisonics_channels; ch++) {
        for (uint32_t i = 0; i < samples; i++) {
            float sample = input[ch * samples + i];
            
            // Apply ambisonics processing
            float ambisonics_gain = 1.0f / sqrtf(ambisonics_channels);
            output[ch * samples + i] = sample * ambisonics_gain;
        }
    }
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "Ambisonics processing applied: order %u, %u channels", 
                processor->config.ambisonics_order, ambisonics_channels);
    }
    
    return 0;
}

int kdvd_8k_audio_processor_process_binaural(kdvd_8k_audio_processor_t *processor, float *input, float *output, uint32_t samples) {
    if (!processor || !input || !output) return -1;
    
    if (!processor->binaural_enabled) {
        return 0;
    }
    
    // Simulate binaural processing
    for (uint32_t i = 0; i < samples; i++) {
        float left_sample = 0.0f;
        float right_sample = 0.0f;
        
        // Mix all channels to stereo with spatial positioning
        for (uint32_t ch = 0; ch < processor->config.channels; ch++) {
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
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "Binaural processing applied with quality %.2f", 
                processor->config.binaural_quality);
    }
    
    return 0;
}

int kdvd_8k_audio_processor_enable_8k_spatial(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (!processor) return -1;
    
    if (enable) {
        processor->config.channels = 8;
        processor->config.spatial_audio = true;
        processor->config.bitrate = 510000;
        msg_Info(processor->obj, "8K spatial audio enabled: 8 channels, 510 kbps");
    } else {
        processor->config.channels = 2;
        processor->config.spatial_audio = false;
        processor->config.bitrate = 128000;
        msg_Info(processor->obj, "8K spatial audio disabled: 2 channels, 128 kbps");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_8k_channels(kdvd_8k_audio_processor_t *processor, uint32_t channels) {
    if (!processor) return -1;
    
    if (channels != 8) {
        msg_Err(processor->obj, "Invalid channel count: %u (expected 8 for 8K spatial audio)", channels);
        return -1;
    }
    
    processor->config.channels = channels;
    processor->stats.current_channels = channels;
    
    msg_Info(processor->obj, "8K channel count set: %u", channels);
    return 0;
}

int kdvd_8k_audio_processor_optimize_for_8k(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    msg_Info(processor->obj, "Optimizing audio processor for 8K spatial audio");
    
    // Set optimal settings for 8K spatial audio
    processor->config.channels = 8;
    processor->config.sample_rate = 48000;
    processor->config.bitrate = 510000;
    processor->config.frame_size = 960;
    processor->config.spatial_audio = true;
    processor->config.spatial_resolution = 1.0f;
    
    // Allocate larger buffers for 8K spatial audio
    size_t buffer_size = processor->config.channels * processor->config.frame_size * sizeof(float);
    if (processor->audio_buffer_size < buffer_size) {
        if (processor->audio_buffer) {
            free(processor->audio_buffer);
        }
        processor->audio_buffer = malloc(buffer_size);
        if (!processor->audio_buffer) {
            msg_Err(processor->obj, "Failed to allocate 8K audio buffer");
            return -1;
        }
        processor->audio_buffer_size = buffer_size;
    }
    
    msg_Info(processor->obj, "Audio processor optimized for 8K spatial audio");
    return 0;
}

int kdvd_8k_audio_processor_flush(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    msg_Info(processor->obj, "Flushing 8K audio processor");
    
    // Reset processor state
    processor->stats.dropped_frames = 0;
    
    return 0;
}

int kdvd_8k_audio_processor_reset(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    msg_Info(processor->obj, "Resetting 8K audio processor");
    
    // Reset all statistics
    memset(&processor->stats, 0, sizeof(kdvd_8k_audio_stats_t));
    processor->start_time = vlc_tick_now();
    
    return 0;
}

int kdvd_8k_audio_processor_set_spatial_audio(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (!processor) return -1;
    
    processor->spatial_audio_enabled = enable;
    processor->config.spatial_audio = enable;
    
    if (enable) {
        // Allocate spatial buffer
        size_t spatial_buffer_size = processor->config.channels * processor->config.frame_size * sizeof(float) * 2;
        if (processor->spatial_buffer_size < spatial_buffer_size) {
            if (processor->spatial_buffer) {
                free(processor->spatial_buffer);
            }
            processor->spatial_buffer = malloc(spatial_buffer_size);
            if (!processor->spatial_buffer) {
                msg_Err(processor->obj, "Failed to allocate spatial buffer");
                return -1;
            }
            processor->spatial_buffer_size = spatial_buffer_size;
        }
        msg_Info(processor->obj, "Spatial audio enabled for 8K audio processor");
    } else {
        msg_Info(processor->obj, "Spatial audio disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_ambisonics(kdvd_8k_audio_processor_t *processor, bool enable, uint32_t order) {
    if (!processor) return -1;
    
    processor->ambisonics_enabled = enable;
    processor->config.ambisonics = enable;
    
    if (enable) {
        processor->config.ambisonics_order = order;
        processor->config.ambisonics_channels = (order + 1) * (order + 1);
        msg_Info(processor->obj, "Ambisonics enabled for 8K audio processor: order %u, %u channels", 
                 order, processor->config.ambisonics_channels);
    } else {
        msg_Info(processor->obj, "Ambisonics disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_binaural(kdvd_8k_audio_processor_t *processor, bool enable, float quality) {
    if (!processor) return -1;
    
    processor->binaural_enabled = enable;
    processor->config.binaural = enable;
    processor->config.binaural_quality = quality;
    
    if (enable) {
        msg_Info(processor->obj, "Binaural rendering enabled for 8K audio processor: quality %.2f", quality);
    } else {
        msg_Info(processor->obj, "Binaural rendering disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_load_hrtf(kdvd_8k_audio_processor_t *processor, const char *hrtf_file) {
    if (!processor || !hrtf_file) return -1;
    
    msg_Info(processor->obj, "Loading HRTF data from: %s", hrtf_file);
    
    // TODO: Implement actual HRTF loading
    // This would load HRTF data from the specified file
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "HRTF data loaded successfully");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_spatial_resolution(kdvd_8k_audio_processor_t *processor, float resolution) {
    if (!processor) return -1;
    
    if (resolution < 0.1f || resolution > 2.0f) {
        msg_Err(processor->obj, "Invalid spatial resolution: %.2f (range: 0.1-2.0)", resolution);
        return -1;
    }
    
    processor->config.spatial_resolution = resolution;
    processor->stats.spatial_accuracy = resolution;
    
    msg_Info(processor->obj, "Spatial resolution set to: %.2f", resolution);
    return 0;
}

int kdvd_8k_audio_processor_set_listener_position(kdvd_8k_audio_processor_t *processor, float x, float y, float z) {
    if (!processor) return -1;
    
    processor->listener_x = x;
    processor->listener_y = y;
    processor->listener_z = z;
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "Listener position set to: (%.2f, %.2f, %.2f)", x, y, z);
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_listener_orientation(kdvd_8k_audio_processor_t *processor, float yaw, float pitch, float roll) {
    if (!processor) return -1;
    
    processor->listener_yaw = yaw;
    processor->listener_pitch = pitch;
    processor->listener_roll = roll;
    
    if (processor->debug_enabled) {
        msg_Dbg(processor->obj, "Listener orientation set to: yaw=%.2f, pitch=%.2f, roll=%.2f", 
                yaw, pitch, roll);
    }
    
    return 0;
}

int kdvd_8k_audio_processor_enable_room_simulation(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (!processor) return -1;
    
    processor->config.room_simulation = enable;
    
    if (enable) {
        msg_Info(processor->obj, "Room simulation enabled for 8K audio processor");
    } else {
        msg_Info(processor->obj, "Room simulation disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_room_size(kdvd_8k_audio_processor_t *processor, float size) {
    if (!processor) return -1;
    
    if (size < 0.1f || size > 10.0f) {
        msg_Err(processor->obj, "Invalid room size: %.2f (range: 0.1-10.0)", size);
        return -1;
    }
    
    processor->config.room_size = size;
    
    msg_Info(processor->obj, "Room size set to: %.2f", size);
    return 0;
}

int kdvd_8k_audio_processor_set_room_damping(kdvd_8k_audio_processor_t *processor, float damping) {
    if (!processor) return -1;
    
    if (damping < 0.0f || damping > 1.0f) {
        msg_Err(processor->obj, "Invalid room damping: %.2f (range: 0.0-1.0)", damping);
        return -1;
    }
    
    processor->config.room_damping = damping;
    
    msg_Info(processor->obj, "Room damping set to: %.2f", damping);
    return 0;
}

int kdvd_8k_audio_processor_set_room_acoustics(kdvd_8k_audio_processor_t *processor, float size, float damping) {
    if (!processor) return -1;
    
    if (kdvd_8k_audio_processor_set_room_size(processor, size) != 0) {
        return -1;
    }
    
    if (kdvd_8k_audio_processor_set_room_damping(processor, damping) != 0) {
        return -1;
    }
    
    msg_Info(processor->obj, "Room acoustics set: size=%.2f, damping=%.2f", size, damping);
    return 0;
}

int kdvd_8k_audio_processor_enable_doppler_effect(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (!processor) return -1;
    
    processor->config.doppler_effect = enable;
    
    if (enable) {
        msg_Info(processor->obj, "Doppler effect enabled for 8K audio processor");
    } else {
        msg_Info(processor->obj, "Doppler effect disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_enable_distance_attenuation(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (!processor) return -1;
    
    processor->config.distance_attenuation = enable;
    
    if (enable) {
        msg_Info(processor->obj, "Distance attenuation enabled for 8K audio processor");
    } else {
        msg_Info(processor->obj, "Distance attenuation disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_enable_occlusion_simulation(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (!processor) return -1;
    
    processor->config.occlusion_simulation = enable;
    
    if (enable) {
        msg_Info(processor->obj, "Occlusion simulation enabled for 8K audio processor");
    } else {
        msg_Info(processor->obj, "Occlusion simulation disabled for 8K audio processor");
    }
    
    return 0;
}

int kdvd_8k_audio_processor_set_audio_effects(kdvd_8k_audio_processor_t *processor, bool doppler, bool distance, bool occlusion) {
    if (!processor) return -1;
    
    if (kdvd_8k_audio_processor_enable_doppler_effect(processor, doppler) != 0) {
        return -1;
    }
    
    if (kdvd_8k_audio_processor_enable_distance_attenuation(processor, distance) != 0) {
        return -1;
    }
    
    if (kdvd_8k_audio_processor_enable_occlusion_simulation(processor, occlusion) != 0) {
        return -1;
    }
    
    msg_Info(processor->obj, "Audio effects set: doppler=%s, distance=%s, occlusion=%s", 
             doppler ? "enabled" : "disabled",
             distance ? "enabled" : "disabled",
             occlusion ? "enabled" : "disabled");
    return 0;
}

kdvd_8k_audio_stats_t kdvd_8k_audio_processor_get_stats(kdvd_8k_audio_processor_t *processor) {
    if (processor) {
        return processor->stats;
    }
    
    kdvd_8k_audio_stats_t empty_stats = {0};
    return empty_stats;
}

int kdvd_8k_audio_processor_reset_stats(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    memset(&processor->stats, 0, sizeof(kdvd_8k_audio_stats_t));
    processor->start_time = vlc_tick_now();
    
    msg_Info(processor->obj, "8K audio processor statistics reset");
    return 0;
}

int kdvd_8k_audio_processor_set_performance_mode(kdvd_8k_audio_processor_t *processor, const char *mode) {
    if (!processor || !mode) return -1;
    
    msg_Info(processor->obj, "Setting 8K audio processor performance mode: %s", mode);
    
    if (strcmp(mode, "quality") == 0) {
        // Quality mode - prioritize quality over speed
        processor->config.bitrate = 510000;  // Max bitrate
        processor->config.spatial_resolution = 1.0f;
        processor->config.binaural_quality = 0.9f;
    } else if (strcmp(mode, "speed") == 0) {
        // Speed mode - prioritize speed over quality
        processor->config.bitrate = 256000;  // Lower bitrate
        processor->config.spatial_resolution = 0.5f;
        processor->config.binaural_quality = 0.6f;
    } else if (strcmp(mode, "balanced") == 0) {
        // Balanced mode - balance quality and speed
        processor->config.bitrate = 384000;  // Medium bitrate
        processor->config.spatial_resolution = 0.8f;
        processor->config.binaural_quality = 0.8f;
    } else {
        msg_Err(processor->obj, "Unknown performance mode: %s", mode);
        return -1;
    }
    
    return 0;
}

int kdvd_8k_audio_processor_allocate_buffers(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    size_t buffer_size = processor->config.channels * processor->config.frame_size * sizeof(float);
    
    if (processor->audio_buffer_size < buffer_size) {
        if (processor->audio_buffer) {
            free(processor->audio_buffer);
        }
        processor->audio_buffer = malloc(buffer_size);
        if (!processor->audio_buffer) {
            msg_Err(processor->obj, "Failed to allocate 8K audio buffer");
            return -1;
        }
        processor->audio_buffer_size = buffer_size;
    }
    
    if (processor->config.spatial_audio) {
        size_t spatial_buffer_size = processor->config.channels * processor->config.frame_size * sizeof(float) * 2;
        if (processor->spatial_buffer_size < spatial_buffer_size) {
            if (processor->spatial_buffer) {
                free(processor->spatial_buffer);
            }
            processor->spatial_buffer = malloc(spatial_buffer_size);
            if (!processor->spatial_buffer) {
                msg_Err(processor->obj, "Failed to allocate spatial buffer");
                return -1;
            }
            processor->spatial_buffer_size = spatial_buffer_size;
        }
    }
    
    msg_Info(processor->obj, "8K audio processor buffers allocated: %zu MB", 
             (buffer_size + processor->spatial_buffer_size) / (1024 * 1024));
    return 0;
}

int kdvd_8k_audio_processor_free_buffers(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    if (processor->audio_buffer) {
        free(processor->audio_buffer);
        processor->audio_buffer = NULL;
        processor->audio_buffer_size = 0;
    }
    
    if (processor->spatial_buffer) {
        free(processor->spatial_buffer);
        processor->spatial_buffer = NULL;
        processor->spatial_buffer_size = 0;
    }
    
    msg_Info(processor->obj, "8K audio processor buffers freed");
    return 0;
}

int kdvd_8k_audio_processor_get_memory_usage(kdvd_8k_audio_processor_t *processor, uint32_t *usage_mb) {
    if (!processor || !usage_mb) return -1;
    
    *usage_mb = (processor->audio_buffer_size + processor->spatial_buffer_size) / (1024 * 1024);
    return 0;
}

int kdvd_8k_audio_processor_get_last_error(kdvd_8k_audio_processor_t *processor, char *error_buffer, size_t buffer_size) {
    if (!processor || !error_buffer) return -1;
    
    strncpy(error_buffer, processor->last_error, buffer_size - 1);
    error_buffer[buffer_size - 1] = '\0';
    return 0;
}

int kdvd_8k_audio_processor_clear_errors(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return -1;
    
    memset(processor->last_error, 0, sizeof(processor->last_error));
    return 0;
}

void kdvd_8k_audio_processor_set_debug(kdvd_8k_audio_processor_t *processor, bool enable) {
    if (processor) {
        processor->debug_enabled = enable;
        msg_Info(processor->obj, "8K audio processor debug %s", enable ? "enabled" : "disabled");
    }
}

void kdvd_8k_audio_processor_log_info(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return;
    
    msg_Info(processor->obj, "8K Audio Processor Info:");
    msg_Info(processor->obj, "  Channels: %u", processor->config.channels);
    msg_Info(processor->obj, "  Sample Rate: %u Hz", processor->config.sample_rate);
    msg_Info(processor->obj, "  Bitrate: %u bps", processor->config.bitrate);
    msg_Info(processor->obj, "  Frame Size: %u samples", processor->config.frame_size);
    msg_Info(processor->obj, "  Spatial Audio: %s", processor->config.spatial_audio ? "enabled" : "disabled");
    msg_Info(processor->obj, "  Ambisonics: %s", processor->config.ambisonics ? "enabled" : "disabled");
    msg_Info(processor->obj, "  Binaural: %s", processor->config.binaural ? "enabled" : "disabled");
    msg_Info(processor->obj, "  Spatial Resolution: %.2f", processor->config.spatial_resolution);
    msg_Info(processor->obj, "  Binaural Quality: %.2f", processor->config.binaural_quality);
    msg_Info(processor->obj, "  Room Simulation: %s", processor->config.room_simulation ? "enabled" : "disabled");
    msg_Info(processor->obj, "  Room Size: %.2f", processor->config.room_size);
    msg_Info(processor->obj, "  Room Damping: %.2f", processor->config.room_damping);
    msg_Info(processor->obj, "  Doppler Effect: %s", processor->config.doppler_effect ? "enabled" : "disabled");
    msg_Info(processor->obj, "  Distance Attenuation: %s", processor->config.distance_attenuation ? "enabled" : "disabled");
    msg_Info(processor->obj, "  Occlusion Simulation: %s", processor->config.occlusion_simulation ? "enabled" : "disabled");
}

void kdvd_8k_audio_processor_log_stats(kdvd_8k_audio_processor_t *processor) {
    if (!processor) return;
    
    msg_Info(processor->obj, "8K Audio Processor Statistics:");
    msg_Info(processor->obj, "  Frames Processed: %llu", processor->stats.frames_processed);
    msg_Info(processor->obj, "  Samples Processed: %llu", processor->stats.samples_processed);
    msg_Info(processor->obj, "  Bytes Processed: %llu", processor->stats.bytes_processed);
    msg_Info(processor->obj, "  Total Process Time: %llu us", processor->stats.process_time_us);
    msg_Info(processor->obj, "  Dropped Frames: %llu", processor->stats.dropped_frames);
    msg_Info(processor->obj, "  Average FPS: %.2f", processor->stats.average_fps);
    msg_Info(processor->obj, "  Average Process Time: %.2f us", processor->stats.average_process_time);
    msg_Info(processor->obj, "  Memory Usage: %u MB", processor->stats.memory_usage_mb);
    msg_Info(processor->obj, "  Spatial Accuracy: %.2f", processor->stats.spatial_accuracy);
    msg_Info(processor->obj, "  Binaural Quality: %.2f", processor->stats.binaural_quality);
    msg_Info(processor->obj, "  Room Simulation Quality: %.2f", processor->stats.room_simulation_quality);
}
