#include "8kdvd_container_parser.h"
#include <vlc_messages.h>
#include <vlc_fs.h>
#include <vlc_meta.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <vlc_fourcc.h>
#include <string.h>
#include <stdlib.h>

// 8KDVD Container Parser Implementation
struct kdvd_container_parser_t {
    vlc_object_t *obj;
    kdvd_container_info_t info;
    kdvd_frame_info_t *frames;
    uint32_t frame_count;
    uint32_t current_frame;
    bool debug_enabled;
    bool header_parsed;
    bool frames_parsed;
};

// 8KDVD Magic Numbers
#define KDVD_MAGIC_8KDVD 0x384B4456  // "8KDV"
#define KDVD_MAGIC_EVO8  0x45564F38  // "EVO8"
#define KDVD_VERSION_1   0x00010000  // Version 1.0

// 8KDVD Container Parser Functions
kdvd_container_parser_t* kdvd_container_parser_create(vlc_object_t *obj) {
    kdvd_container_parser_t *parser = calloc(1, sizeof(kdvd_container_parser_t));
    if (!parser) return NULL;
    
    parser->obj = obj;
    parser->frame_count = 0;
    parser->current_frame = 0;
    parser->debug_enabled = false;
    parser->header_parsed = false;
    parser->frames_parsed = false;
    parser->frames = NULL;
    
    // Initialize container info
    memset(&parser->info, 0, sizeof(kdvd_container_info_t));
    
    msg_Info(obj, "8KDVD container parser created");
    return parser;
}

void kdvd_container_parser_destroy(kdvd_container_parser_t *parser) {
    if (!parser) return;
    
    if (parser->frames) {
        free(parser->frames);
    }
    
    free(parser);
    msg_Info(parser->obj, "8KDVD container parser destroyed");
}

int kdvd_container_parser_detect(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    msg_Info(parser->obj, "Detecting 8KDVD container format");
    
    // Read magic number
    uint32_t magic;
    if (vlc_stream_Read(stream, &magic, sizeof(magic)) != sizeof(magic)) {
        msg_Err(parser->obj, "Failed to read magic number");
        return -1;
    }
    
    // Check for 8KDVD magic numbers
    if (magic == KDVD_MAGIC_8KDVD || magic == KDVD_MAGIC_EVO8) {
        parser->info.magic = magic;
        msg_Info(parser->obj, "8KDVD container detected (magic: 0x%08X)", magic);
        return 0;
    }
    
    msg_Err(parser->obj, "Not a valid 8KDVD container (magic: 0x%08X)", magic);
    return -1;
}

int kdvd_container_parser_parse_header(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    msg_Info(parser->obj, "Parsing 8KDVD container header");
    
    // Parse EVO8 header
    if (kdvd_container_parser_parse_evo8_header(parser, stream) != 0) {
        return -1;
    }
    
    // Extract metadata
    if (kdvd_container_parser_extract_metadata(parser, stream) != 0) {
        return -1;
    }
    
    parser->header_parsed = true;
    
    if (parser->debug_enabled) {
        kdvd_container_parser_log_info(parser);
    }
    
    msg_Info(parser->obj, "8KDVD container header parsed successfully");
    return 0;
}

int kdvd_container_parser_parse_frames(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    msg_Info(parser->obj, "Parsing 8KDVD container frames");
    
    // Allocate frame array
    parser->frames = calloc(parser->info.frame_count, sizeof(kdvd_frame_info_t));
    if (!parser->frames) {
        msg_Err(parser->obj, "Failed to allocate frame array");
        return -1;
    }
    
    // Parse frame index
    uint64_t frame_offset = parser->info.header_size;
    for (uint32_t i = 0; i < parser->info.frame_count; i++) {
        kdvd_frame_info_t *frame = &parser->frames[i];
        
        // Read frame header
        uint32_t frame_header[4];
        if (vlc_stream_Read(stream, frame_header, sizeof(frame_header)) != sizeof(frame_header)) {
            msg_Err(parser->obj, "Failed to read frame %u header", i);
            return -1;
        }
        
        frame->frame_number = i;
        frame->offset = frame_offset;
        frame->size = frame_header[0];
        frame->timestamp = (uint64_t)i * 1000000 / parser->info.frame_rate; // Microseconds
        frame->keyframe = (frame_header[1] & 0x01) != 0;
        frame->frame_type = (frame_header[1] >> 1) & 0x03;
        frame->quality = frame_header[2];
        
        frame_offset += frame->size;
        
        if (parser->debug_enabled) {
            msg_Dbg(parser->obj, "Frame %u: offset=%llu size=%llu timestamp=%llu keyframe=%s",
                   i, frame->offset, frame->size, frame->timestamp,
                   frame->keyframe ? "yes" : "no");
        }
    }
    
    parser->frames_parsed = true;
    parser->frame_count = parser->info.frame_count;
    
    msg_Info(parser->obj, "8KDVD container frames parsed successfully (%u frames)", parser->frame_count);
    return 0;
}

