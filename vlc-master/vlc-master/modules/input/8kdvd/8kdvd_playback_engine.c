#include "8kdvd_playback_engine.h"
#include "8kdvd_disc_manager.h"
#include <vlc_messages.h>
#include <vlc_fs.h>
#include <vlc_meta.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// 8KDVD Playback Engine Implementation
struct kdvd_playback_engine_t {
    vlc_object_t *obj;
    kdvd_playback_state_t state;
    kdvd_playback_config_t config;
    kdvd_playback_stats_t stats;
    bool initialized;
    bool debug_enabled;
    char last_error[256];
    void *engine_context;  // Placeholder for actual engine context
    kdvd_disc_manager_t *disc_manager;
    uint64_t start_time;
    uint64_t last_frame_time;
    uint64_t frame_count;
    uint64_t audio_sample_count;
    uint64_t bytes_processed;
    uint32_t current_framerate;
    uint32_t target_framerate;
    bool hardware_acceleration_enabled;
    bool hdr_processing_enabled;
    bool spatial_audio_enabled;
    char gpu_device[128];
    uint32_t buffer_usage_percent;
    uint32_t cpu_usage_percent;
    uint32_t gpu_usage_percent;
    uint32_t memory_usage_mb;
    uint64_t seek_operations;
    uint64_t pause_operations;
    uint64_t error_count;
    float average_seek_time_ms;
    float average_pause_time_ms;
};

// 8KDVD Playback Engine Functions
kdvd_playback_engine_t* kdvd_playback_engine_create(vlc_object_t *obj) {
    kdvd_playback_engine_t *engine = calloc(1, sizeof(kdvd_playback_engine_t));
    if (!engine) return NULL;
    
    engine->obj = obj;
    engine->state = EIGHTKDVD_PLAYBACK_STOPPED;
    engine->initialized = false;
    engine->debug_enabled = false;
    engine->engine_context = NULL;
    engine->disc_manager = NULL;
    engine->start_time = 0;
    engine->last_frame_time = 0;
    engine->frame_count = 0;
    engine->audio_sample_count = 0;
    engine->bytes_processed = 0;
    engine->current_framerate = 0;
    engine->target_framerate = 60;
    engine->hardware_acceleration_enabled = false;
    engine->hdr_processing_enabled = false;
    engine->spatial_audio_enabled = false;
    engine->buffer_usage_percent = 0;
    engine->cpu_usage_percent = 0;
    engine->gpu_usage_percent = 0;
    engine->memory_usage_mb = 0;
    engine->seek_operations = 0;
    engine->pause_operations = 0;
    engine->error_count = 0;
    engine->average_seek_time_ms = 0.0f;
    engine->average_pause_time_ms = 0.0f;
    
    // Initialize default config
    engine->config.enable_hardware_acceleration = true;
    engine->config.enable_hdr_processing = true;
    engine->config.enable_spatial_audio = true;
    engine->config.enable_adaptive_bitrate = true;
    engine->config.buffer_size_mb = 1024; // 1GB buffer
    engine->config.max_bitrate_mbps = 100; // 100 Mbps
    engine->config.target_framerate = 60;
    engine->config.enable_vsync = true;
    engine->config.enable_adaptive_sync = true;
    engine->config.audio_channels = 8;
    engine->config.audio_sample_rate = 48000;
    engine->config.video_width = 7680;
    engine->config.video_height = 4320;
    engine->config.video_bit_depth = 10;
    engine->config.enable_dolby_vision = true;
    engine->config.enable_hdr10_plus = true;
    strcpy(engine->config.performance_mode, "balanced");
    
    // Initialize stats
    memset(&engine->stats, 0, sizeof(kdvd_playback_stats_t));
    
    engine->initialized = true;
    engine->start_time = vlc_tick_now();
    
    msg_Info(obj, "8KDVD playback engine created");
    return engine;
}

void kdvd_playback_engine_destroy(kdvd_playback_engine_t *engine) {
    if (!engine) return;
    
    if (engine->disc_manager) {
        kdvd_disc_manager_destroy(engine->disc_manager);
    }
    
    if (engine->engine_context) {
        free(engine->engine_context);
    }
    
    free(engine);
    msg_Info(engine->obj, "8KDVD playback engine destroyed");
}

