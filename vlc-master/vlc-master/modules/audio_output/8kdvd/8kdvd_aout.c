#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include "8k_audio_processor.h"

// 8KDVD Audio Output Module for VLC
typedef struct aout_sys_t {
    kdvd_8k_audio_processor_t *processor;
    bool initialized;
    bool debug_enabled;
} aout_sys_t;

// Module descriptor
vlc_module_begin()
    set_shortname("8KDVD Aout")
    set_description("8KDVD 8K Spatial Audio Output")
    set_capability("aout", 10)
    set_category(CAT_AUDIO)
    set_subcategory(SUBCAT_AUDIO_AOUT)
    set_callbacks(Open, Close)
    add_shortcut("8kdvd", "8k_aout")
vlc_module_end()

// Forward declarations
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);
static int Play(aout_stream_t *, block_t *);
static int Pause(aout_stream_t *, bool);
static int Flush(aout_stream_t *);

// Module functions
static int Open(vlc_object_t *obj) {
    aout_stream_t *aout = (aout_stream_t *)obj;
    aout_sys_t *sys = calloc(1, sizeof(aout_sys_t));
    if (!sys) {
        msg_Err(aout, "Failed to allocate audio output system");
        return VLC_EGENERIC;
    }
    
    msg_Info(aout, "8KDVD audio output module opening");
    
    // Check if this is an 8K spatial audio output
    if (aout->fmt.audio.i_channels != 8) {
        msg_Err(aout, "Not an 8K spatial audio output: %u channels (expected 8)", 
               aout->fmt.audio.i_channels);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Create 8K audio processor
    sys->processor = kdvd_8k_audio_processor_create(aout);
    if (!sys->processor) {
        msg_Err(aout, "Failed to create 8K audio processor");
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Configure 8K audio processor
    kdvd_8k_audio_config_t audio_config = {
        .channels = aout->fmt.audio.i_channels,
        .sample_rate = aout->fmt.audio.i_rate,
        .bitrate = 510000,  // Max bitrate for 8-channel
        .frame_size = 960,  // 20ms frames
        .spatial_audio = true,
        .ambisonics = false,
        .binaural = false,
        .ambisonics_order = 1,
        .ambisonics_channels = 4,
        .spatial_resolution = 1.0f,
        .binaural_quality = 0.8f,
        .hrtf_enabled = true,
        .room_simulation = false,
        .room_size = 1.0f,
        .room_damping = 0.5f,
        .doppler_effect = false,
        .distance_attenuation = false,
        .occlusion_simulation = false
    };
    
    if (kdvd_8k_audio_processor_configure(sys->processor, &audio_config) != 0) {
        msg_Err(aout, "Failed to configure 8K audio processor");
        kdvd_8k_audio_processor_destroy(sys->processor);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Enable 8K spatial audio
    kdvd_8k_audio_processor_enable_8k_spatial(sys->processor, true);
    kdvd_8k_audio_processor_optimize_for_8k(sys->processor);
    
    // Set up audio output
    aout->p_sys = sys;
    aout->pf_play = Play;
    aout->pf_pause = Pause;
    aout->pf_flush = Flush;
    
    // Set up audio format
    aout->fmt.audio.i_channels = 8;
    aout->fmt.audio.i_rate = 48000;
    aout->fmt.audio.i_bitspersample = 16;
    aout->fmt.audio.i_physical_channels = AOUT_CHANS_8_0;
    
    sys->initialized = true;
    sys->debug_enabled = false;
    
    msg_Info(aout, "8KDVD audio output module opened successfully");
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj) {
    aout_stream_t *aout = (aout_stream_t *)obj;
    aout_sys_t *sys = aout->p_sys;
    
    if (!sys) return;
    
    msg_Info(aout, "8KDVD audio output module closing");
    
    if (sys->processor) {
        kdvd_8k_audio_processor_destroy(sys->processor);
    }
    
    free(sys);
    aout->p_sys = NULL;
    
    msg_Info(aout, "8KDVD audio output module closed");
}

static int Play(aout_stream_t *aout, block_t *block) {
    aout_sys_t *sys = aout->p_sys;
    if (!sys || !block) return 0;
    
    if (!sys->initialized) {
        msg_Err(aout, "8KDVD audio output not initialized");
        return -1;
    }
    
    // Process 8K spatial audio frame
    block_t *output_block = NULL;
    
    if (kdvd_8k_audio_processor_process_frame(sys->processor, block, &output_block) != 0) {
        msg_Err(aout, "Failed to process 8K audio frame");
        return -1;
    }
    
    if (output_block) {
        // Send processed audio to output
        aout_Play(aout, output_block);
        
        if (sys->debug_enabled) {
            kdvd_8k_audio_stats_t stats = kdvd_8k_audio_processor_get_stats(sys->processor);
            msg_Dbg(aout, "8K audio frame played: %llu frames, %.2f FPS", 
                   stats.frames_processed, stats.average_fps);
        }
    }
    
    return 0;
}

static int Pause(aout_stream_t *aout, bool pause) {
    aout_sys_t *sys = aout->p_sys;
    if (!sys) return 0;
    
    msg_Info(aout, "8KDVD audio output %s", pause ? "paused" : "resumed");
    
    if (pause) {
        // Pause audio processing
        if (sys->processor) {
            kdvd_8k_audio_processor_flush(sys->processor);
        }
    } else {
        // Resume audio processing
        if (sys->processor) {
            kdvd_8k_audio_processor_reset(sys->processor);
        }
    }
    
    return 0;
}

static int Flush(aout_stream_t *aout) {
    aout_sys_t *sys = aout->p_sys;
    if (!sys) return 0;
    
    msg_Info(aout, "Flushing 8KDVD audio output");
    
    if (sys->processor) {
        kdvd_8k_audio_processor_flush(sys->processor);
    }
    
    return 0;
}