kdvd_container_info_t kdvd_container_parser_get_info(kdvd_container_parser_t *parser) {
    if (parser) {
        return parser->info;
    }
    
    kdvd_container_info_t empty_info = {0};
    return empty_info;
}

int kdvd_container_parser_get_frame_count(kdvd_container_parser_t *parser) {
    return parser ? parser->frame_count : 0;
}

kdvd_frame_info_t kdvd_container_parser_get_frame(kdvd_container_parser_t *parser, uint32_t frame_index) {
    if (parser && parser->frames && frame_index < parser->frame_count) {
        return parser->frames[frame_index];
    }
    
    kdvd_frame_info_t empty_frame = {0};
    return empty_frame;
}

int kdvd_container_parser_seek_to_frame(kdvd_container_parser_t *parser, stream_t *stream, uint32_t frame_index) {
    if (!parser || !stream) return -1;
    
    if (frame_index >= parser->frame_count) {
        msg_Err(parser->obj, "Frame index out of range: %u >= %u", frame_index, parser->frame_count);
        return -1;
    }
    
    kdvd_frame_info_t *frame = &parser->frames[frame_index];
    
    if (vlc_stream_Seek(stream, frame->offset) != VLC_SUCCESS) {
        msg_Err(parser->obj, "Failed to seek to frame %u at offset %llu", frame_index, frame->offset);
        return -1;
    }
    
    parser->current_frame = frame_index;
    
    if (parser->debug_enabled) {
        msg_Dbg(parser->obj, "Seeked to frame %u at offset %llu", frame_index, frame->offset);
    }
    
    return 0;
}

int kdvd_container_parser_read_frame(kdvd_container_parser_t *parser, stream_t *stream, uint8_t *buffer, size_t buffer_size) {
    if (!parser || !stream || !buffer) return -1;
    
    if (parser->current_frame >= parser->frame_count) {
        msg_Err(parser->obj, "No more frames to read");
        return -1;
    }
    
    kdvd_frame_info_t *frame = &parser->frames[parser->current_frame];
    
    if (buffer_size < frame->size) {
        msg_Err(parser->obj, "Buffer too small for frame %u: %zu < %llu", 
               parser->current_frame, buffer_size, frame->size);
        return -1;
    }
    
    ssize_t bytes_read = vlc_stream_Read(stream, buffer, frame->size);
    if (bytes_read != (ssize_t)frame->size) {
        msg_Err(parser->obj, "Failed to read frame %u: %zd != %llu", 
               parser->current_frame, bytes_read, frame->size);
        return -1;
    }
    
    if (parser->debug_enabled) {
        msg_Dbg(parser->obj, "Read frame %u: %zd bytes", parser->current_frame, bytes_read);
    }
    
    return bytes_read;
}

int kdvd_container_parser_get_next_frame(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    if (parser->current_frame >= parser->frame_count) {
        return -1; // No more frames
    }
    
    parser->current_frame++;
    return 0;
}

int kdvd_container_parser_validate_8kdvd(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    msg_Info(parser->obj, "Validating 8KDVD container");
    
    // Check magic number
    if (parser->info.magic != KDVD_MAGIC_8KDVD && parser->info.magic != KDVD_MAGIC_EVO8) {
        msg_Err(parser->obj, "Invalid 8KDVD magic number: 0x%08X", parser->info.magic);
        return -1;
    }
    
    // Check version
    if (parser->info.version != KDVD_VERSION_1) {
        msg_Err(parser->obj, "Unsupported 8KDVD version: 0x%08X", parser->info.version);
        return -1;
    }
    
    // Check resolution (must be 8K)
    if (parser->info.width != 7680 || parser->info.height != 4320) {
        msg_Err(parser->obj, "Invalid 8KDVD resolution: %ux%u (expected 7680x4320)", 
               parser->info.width, parser->info.height);
        return -1;
    }
    
    // Check frame rate (must be 60 FPS)
    if (parser->info.frame_rate != 60) {
        msg_Err(parser->obj, "Invalid 8KDVD frame rate: %u (expected 60)", parser->info.frame_rate);
        return -1;
    }
    
    // Check bit depth (must be 10-bit for HDR)
    if (parser->info.bit_depth != 10) {
        msg_Err(parser->obj, "Invalid 8KDVD bit depth: %u (expected 10)", parser->info.bit_depth);
        return -1;
    }
    
    msg_Info(parser->obj, "8KDVD container validation successful");
    return 0;
}