int kdvd_playback_engine_play(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    msg_Info(engine->obj, "Starting 8KDVD playback");
    
    if (engine->state == EIGHTKDVD_PLAYBACK_PLAYING) {
        msg_Warn(engine->obj, "Playback already in progress");
        return 0;
    }
    
    // Set state to playing
    engine->state = EIGHTKDVD_PLAYBACK_PLAYING;
    engine->start_time = vlc_tick_now();
    engine->last_frame_time = vlc_tick_now();
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Playback started");
        msg_Dbg(engine->obj, "Target framerate: %u fps", engine->target_framerate);
        msg_Dbg(engine->obj, "Hardware acceleration: %s", engine->hardware_acceleration_enabled ? "enabled" : "disabled");
        msg_Dbg(engine->obj, "HDR processing: %s", engine->hdr_processing_enabled ? "enabled" : "disabled");
        msg_Dbg(engine->obj, "Spatial audio: %s", engine->spatial_audio_enabled ? "enabled" : "disabled");
    }
    
    msg_Info(engine->obj, "8KDVD playback started");
    return 0;
}

int kdvd_playback_engine_pause(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    msg_Info(engine->obj, "Pausing 8KDVD playback");
    
    if (engine->state != EIGHTKDVD_PLAYBACK_PLAYING) {
        msg_Warn(engine->obj, "Playback not in playing state");
        return -1;
    }
    
    // Set state to paused
    engine->state = EIGHTKDVD_PLAYBACK_PAUSED;
    engine->pause_operations++;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Playback paused");
    }
    
    msg_Info(engine->obj, "8KDVD playback paused");
    return 0;
}

int kdvd_playback_engine_stop(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    msg_Info(engine->obj, "Stopping 8KDVD playback");
    
    // Set state to stopped
    engine->state = EIGHTKDVD_PLAYBACK_STOPPED;
    
    // Reset counters
    engine->frame_count = 0;
    engine->audio_sample_count = 0;
    engine->bytes_processed = 0;
    engine->current_framerate = 0;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Playback stopped");
    }
    
    msg_Info(engine->obj, "8KDVD playback stopped");
    return 0;
}

int kdvd_playback_engine_seek(kdvd_playback_engine_t *engine, uint64_t position_ms) {
    if (!engine) return -1;
    
    msg_Info(engine->obj, "Seeking to position: %llu ms", position_ms);
    
    uint64_t seek_start = vlc_tick_now();
    
    // Set state to seeking
    engine->state = EIGHTKDVD_PLAYBACK_SEEKING;
    engine->seek_operations++;
    
    // Simulate seek operation
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Seeking to position: %llu ms", position_ms);
        msg_Dbg(engine->obj, "Calculating seek target");
        msg_Dbg(engine->obj, "Updating playback position");
    }
    
    // Calculate seek time
    uint64_t seek_time = vlc_tick_now() - seek_start;
    engine->average_seek_time_ms = (engine->average_seek_time_ms + (float)seek_time / 1000.0f) / 2.0f;
    
    // Set state back to playing
    engine->state = EIGHTKDVD_PLAYBACK_PLAYING;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Seek completed in %llu us", seek_time);
    }
    
    msg_Info(engine->obj, "8KDVD playback seek completed");
    return 0;
}

kdvd_playback_state_t kdvd_playback_engine_get_state(kdvd_playback_engine_t *engine) {
    return engine ? engine->state : EIGHTKDVD_PLAYBACK_ERROR;
}

int kdvd_playback_engine_set_state(kdvd_playback_engine_t *engine, kdvd_playback_state_t state) {
    if (!engine) return -1;
    
    engine->state = state;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Playback state changed to: %d", state);
    }
    
    return 0;
}

bool kdvd_playback_engine_is_playing(kdvd_playback_engine_t *engine) {
    return engine && engine->state == EIGHTKDVD_PLAYBACK_PLAYING;
}

bool kdvd_playback_engine_is_paused(kdvd_playback_engine_t *engine) {
    return engine && engine->state == EIGHTKDVD_PLAYBACK_PAUSED;
}

