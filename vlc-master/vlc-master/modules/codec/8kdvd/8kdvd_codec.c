#include <vlc_common.h>
#include <vlc_codec.h>
#include <vlc_fourcc.h>
#include <vlc_picture.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include "vp9_8k_decoder.h"
#include "opus_8k_decoder.h"

// 8KDVD Codec Module for VLC
typedef struct decoder_sys_t {
    vp9_8k_decoder_t *vp9_decoder;
    opus_8k_decoder_t *opus_decoder;
    bool video_initialized;
    bool audio_initialized;
    bool debug_enabled;
} decoder_sys_t;

// Module descriptor
vlc_module_begin()
    set_shortname("8KDVD Codec")
    set_description("8KDVD VP9/Opus Codec Decoder")
    set_capability("decoder", 10)
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_CODEC)
    set_callbacks(Open, Close)
    add_shortcut("8kdvd", "vp9_8k", "opus_8k")
vlc_module_end()

// Forward declarations
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);
static int Decode(decoder_t *, block_t *);
static int Flush(decoder_t *);

// Module functions
static int Open(vlc_object_t *obj) {
    decoder_t *decoder = (decoder_t *)obj;
    decoder_sys_t *sys = calloc(1, sizeof(decoder_sys_t));
    if (!sys) {
        msg_Err(decoder, "Failed to allocate decoder system");
        return VLC_EGENERIC;
    }
    
    msg_Info(decoder, "8KDVD codec module opening");
    
    // Check if this is a VP9 or Opus stream
    if (decoder->fmt_in.i_codec == VLC_CODEC_VP9) {
        // VP9 8K video decoder
        sys->vp9_decoder = vp9_8k_decoder_create(decoder);
        if (!sys->vp9_decoder) {
            msg_Err(decoder, "Failed to create VP9 8K decoder");
            free(sys);
            return VLC_EGENERIC;
        }
        
        // Configure VP9 decoder for 8K
        vp9_8k_config_t vp9_config = {
            .width = 7680,
            .height = 4320,
            .bit_depth = 10,
            .frame_rate = 60,
            .hdr_enabled = true,
            .dolby_vision_enabled = false,
            .hardware_acceleration = true,
            .profile = 2,
            .level = 6,
            .color_space = 1,
            .color_range = 1,
            .chroma_subsampling = 1
        };
        
        if (vp9_8k_decoder_configure(sys->vp9_decoder, &vp9_config) != 0) {
            msg_Err(decoder, "Failed to configure VP9 8K decoder");
            vp9_8k_decoder_destroy(sys->vp9_decoder);
            free(sys);
            return VLC_EGENERIC;
        }
        
        // Enable 8K mode
        vp9_8k_decoder_enable_8k_mode(sys->vp9_decoder, true);
        vp9_8k_decoder_optimize_for_8k(sys->vp9_decoder);
        
        // Set up video output format
        decoder->fmt_out.video.i_width = 7680;
        decoder->fmt_out.video.i_height = 4320;
        decoder->fmt_out.video.i_bits_per_pixel = 10;
        decoder->fmt_out.video.i_frame_rate = 60;
        decoder->fmt_out.video.i_frame_rate_base = 1;
        decoder->fmt_out.video.i_sar_num = 1;
        decoder->fmt_out.video.i_sar_den = 1;
        decoder->fmt_out.video.i_chroma = VLC_CODEC_I420_10L;
        
        sys->video_initialized = true;
        sys->audio_initialized = false;
        
        msg_Info(decoder, "VP9 8K decoder initialized");
        
    } else if (decoder->fmt_in.i_codec == VLC_CODEC_OPUS) {
        // Opus 8K audio decoder
        sys->opus_decoder = opus_8k_decoder_create(decoder);
        if (!sys->opus_decoder) {
            msg_Err(decoder, "Failed to create Opus 8K decoder");
            free(sys);
            return VLC_EGENERIC;
        }
        
        // Configure Opus decoder for 8K spatial audio
        opus_8k_config_t opus_config = {
            .channels = 8,
            .sample_rate = 48000,
            .bitrate = 510000,
            .frame_size = 960,
            .application = 0,
            .spatial_audio = true,
            .ambisonics = false,
            .binaural = false,
            .ambisonics_order = 1,
            .ambisonics_channels = 4,
            .spatial_resolution = 1.0f,
            .binaural_quality = 0.8f
        };
        
        if (opus_8k_decoder_configure(sys->opus_decoder, &opus_config) != 0) {
            msg_Err(decoder, "Failed to configure Opus 8K decoder");
            opus_8k_decoder_destroy(sys->opus_decoder);
            free(sys);
            return VLC_EGENERIC;
        }
        
        // Enable 8K spatial audio
        opus_8k_decoder_enable_8k_spatial(sys->opus_decoder, true);
        opus_8k_decoder_optimize_for_8k(sys->opus_decoder);
        
        // Set up audio output format
        decoder->fmt_out.audio.i_channels = 8;
        decoder->fmt_out.audio.i_rate = 48000;
        decoder->fmt_out.audio.i_bitspersample = 16;
        decoder->fmt_out.audio.i_physical_channels = AOUT_CHANS_8_0;
        
        sys->audio_initialized = true;
        sys->video_initialized = false;
        
        msg_Info(decoder, "Opus 8K decoder initialized");
        
    } else {
        msg_Err(decoder, "Unsupported codec: %s", vlc_fourcc_GetDescription(decoder->fmt_in.i_codec));
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Set up decoder
    decoder->p_sys = sys;
    decoder->pf_decode_video = (decoder->fmt_in.i_codec == VLC_CODEC_VP9) ? Decode : NULL;
    decoder->pf_decode_audio = (decoder->fmt_in.i_codec == VLC_CODEC_OPUS) ? Decode : NULL;
    decoder->pf_flush = Flush;
    
    sys->debug_enabled = false;
    
    msg_Info(decoder, "8KDVD codec module opened successfully");
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj) {
    decoder_t *decoder = (decoder_t *)obj;
    decoder_sys_t *sys = decoder->p_sys;
    
    if (!sys) return;
    
    msg_Info(decoder, "8KDVD codec module closing");
    
    if (sys->vp9_decoder) {
        vp9_8k_decoder_destroy(sys->vp9_decoder);
    }
    
    if (sys->opus_decoder) {
        opus_8k_decoder_destroy(sys->opus_decoder);
    }
    
    free(sys);
    decoder->p_sys = NULL;
    
    msg_Info(decoder, "8KDVD codec module closed");
}

static int Decode(decoder_t *decoder, block_t *block) {
    decoder_sys_t *sys = decoder->p_sys;
    if (!sys || !block) return 0;
    
    if (block->i_flags & BLOCK_FLAG_DISCONTINUITY) {
        if (sys->vp9_decoder) {
            vp9_8k_decoder_flush(sys->vp9_decoder);
        }
        if (sys->opus_decoder) {
            opus_8k_decoder_flush(sys->opus_decoder);
        }
        return 0;
    }
    
    if (sys->vp9_decoder && sys->video_initialized) {
        // Decode VP9 8K video
        picture_t *picture = NULL;
        
        if (vp9_8k_decoder_decode_frame(sys->vp9_decoder, block, &picture) == 0 && picture) {
            picture->date = block->i_pts;
            picture->b_progressive = true;
            picture->b_top_field_first = true;
            
            // Set HDR metadata if enabled
            if (sys->vp9_decoder->hdr_enabled) {
                picture->p_sys = malloc(sizeof(vlc_meta_t));
                if (picture->p_sys) {
                    vlc_meta_t *meta = (vlc_meta_t*)picture->p_sys;
                    vlc_meta_Set(meta, vlc_meta_Title, "8K HDR Video");
                }
            }
            
            decoder->pf_video_buffer(decoder, picture);
            
            if (sys->debug_enabled) {
                vp9_8k_stats_t stats = vp9_8k_decoder_get_stats(sys->vp9_decoder);
                msg_Dbg(decoder, "VP9 8K frame decoded: %llu frames, %.2f FPS", 
                       stats.frames_decoded, stats.average_fps);
            }
        }
        
    } else if (sys->opus_decoder && sys->audio_initialized) {
        // Decode Opus 8K audio
        block_t *output_block = NULL;
        
        if (opus_8k_decoder_decode_frame(sys->opus_decoder, block, &output_block) == 0 && output_block) {
            output_block->i_dts = block->i_dts;
            output_block->i_pts = block->i_pts;
            output_block->i_length = block->i_length;
            
            decoder->pf_audio_buffer(decoder, output_block);
            
            if (sys->debug_enabled) {
                opus_8k_stats_t stats = opus_8k_decoder_get_stats(sys->opus_decoder);
                msg_Dbg(decoder, "Opus 8K frame decoded: %llu frames, %.2f FPS", 
                       stats.frames_decoded, stats.average_fps);
            }
        }
    }
    
    return 0;
}

static int Flush(decoder_t *decoder) {
    decoder_sys_t *sys = decoder->p_sys;
    if (!sys) return 0;
    
    msg_Info(decoder, "Flushing 8KDVD codec");
    
    if (sys->vp9_decoder) {
        vp9_8k_decoder_flush(sys->vp9_decoder);
    }
    
    if (sys->opus_decoder) {
        opus_8k_decoder_flush(sys->opus_decoder);
    }
    
    return 0;
}