int kdvd_container_parser_extract_metadata(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    msg_Info(parser->obj, "Extracting 8KDVD metadata");
    
    // Read container header
    uint32_t header[16];
    if (vlc_stream_Read(stream, header, sizeof(header)) != sizeof(header)) {
        msg_Err(parser->obj, "Failed to read container header");
        return -1;
    }
    
    // Parse header fields
    parser->info.version = header[0];
    parser->info.file_size = ((uint64_t)header[1] << 32) | header[2];
    parser->info.header_size = ((uint64_t)header[3] << 32) | header[4];
    parser->info.payload_size = ((uint64_t)header[5] << 32) | header[6];
    parser->info.frame_count = header[7];
    parser->info.frame_rate = header[8];
    parser->info.width = header[9];
    parser->info.height = header[10];
    parser->info.bit_depth = header[11];
    parser->info.hdr_enabled = (header[12] & 0x01) != 0;
    parser->info.dolby_vision_enabled = (header[12] & 0x02) != 0;
    parser->info.audio_channels = header[13];
    parser->info.audio_sample_rate = header[14];
    parser->info.audio_bitrate = header[15];
    
    // Set codec and container type
    strcpy(parser->info.codec_name, "VP9");
    strcpy(parser->info.container_type, "EVO8");
    
    if (parser->debug_enabled) {
        msg_Dbg(parser->obj, "8KDVD metadata: %ux%u %u-bit %u FPS HDR:%s Dolby:%s %u-ch audio",
               parser->info.width, parser->info.height, parser->info.bit_depth,
               parser->info.frame_rate, parser->info.hdr_enabled ? "yes" : "no",
               parser->info.dolby_vision_enabled ? "yes" : "no", parser->info.audio_channels);
    }
    
    return 0;
}

int kdvd_container_parser_parse_evo8_header(kdvd_container_parser_t *parser, stream_t *stream) {
    if (!parser || !stream) return -1;
    
    msg_Info(parser->obj, "Parsing EVO8 header");
    
    // Read EVO8 header
    uint32_t evo8_header[8];
    if (vlc_stream_Read(stream, evo8_header, sizeof(evo8_header)) != sizeof(evo8_header)) {
        msg_Err(parser->obj, "Failed to read EVO8 header");
        return -1;
    }
    
    // Parse EVO8 specific fields
    parser->info.magic = evo8_header[0];
    parser->info.version = evo8_header[1];
    parser->info.file_size = ((uint64_t)evo8_header[2] << 32) | evo8_header[3];
    parser->info.header_size = ((uint64_t)evo8_header[4] << 32) | evo8_header[5];
    parser->info.payload_size = ((uint64_t)evo8_header[6] << 32) | evo8_header[7];
    
    if (parser->debug_enabled) {
        msg_Dbg(parser->obj, "EVO8 header: magic=0x%08X version=0x%08X size=%llu header=%llu payload=%llu",
               parser->info.magic, parser->info.version, parser->info.file_size,
               parser->info.header_size, parser->info.payload_size);
    }
    
    return 0;
}