int kdvd_playback_engine_set_config(kdvd_playback_engine_t *engine, const kdvd_playback_config_t *config) {
    if (!engine || !config) return -1;
    
    engine->config = *config;
    
    // Update engine settings based on config
    engine->hardware_acceleration_enabled = config->enable_hardware_acceleration;
    engine->hdr_processing_enabled = config->enable_hdr_processing;
    engine->spatial_audio_enabled = config->enable_spatial_audio;
    engine->target_framerate = config->target_framerate;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Playback configuration updated");
        msg_Dbg(engine->obj, "Hardware acceleration: %s", config->enable_hardware_acceleration ? "enabled" : "disabled");
        msg_Dbg(engine->obj, "HDR processing: %s", config->enable_hdr_processing ? "enabled" : "disabled");
        msg_Dbg(engine->obj, "Spatial audio: %s", config->enable_spatial_audio ? "enabled" : "disabled");
        msg_Dbg(engine->obj, "Target framerate: %u fps", config->target_framerate);
    }
    
    return 0;
}

kdvd_playback_config_t kdvd_playback_engine_get_config(kdvd_playback_engine_t *engine) {
    if (engine) {
        return engine->config;
    }
    
    kdvd_playback_config_t empty_config = {0};
    return empty_config;
}

int kdvd_playback_engine_reset_config(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    // Reset to default config
    engine->config.enable_hardware_acceleration = true;
    engine->config.enable_hdr_processing = true;
    engine->config.enable_spatial_audio = true;
    engine->config.enable_adaptive_bitrate = true;
    engine->config.buffer_size_mb = 1024;
    engine->config.max_bitrate_mbps = 100;
    engine->config.target_framerate = 60;
    engine->config.enable_vsync = true;
    engine->config.enable_adaptive_sync = true;
    engine->config.audio_channels = 8;
    engine->config.audio_sample_rate = 48000;
    engine->config.video_width = 7680;
    engine->config.video_height = 4320;
    engine->config.video_bit_depth = 10;
    engine->config.enable_dolby_vision = true;
    engine->config.enable_hdr10_plus = true;
    strcpy(engine->config.performance_mode, "balanced");
    
    msg_Info(engine->obj, "Playback configuration reset to defaults");
    return 0;
}

int kdvd_playback_engine_process_video_frame(kdvd_playback_engine_t *engine, const uint8_t *frame_data, size_t frame_size) {
    if (!engine || !frame_data) return -1;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Processing video frame: %zu bytes", frame_size);
    }
    
    // Simulate video frame processing
    engine->frame_count++;
    engine->bytes_processed += frame_size;
    
    // Update framerate
    uint64_t current_time = vlc_tick_now();
    if (current_time - engine->last_frame_time > 0) {
        engine->current_framerate = (uint32_t)(1000000.0f / (current_time - engine->last_frame_time));
    }
    engine->last_frame_time = current_time;
    
    // Update statistics
    engine->stats.frames_rendered++;
    engine->stats.bytes_processed += frame_size;
    
    return 0;
}

int kdvd_playback_engine_render_video_frame(kdvd_playback_engine_t *engine, uint8_t *output_buffer, size_t buffer_size) {
    if (!engine || !output_buffer) return -1;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Rendering video frame: %zu bytes", buffer_size);
    }
    
    // Simulate video frame rendering
    // In a real implementation, this would render the frame to the output buffer
    
    return 0;
}

int kdvd_playback_engine_set_video_resolution(kdvd_playback_engine_t *engine, uint32_t width, uint32_t height) {
    if (!engine) return -1;
    
    engine->config.video_width = width;
    engine->config.video_height = height;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Video resolution set to: %ux%u", width, height);
    }
    
    return 0;
}

int kdvd_playback_engine_set_video_bitrate(kdvd_playback_engine_t *engine, uint32_t bitrate_mbps) {
    if (!engine) return -1;
    
    engine->config.max_bitrate_mbps = bitrate_mbps;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Video bitrate set to: %u Mbps", bitrate_mbps);
    }
    
    return 0;
}

int kdvd_playback_engine_process_audio_frame(kdvd_playback_engine_t *engine, const uint8_t *audio_data, size_t audio_size) {
    if (!engine || !audio_data) return -1;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Processing audio frame: %zu bytes", audio_size);
    }
    
    // Simulate audio frame processing
    engine->audio_sample_count += audio_size / 4; // Assuming 32-bit samples
    engine->bytes_processed += audio_size;
    
    // Update statistics
    engine->stats.audio_samples_processed += audio_size / 4;
    engine->stats.bytes_processed += audio_size;
    
    return 0;
}

