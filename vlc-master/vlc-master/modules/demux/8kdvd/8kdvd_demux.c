#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_stream.h>
#include <vlc_meta.h>
#include <vlc_fourcc.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include "8kdvd_container_parser.h"

// 8KDVD Demux Module for VLC
typedef struct demux_sys_t {
    kdvd_container_parser_t *parser;
    es_out_id_t *video_es;
    es_out_id_t *audio_es;
    bool header_sent;
    bool eof;
} demux_sys_t;

// Module descriptor
vlc_module_begin()
    set_shortname("8KDVD")
    set_description("8KDVD Container Demuxer")
    set_capability("demux", 10)
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_DEMUX)
    set_callbacks(Open, Close)
    add_shortcut("8kdvd", "evo8")
vlc_module_end()

// Forward declarations
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);
static int Demux(demux_t *);
static int Control(demux_t *, int, va_list);

// Module functions
static int Open(vlc_object_t *obj) {
    demux_t *demux = (demux_t *)obj;
    stream_t *stream = demux->s;
    
    msg_Info(demux, "8KDVD demux module opening");
    
    // Check if this is an 8KDVD file
    if (!stream_IsExtension(stream, "evo8") && 
        !stream_IsExtension(stream, "8kdvd")) {
        return VLC_EGENERIC;
    }
    
    // Allocate demux system
    demux_sys_t *sys = calloc(1, sizeof(demux_sys_t));
    if (!sys) {
        msg_Err(demux, "Failed to allocate demux system");
        return VLC_EGENERIC;
    }
    
    // Create container parser
    sys->parser = kdvd_container_parser_create(demux);
    if (!sys->parser) {
        msg_Err(demux, "Failed to create 8KDVD container parser");
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Detect container format
    if (kdvd_container_parser_detect(sys->parser, stream) != 0) {
        msg_Err(demux, "Not a valid 8KDVD container");
        kdvd_container_parser_destroy(sys->parser);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Parse container header
    if (kdvd_container_parser_parse_header(sys->parser, stream) != 0) {
        msg_Err(demux, "Failed to parse 8KDVD container header");
        kdvd_container_parser_destroy(sys->parser);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Validate 8KDVD container
    if (kdvd_container_parser_validate_8kdvd(sys->parser, stream) != 0) {
        msg_Err(demux, "8KDVD container validation failed");
        kdvd_container_parser_destroy(sys->parser);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Parse frame index
    if (kdvd_container_parser_parse_frames(sys->parser, stream) != 0) {
        msg_Err(demux, "Failed to parse 8KDVD frame index");
        kdvd_container_parser_destroy(sys->parser);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Create elementary streams
    if (kdvd_container_parser_create_es(sys->parser, demux) != 0) {
        msg_Err(demux, "Failed to create 8KDVD elementary streams");
        kdvd_container_parser_destroy(sys->parser);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Set up demux
    demux->p_sys = sys;
    demux->pf_demux = Demux;
    demux->pf_control = Control;
    demux->info.i_update = UPDATE_META;
    demux->info.i_title = 0;
    demux->info.i_seekpoint = 0;
    
    // Set up metadata
    kdvd_container_info_t info = kdvd_container_parser_get_info(sys->parser);
    if (info.width > 0 && info.height > 0) {
        demux->info.i_width = info.width;
        demux->info.i_height = info.height;
    }
    if (info.frame_rate > 0) {
        demux->info.i_fps = (float)info.frame_rate;
    }
    
    // Set up input item metadata
    if (demux->p_input_item) {
        vlc_meta_t *meta = vlc_meta_New();
        if (meta) {
            vlc_meta_Set(meta, vlc_meta_Title, "8KDVD Content");
            vlc_meta_Set(meta, vlc_meta_Description, "8K Ultra High Definition Video");
            vlc_meta_Set(meta, vlc_meta_Genre, "8KDVD");
            input_item_AddMeta(demux->p_input_item, meta);
            vlc_meta_Delete(meta);
        }
    }
    
    sys->header_sent = false;
    sys->eof = false;
    
    msg_Info(demux, "8KDVD demux module opened successfully");
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj) {
    demux_t *demux = (demux_t *)obj;
    demux_sys_t *sys = demux->p_sys;
    
    if (!sys) return;
    
    msg_Info(demux, "8KDVD demux module closing");
    
    if (sys->parser) {
        kdvd_container_parser_destroy(sys->parser);
    }
    
    free(sys);
    demux->p_sys = NULL;
    
    msg_Info(demux, "8KDVD demux module closed");
}

static int Demux(demux_t *demux) {
    demux_sys_t *sys = demux->p_sys;
    if (!sys || !sys->parser) return 0;
    
    if (sys->eof) return 0;
    
    // Send header if not sent yet
    if (!sys->header_sent) {
        // Create and send video ES
        es_format_t video_fmt;
        es_format_Init(&video_fmt, VIDEO_ES, VLC_CODEC_VP9);
        
        kdvd_container_info_t info = kdvd_container_parser_get_info(sys->parser);
        video_fmt.video.i_width = info.width;
        video_fmt.video.i_height = info.height;
        video_fmt.video.i_frame_rate = info.frame_rate;
        video_fmt.video.i_frame_rate_base = 1;
        video_fmt.video.i_bits_per_pixel = info.bit_depth;
        video_fmt.video.i_sar_num = 1;
        video_fmt.video.i_sar_den = 1;
        
        if (info.hdr_enabled) {
            video_fmt.video.i_chroma = VLC_CODEC_VP9_HDR;
        }
        
        sys->video_es = es_out_Add(demux->out, &video_fmt);
        es_format_Clean(&video_fmt);
        
        if (!sys->video_es) {
            msg_Err(demux, "Failed to create video ES");
            return 0;
        }
        
        // Create and send audio ES
        es_format_t audio_fmt;
        es_format_Init(&audio_fmt, AUDIO_ES, VLC_CODEC_OPUS);
        audio_fmt.audio.i_channels = info.audio_channels;
        audio_fmt.audio.i_rate = info.audio_sample_rate;
        audio_fmt.audio.i_bitspersample = 16;
        audio_fmt.audio.i_physical_channels = AOUT_CHANS_8_0;
        
        sys->audio_es = es_out_Add(demux->out, &audio_fmt);
        es_format_Clean(&audio_fmt);
        
        if (!sys->audio_es) {
            msg_Err(demux, "Failed to create audio ES");
            return 0;
        }
        
        sys->header_sent = true;
        msg_Info(demux, "8KDVD elementary streams created");
    }
    
    // Read and send frame data
    kdvd_frame_info_t frame = kdvd_container_parser_get_frame(sys->parser, sys->parser->current_frame);
    if (frame.size == 0) {
        sys->eof = true;
        return 0;
    }
    
    // Allocate frame buffer
    uint8_t *frame_buffer = malloc(frame.size);
    if (!frame_buffer) {
        msg_Err(demux, "Failed to allocate frame buffer");
        return 0;
    }
    
    // Read frame data
    int bytes_read = kdvd_container_parser_read_frame(sys->parser, demux->s, frame_buffer, frame.size);
    if (bytes_read <= 0) {
        free(frame_buffer);
        sys->eof = true;
        return 0;
    }
    
    // Create block
    block_t *block = block_Alloc(bytes_read);
    if (!block) {
        free(frame_buffer);
        return 0;
    }
    
    memcpy(block->p_buffer, frame_buffer, bytes_read);
    block->i_dts = frame.timestamp;
    block->i_pts = frame.timestamp;
    block->i_length = 1000000 / info.frame_rate; // Frame duration in microseconds
    
    if (frame.keyframe) {
        block->i_flags |= BLOCK_FLAG_TYPE_I;
    }
    
    // Send block to appropriate ES
    if (sys->video_es) {
        es_out_Send(demux->out, sys->video_es, block);
    } else {
        block_Release(block);
    }
    
    free(frame_buffer);
    
    // Move to next frame
    kdvd_container_parser_get_next_frame(sys->parser, demux->s);
    
    return 1;
}

static int Control(demux_t *demux, int query, va_list args) {
    demux_sys_t *sys = demux->p_sys;
    if (!sys || !sys->parser) return VLC_EGENERIC;
    
    switch (query) {
        case DEMUX_GET_POSITION: {
            float *pos = va_arg(args, float *);
            if (sys->parser->frame_count > 0) {
                *pos = (float)sys->parser->current_frame / sys->parser->frame_count;
            } else {
                *pos = 0.0f;
            }
            return VLC_SUCCESS;
        }
        
        case DEMUX_SET_POSITION: {
            float pos = va_arg(args, double);
            if (pos < 0.0f) pos = 0.0f;
            if (pos > 1.0f) pos = 1.0f;
            
            uint32_t frame_index = (uint32_t)(pos * sys->parser->frame_count);
            if (kdvd_container_parser_seek_to_frame(sys->parser, demux->s, frame_index) == 0) {
                return VLC_SUCCESS;
            }
            return VLC_EGENERIC;
        }
        
        case DEMUX_GET_LENGTH: {
            int64_t *length = va_arg(args, int64_t *);
            kdvd_container_info_t info = kdvd_container_parser_get_info(sys->parser);
            if (info.frame_rate > 0) {
                *length = (int64_t)sys->parser->frame_count * 1000000 / info.frame_rate;
            } else {
                *length = 0;
            }
            return VLC_SUCCESS;
        }
        
        case DEMUX_GET_TIME: {
            int64_t *time = va_arg(args, int64_t *);
            if (sys->parser->current_frame < sys->parser->frame_count) {
                kdvd_frame_info_t frame = kdvd_container_parser_get_frame(sys->parser, sys->parser->current_frame);
                *time = frame.timestamp;
            } else {
                *time = 0;
            }
            return VLC_SUCCESS;
        }
        
        case DEMUX_CAN_SEEK: {
            bool *can_seek = va_arg(args, bool *);
            *can_seek = true;
            return VLC_SUCCESS;
        }
        
        case DEMUX_CAN_PAUSE: {
            bool *can_pause = va_arg(args, bool *);
            *can_pause = true;
            return VLC_SUCCESS;
        }
        
        case DEMUX_CAN_CONTROL_PACE: {
            bool *can_control = va_arg(args, bool *);
            *can_control = true;
            return VLC_SUCCESS;
        }
        
        default:
            return VLC_EGENERIC;
    }
}