int kdvd_container_parser_create_es(kdvd_container_parser_t *parser, demux_t *demux) {
    if (!parser || !demux) return -1;
    
    msg_Info(parser->obj, "Creating 8KDVD elementary streams");
    
    // Create video ES
    es_format_t video_fmt;
    es_format_Init(&video_fmt, VIDEO_ES, VLC_CODEC_VP9);
    video_fmt.video.i_width = parser->info.width;
    video_fmt.video.i_height = parser->info.height;
    video_fmt.video.i_frame_rate = parser->info.frame_rate;
    video_fmt.video.i_frame_rate_base = 1;
    video_fmt.video.i_bits_per_pixel = parser->info.bit_depth;
    video_fmt.video.i_sar_num = 1;
    video_fmt.video.i_sar_den = 1;
    
    if (parser->info.hdr_enabled) {
        video_fmt.video.i_chroma = VLC_CODEC_VP9_HDR;
    }
    
    es_out_id_t *video_es = es_out_Add(demux->out, &video_fmt);
    if (!video_es) {
        msg_Err(parser->obj, "Failed to create video ES");
        es_format_Clean(&video_fmt);
        return -1;
    }
    
    // Create audio ES
    es_format_t audio_fmt;
    es_format_Init(&audio_fmt, AUDIO_ES, VLC_CODEC_OPUS);
    audio_fmt.audio.i_channels = parser->info.audio_channels;
    audio_fmt.audio.i_rate = parser->info.audio_sample_rate;
    audio_fmt.audio.i_bitspersample = 16;
    audio_fmt.audio.i_physical_channels = AOUT_CHANS_8_0; // 8-channel spatial audio
    
    es_out_id_t *audio_es = es_out_Add(demux->out, &audio_fmt);
    if (!audio_es) {
        msg_Err(parser->obj, "Failed to create audio ES");
        es_format_Clean(&video_fmt);
        es_format_Clean(&audio_fmt);
        return -1;
    }
    
    es_format_Clean(&video_fmt);
    es_format_Clean(&audio_fmt);
    
    msg_Info(parser->obj, "8KDVD elementary streams created successfully");
    return 0;
}

int kdvd_container_parser_send_es(kdvd_container_parser_t *parser, demux_t *demux, es_out_t *out) {
    if (!parser || !demux || !out) return -1;
    
    // This would be implemented to send ES data to VLC
    // For now, just return success
    return 0;
}

int kdvd_container_parser_control(kdvd_container_parser_t *parser, demux_t *demux, int query, va_list args) {
    if (!parser || !demux) return -1;
    
    switch (query) {
        case DEMUX_GET_POSITION:
            if (parser->frame_count > 0) {
                float *pos = va_arg(args, float *);
                *pos = (float)parser->current_frame / parser->frame_count;
            }
            break;
            
        case DEMUX_GET_LENGTH:
            if (parser->frame_count > 0) {
                int64_t *length = va_arg(args, int64_t *);
                *length = (int64_t)parser->frame_count * 1000000 / parser->info.frame_rate;
            }
            break;
            
        case DEMUX_GET_TIME:
            if (parser->current_frame < parser->frame_count) {
                int64_t *time = va_arg(args, int64_t *);
                *time = parser->frames[parser->current_frame].timestamp;
            }
            break;
            
        default:
            return -1;
    }
    
    return 0;
}

void kdvd_container_parser_set_debug(kdvd_container_parser_t *parser, bool enable) {
    if (parser) {
        parser->debug_enabled = enable;
        msg_Info(parser->obj, "8KDVD container parser debug %s", enable ? "enabled" : "disabled");
    }
}

void kdvd_container_parser_log_info(kdvd_container_parser_t *parser) {
    if (!parser) return;
    
    msg_Info(parser->obj, "8KDVD Container Info:");
    msg_Info(parser->obj, "  Magic: 0x%08X", parser->info.magic);
    msg_Info(parser->obj, "  Version: 0x%08X", parser->info.version);
    msg_Info(parser->obj, "  File Size: %llu bytes", parser->info.file_size);
    msg_Info(parser->obj, "  Header Size: %llu bytes", parser->info.header_size);
    msg_Info(parser->obj, "  Payload Size: %llu bytes", parser->info.payload_size);
    msg_Info(parser->obj, "  Frame Count: %u", parser->info.frame_count);
    msg_Info(parser->obj, "  Resolution: %ux%u", parser->info.width, parser->info.height);
    msg_Info(parser->obj, "  Frame Rate: %u FPS", parser->info.frame_rate);
    msg_Info(parser->obj, "  Bit Depth: %u-bit", parser->info.bit_depth);
    msg_Info(parser->obj, "  HDR: %s", parser->info.hdr_enabled ? "enabled" : "disabled");
    msg_Info(parser->obj, "  Dolby Vision: %s", parser->info.dolby_vision_enabled ? "enabled" : "disabled");
    msg_Info(parser->obj, "  Audio: %u channels, %u Hz, %u bps", 
           parser->info.audio_channels, parser->info.audio_sample_rate, parser->info.audio_bitrate);
    msg_Info(parser->obj, "  Codec: %s", parser->info.codec_name);
    msg_Info(parser->obj, "  Container: %s", parser->info.container_type);
}