int kdvd_playback_engine_render_audio_frame(kdvd_playback_engine_t *engine, float *output_buffer, size_t buffer_size) {
    if (!engine || !output_buffer) return -1;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Rendering audio frame: %zu samples", buffer_size);
    }
    
    // Simulate audio frame rendering
    // In a real implementation, this would render the audio to the output buffer
    
    return 0;
}

int kdvd_playback_engine_set_audio_channels(kdvd_playback_engine_t *engine, uint32_t channels) {
    if (!engine) return -1;
    
    engine->config.audio_channels = channels;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Audio channels set to: %u", channels);
    }
    
    return 0;
}

int kdvd_playback_engine_set_audio_sample_rate(kdvd_playback_engine_t *engine, uint32_t sample_rate) {
    if (!engine) return -1;
    
    engine->config.audio_sample_rate = sample_rate;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Audio sample rate set to: %u Hz", sample_rate);
    }
    
    return 0;
}

int kdvd_playback_engine_set_hdr_mode(kdvd_playback_engine_t *engine, const char *hdr_mode) {
    if (!engine || !hdr_mode) return -1;
    
    if (strcmp(hdr_mode, "hdr10") == 0) {
        engine->config.enable_hdr_processing = true;
        engine->config.enable_dolby_vision = false;
        engine->config.enable_hdr10_plus = false;
    } else if (strcmp(hdr_mode, "dolby_vision") == 0) {
        engine->config.enable_hdr_processing = true;
        engine->config.enable_dolby_vision = true;
        engine->config.enable_hdr10_plus = false;
    } else if (strcmp(hdr_mode, "hdr10_plus") == 0) {
        engine->config.enable_hdr_processing = true;
        engine->config.enable_dolby_vision = false;
        engine->config.enable_hdr10_plus = true;
    } else {
        msg_Err(engine->obj, "Unknown HDR mode: %s", hdr_mode);
        return -1;
    }
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "HDR mode set to: %s", hdr_mode);
    }
    
    return 0;
}

int kdvd_playback_engine_set_dolby_vision(kdvd_playback_engine_t *engine, bool enable) {
    if (!engine) return -1;
    
    engine->config.enable_dolby_vision = enable;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Dolby Vision: %s", enable ? "enabled" : "disabled");
    }
    
    return 0;
}

int kdvd_playback_engine_set_hdr10_plus(kdvd_playback_engine_t *engine, bool enable) {
    if (!engine) return -1;
    
    engine->config.enable_hdr10_plus = enable;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "HDR10+: %s", enable ? "enabled" : "disabled");
    }
    
    return 0;
}

int kdvd_playback_engine_process_hdr_metadata(kdvd_playback_engine_t *engine, const uint8_t *metadata, size_t metadata_size) {
    if (!engine || !metadata) return -1;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Processing HDR metadata: %zu bytes", metadata_size);
    }
    
    // Simulate HDR metadata processing
    // In a real implementation, this would process the HDR metadata
    
    return 0;
}

int kdvd_playback_engine_enable_hardware_acceleration(kdvd_playback_engine_t *engine, bool enable) {
    if (!engine) return -1;
    
    engine->hardware_acceleration_enabled = enable;
    engine->config.enable_hardware_acceleration = enable;
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "Hardware acceleration: %s", enable ? "enabled" : "disabled");
    }
    
    return 0;
}

int kdvd_playback_engine_set_gpu_device(kdvd_playback_engine_t *engine, const char *device_name) {
    if (!engine || !device_name) return -1;
    
    strncpy(engine->gpu_device, device_name, sizeof(engine->gpu_device) - 1);
    engine->gpu_device[sizeof(engine->gpu_device) - 1] = '\0';
    
    if (engine->debug_enabled) {
        msg_Dbg(engine->obj, "GPU device set to: %s", device_name);
    }
    
    return 0;
}

int kdvd_playback_engine_get_gpu_info(kdvd_playback_engine_t *engine, char *info_buffer, size_t buffer_size) {
    if (!engine || !info_buffer) return -1;
    
    snprintf(info_buffer, buffer_size, "GPU Device: %s, Hardware Acceleration: %s", 
             engine->gpu_device, engine->hardware_acceleration_enabled ? "Enabled" : "Disabled");
    
    return 0;
}

