/**
 * 8KDVD Container Demuxer
 * 
 * This module handles the parsing of 8KDVD container files (PAYLOAD_*.evo8, *.evo4, *.evoh, *.3d4)
 * and extracts video/audio streams according to the 8KDVD specification.
 */

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_stream.h>
#include <vlc_fs.h>
#include <vlc_url.h>
#include "container_parser.c"

#define EIGHTKDVD_MAGIC "8KDVD"

/**
 * 8KDVD demuxer context
 */
typedef struct
{
    demux_t *p_demux;
    stream_t *p_stream;
    
    // 8KDVD container parser
    eightkdvd_container_t container;
    
    // Stream information
    es_out_id_t *p_video_es;
    es_out_id_t *p_audio_es;
    es_out_id_t *p_subtitle_es;
    
    // Playback state
    bool b_playing;
    mtime_t i_start_time;
    
} demux_sys_t;

/**
 * Initialize 8KDVD container and parse available streams
 */
static int Init8KDVDContainer(demux_sys_t *p_sys)
{
    char *psz_disc_path = NULL;
    int i_result = VLC_EGENERIC;
    
    msg_Dbg(p_sys->p_demux, "Initializing 8KDVD container parser");
    
    // Get disc path from stream URL
    psz_disc_path = vlc_uri2path(p_sys->p_stream->psz_url);
    if (!psz_disc_path) {
        msg_Err(p_sys->p_demux, "Cannot get disc path from URL");
        return VLC_EGENERIC;
    }
    
    // Initialize container parser
    if (Init8KDVDContainer(&p_sys->container, psz_disc_path) != VLC_SUCCESS) {
        msg_Err(p_sys->p_demux, "Failed to initialize 8KDVD container");
        free(psz_disc_path);
        return VLC_EGENERIC;
    }
    
    // Log available streams
    eightkdvd_stream_t *p_streams = NULL;
    int i_stream_count = 0;
    
    if (GetAvailableStreams(&p_sys->container, &p_streams, &i_stream_count) == VLC_SUCCESS) {
        msg_Info(p_sys->p_demux, "Found %d available 8KDVD streams:", i_stream_count);
        
        for (int i = 0; i < i_stream_count; i++) {
            eightkdvd_stream_t *p_stream = &p_streams[i];
            msg_Info(p_sys->p_demux, "  %s: %s (%dx%d, %d bps) - %s", 
                    GetFormatName(p_stream->format), p_stream->psz_filename,
                    p_stream->i_width, p_stream->i_height, p_stream->i_bandwidth,
                    p_stream->b_available ? "Available" : "Missing");
        }
    }
    
    free(psz_disc_path);
    return VLC_SUCCESS;
}

/**
 * Open 8KDVD demuxer
 */
static int Open8KDVD(demux_t *p_demux)
{
    demux_sys_t *p_sys;
    
    msg_Dbg(p_demux, "Opening 8KDVD demuxer");
    
    // Allocate demuxer context
    p_sys = calloc(1, sizeof(demux_sys_t));
    if (!p_sys)
        return VLC_ENOMEM;
    
    p_sys->p_demux = p_demux;
    p_sys->p_stream = p_demux->s;
    p_demux->p_sys = p_sys;
    
    // Initialize 8KDVD container parser
    if (Init8KDVDContainer(p_sys) != VLC_SUCCESS) {
        msg_Err(p_demux, "Failed to initialize 8KDVD container");
        free(p_sys);
        return VLC_EGENERIC;
    }
    
    // Set demuxer callbacks
    p_demux->pf_demux = NULL; // TODO: Implement demux function
    p_demux->pf_control = NULL; // TODO: Implement control function
    
    msg_Info(p_demux, "8KDVD demuxer opened successfully");
    return VLC_SUCCESS;
}

/**
 * Close 8KDVD demuxer
 */
static void Close8KDVD(demux_t *p_demux)
{
    demux_sys_t *p_sys = p_demux->p_sys;
    
    if (!p_sys)
        return;
    
    msg_Dbg(p_demux, "Closing 8KDVD demuxer");
    
    // Clean up container parser
    Cleanup8KDVDContainer(&p_sys->container);
    
    // Clean up demuxer context
    free(p_sys);
}

/**
 * VLC module descriptor
 */
vlc_module_begin()
    set_shortname("8KDVD")
    set_description("8KDVD Container Demuxer")
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_DEMUX)
    set_capability("demux", 0)
    set_callbacks(Open8KDVD, Close8KDVD)
vlc_module_end()
