#ifndef VLC_8KDVD_CONTAINER_PARSER_H
#define VLC_8KDVD_CONTAINER_PARSER_H

#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_stream.h>
#include <vlc_fourcc.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_meta.h>
#include <vlc_input_item.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Container Parser for PAYLOAD_*.evo8 files
typedef struct kdvd_container_parser_t kdvd_container_parser_t;

// 8KDVD Container Information
typedef struct kdvd_container_info_t {
    uint32_t magic;                    // File magic number
    uint32_t version;                  // Container version
    uint64_t file_size;               // Total file size
    uint64_t header_size;             // Header size
    uint64_t payload_size;            // Payload size
    uint32_t frame_count;             // Number of frames
    uint32_t frame_rate;              // Frame rate (60 FPS for 8K)
    uint32_t width;                   // Video width (7680 for 8K)
    uint32_t height;                  // Video height (4320 for 8K)
    uint32_t bit_depth;               // Bit depth (10-bit for HDR)
    bool hdr_enabled;                 // HDR support
    bool dolby_vision_enabled;        // Dolby Vision support
    uint32_t audio_channels;          // Audio channels (8 for spatial)
    uint32_t audio_sample_rate;       // Audio sample rate
    uint32_t audio_bitrate;           // Audio bitrate
    char codec_name[32];              // Codec name
    char container_type[32];           // Container type
} kdvd_container_info_t;

// 8KDVD Frame Information
typedef struct kdvd_frame_info_t {
    uint64_t offset;                  // Frame offset in file
    uint64_t size;                    // Frame size
    uint32_t frame_number;            // Frame number
    uint64_t timestamp;               // Frame timestamp
    bool keyframe;                    // Is keyframe
    uint32_t frame_type;              // Frame type (I/P/B)
    uint32_t quality;                 // Frame quality
} kdvd_frame_info_t;

// 8KDVD Container Parser Functions
kdvd_container_parser_t* kdvd_container_parser_create(vlc_object_t *obj);
void kdvd_container_parser_destroy(kdvd_container_parser_t *parser);

// Container Detection and Parsing
int kdvd_container_parser_detect(kdvd_container_parser_t *parser, stream_t *stream);
int kdvd_container_parser_parse_header(kdvd_container_parser_t *parser, stream_t *stream);
int kdvd_container_parser_parse_frames(kdvd_container_parser_t *parser, stream_t *stream);

// Container Information
kdvd_container_info_t kdvd_container_parser_get_info(kdvd_container_parser_t *parser);
int kdvd_container_parser_get_frame_count(kdvd_container_parser_t *parser);
kdvd_frame_info_t kdvd_container_parser_get_frame(kdvd_container_parser_t *parser, uint32_t frame_index);

// Frame Access
int kdvd_container_parser_seek_to_frame(kdvd_container_parser_t *parser, stream_t *stream, uint32_t frame_index);
int kdvd_container_parser_read_frame(kdvd_container_parser_t *parser, stream_t *stream, uint8_t *buffer, size_t buffer_size);
int kdvd_container_parser_get_next_frame(kdvd_container_parser_t *parser, stream_t *stream);

// 8KDVD Specific Functions
int kdvd_container_parser_validate_8kdvd(kdvd_container_parser_t *parser, stream_t *stream);
int kdvd_container_parser_extract_metadata(kdvd_container_parser_t *parser, stream_t *stream);
int kdvd_container_parser_parse_evo8_header(kdvd_container_parser_t *parser, stream_t *stream);

// VLC Integration
int kdvd_container_parser_create_es(kdvd_container_parser_t *parser, demux_t *demux);
int kdvd_container_parser_send_es(kdvd_container_parser_t *parser, demux_t *demux, es_out_t *out);
int kdvd_container_parser_control(kdvd_container_parser_t *parser, demux_t *demux, int query, va_list args);

// Debug and Logging
void kdvd_container_parser_set_debug(kdvd_container_parser_t *parser, bool enable);
void kdvd_container_parser_log_info(kdvd_container_parser_t *parser);

#endif // VLC_8KDVD_CONTAINER_PARSER_H