kdvd_playback_stats_t kdvd_playback_engine_get_stats(kdvd_playback_engine_t *engine) {
    if (engine) {
        // Update real-time stats
        engine->stats.frames_rendered = engine->frame_count;
        engine->stats.audio_samples_processed = engine->audio_sample_count;
        engine->stats.bytes_processed = engine->bytes_processed;
        engine->stats.current_framerate = engine->current_framerate;
        engine->stats.buffer_usage_percent = engine->buffer_usage_percent;
        engine->stats.cpu_usage_percent = engine->cpu_usage_percent;
        engine->stats.gpu_usage_percent = engine->gpu_usage_percent;
        engine->stats.memory_usage_mb = engine->memory_usage_mb;
        engine->stats.seek_operations = engine->seek_operations;
        engine->stats.pause_operations = engine->pause_operations;
        engine->stats.error_count = engine->error_count;
        engine->stats.average_seek_time_ms = engine->average_seek_time_ms;
        engine->stats.average_pause_time_ms = engine->average_pause_time_ms;
        
        return engine->stats;
    }
    
    kdvd_playback_stats_t empty_stats = {0};
    return empty_stats;
}

int kdvd_playback_engine_reset_stats(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    memset(&engine->stats, 0, sizeof(kdvd_playback_stats_t));
    engine->frame_count = 0;
    engine->audio_sample_count = 0;
    engine->bytes_processed = 0;
    engine->seek_operations = 0;
    engine->pause_operations = 0;
    engine->error_count = 0;
    engine->start_time = vlc_tick_now();
    
    msg_Info(engine->obj, "8KDVD playback engine statistics reset");
    return 0;
}

int kdvd_playback_engine_set_performance_mode(kdvd_playback_engine_t *engine, const char *mode) {
    if (!engine || !mode) return -1;
    
    strncpy(engine->config.performance_mode, mode, sizeof(engine->config.performance_mode) - 1);
    engine->config.performance_mode[sizeof(engine->config.performance_mode) - 1] = '\0';
    
    if (strcmp(mode, "quality") == 0) {
        // Quality mode - prioritize quality over performance
        engine->config.enable_hardware_acceleration = true;
        engine->config.enable_hdr_processing = true;
        engine->config.buffer_size_mb = 2048; // 2GB buffer
        engine->config.max_bitrate_mbps = 200; // 200 Mbps
        msg_Info(engine->obj, "Performance mode set to: Quality (maximum quality)");
    } else if (strcmp(mode, "speed") == 0) {
        // Speed mode - prioritize performance over quality
        engine->config.enable_hardware_acceleration = true;
        engine->config.enable_hdr_processing = false;
        engine->config.buffer_size_mb = 512; // 512MB buffer
        engine->config.max_bitrate_mbps = 50; // 50 Mbps
        msg_Info(engine->obj, "Performance mode set to: Speed (maximum performance)");
    } else if (strcmp(mode, "balanced") == 0) {
        // Balanced mode - balance quality and performance
        engine->config.enable_hardware_acceleration = true;
        engine->config.enable_hdr_processing = true;
        engine->config.buffer_size_mb = 1024; // 1GB buffer
        engine->config.max_bitrate_mbps = 100; // 100 Mbps
        msg_Info(engine->obj, "Performance mode set to: Balanced (optimal performance)");
    } else {
        msg_Err(engine->obj, "Unknown performance mode: %s", mode);
        return -1;
    }
    
    return 0;
}

int kdvd_playback_engine_allocate_buffers(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    // Allocate engine context
    if (engine->engine_context) {
        free(engine->engine_context);
    }
    
    engine->engine_context = malloc(engine->config.buffer_size_mb * 1024 * 1024);
    if (!engine->engine_context) {
        msg_Err(engine->obj, "Failed to allocate playback buffers");
        return -1;
    }
    
    msg_Info(engine->obj, "Playback buffers allocated: %u MB", engine->config.buffer_size_mb);
    return 0;
}

int kdvd_playback_engine_free_buffers(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    if (engine->engine_context) {
        free(engine->engine_context);
        engine->engine_context = NULL;
    }
    
    msg_Info(engine->obj, "Playback buffers freed");
    return 0;
}

int kdvd_playback_engine_get_buffer_usage(kdvd_playback_engine_t *engine, uint32_t *usage_percent) {
    if (!engine || !usage_percent) return -1;
    
    *usage_percent = engine->buffer_usage_percent;
    return 0;
}

int kdvd_playback_engine_get_last_error(kdvd_playback_engine_t *engine, char *error_buffer, size_t buffer_size) {
    if (!engine || !error_buffer) return -1;
    
    strncpy(error_buffer, engine->last_error, buffer_size - 1);
    error_buffer[buffer_size - 1] = '\0';
    return 0;
}

int kdvd_playback_engine_clear_errors(kdvd_playback_engine_t *engine) {
    if (!engine) return -1;
    
    memset(engine->last_error, 0, sizeof(engine->last_error));
    return 0;
}

void kdvd_playback_engine_set_debug(kdvd_playback_engine_t *engine, bool enable) {
    if (engine) {
        engine->debug_enabled = enable;
        msg_Info(engine->obj, "8KDVD playback engine debug %s", enable ? "enabled" : "disabled");
    }
}

void kdvd_playback_engine_log_info(kdvd_playback_engine_t *engine) {
    if (!engine) return;
    
    msg_Info(engine->obj, "8KDVD Playback Engine Info:");
    msg_Info(engine->obj, "  State: %d", engine->state);
    msg_Info(engine->obj, "  Hardware Acceleration: %s", engine->hardware_acceleration_enabled ? "enabled" : "disabled");
    msg_Info(engine->obj, "  HDR Processing: %s", engine->hdr_processing_enabled ? "enabled" : "disabled");
    msg_Info(engine->obj, "  Spatial Audio: %s", engine->spatial_audio_enabled ? "enabled" : "disabled");
    msg_Info(engine->obj, "  Target Framerate: %u fps", engine->target_framerate);
    msg_Info(engine->obj, "  Current Framerate: %u fps", engine->current_framerate);
    msg_Info(engine->obj, "  GPU Device: %s", engine->gpu_device);
    msg_Info(engine->obj, "  Performance Mode: %s", engine->config.performance_mode);
}

void kdvd_playback_engine_log_stats(kdvd_playback_engine_t *engine) {
    if (!engine) return;
    
    msg_Info(engine->obj, "8KDVD Playback Engine Statistics:");
    msg_Info(engine->obj, "  Frames Rendered: %llu", engine->stats.frames_rendered);
    msg_Info(engine->obj, "  Frames Dropped: %llu", engine->stats.frames_dropped);
    msg_Info(engine->obj, "  Audio Samples Processed: %llu", engine->stats.audio_samples_processed);
    msg_Info(engine->obj, "  Bytes Processed: %llu", engine->stats.bytes_processed);
    msg_Info(engine->obj, "  Playback Time: %llu ms", engine->stats.playback_time_ms);
    msg_Info(engine->obj, "  Average Framerate: %.2f fps", engine->stats.average_framerate);
    msg_Info(engine->obj, "  Current Framerate: %.2f fps", engine->stats.current_framerate);
    msg_Info(engine->obj, "  Average Bitrate: %.2f Mbps", engine->stats.average_bitrate_mbps);
    msg_Info(engine->obj, "  Current Bitrate: %.2f Mbps", engine->stats.current_bitrate_mbps);
    msg_Info(engine->obj, "  Buffer Usage: %u%%", engine->stats.buffer_usage_percent);
    msg_Info(engine->obj, "  CPU Usage: %u%%", engine->stats.cpu_usage_percent);
    msg_Info(engine->obj, "  GPU Usage: %u%%", engine->stats.gpu_usage_percent);
    msg_Info(engine->obj, "  Memory Usage: %u MB", engine->stats.memory_usage_mb);
    msg_Info(engine->obj, "  Seek Operations: %llu", engine->stats.seek_operations);
    msg_Info(engine->obj, "  Pause Operations: %llu", engine->stats.pause_operations);
    msg_Info(engine->obj, "  Error Count: %llu", engine->stats.error_count);
    msg_Info(engine->obj, "  Average Seek Time: %.2f ms", engine->stats.average_seek_time_ms);
    msg_Info(engine->obj, "  Average Pause Time: %.2f ms", engine->stats.average_pause_time_ms);
}
