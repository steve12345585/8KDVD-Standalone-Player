/**
 * 8KDVD Container Parser
 * 
 * Handles parsing of all 8KDVD container formats:
 * - EVO8 (8K Ultra HD)
 * - EVO4 (4K Ultra HD) 
 * - EVOH (1080p HD)
 * - 3D4 (3D Anaglyph)
 */

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_stream.h>
#include <vlc_fs.h>
#include <vlc_url.h>
#include <vlc_xml.h>

#define EVO8_EXTENSION ".EVO8"
#define EVO4_EXTENSION ".EVO4"
#define EVOH_EXTENSION ".EVOH"
#define THREED4_EXTENSION ".3D4"

#define PAYLOAD_PREFIX "PAYLOAD_"
#define STREAM_FOLDER "STREAM"
#define PLAYLIST_FOLDER "PLAYLIST"
#define MAIN_PLAYLIST "main.m3u8"

/**
 * 8KDVD container format types
 */
typedef enum {
    EIGHTKDVD_FORMAT_UNKNOWN = 0,
    EIGHTKDVD_FORMAT_EVO8,    // 8K Ultra HD
    EIGHTKDVD_FORMAT_EVO4,    // 4K Ultra HD
    EIGHTKDVD_FORMAT_EVOH,    // 1080p HD
    EIGHTKDVD_FORMAT_3D4      // 3D Anaglyph
} eightkdvd_format_t;

/**
 * 8KDVD stream information
 */
typedef struct {
    char *psz_filename;
    eightkdvd_format_t format;
    int i_bandwidth;
    int i_width;
    int i_height;
    bool b_available;
} eightkdvd_stream_t;

/**
 * 8KDVD container context
 */
typedef struct {
    char *psz_disc_path;
    char *psz_stream_path;
    char *psz_playlist_path;
    
    // Available streams
    eightkdvd_stream_t streams[4]; // EVO8, EVO4, EVOH, 3D4
    int i_stream_count;
    
    // Current playback
    eightkdvd_stream_t *p_current_stream;
    bool b_playing;
    
} eightkdvd_container_t;

/**
 * Get format type from filename
 */
static eightkdvd_format_t GetFormatFromFilename(const char *psz_filename)
{
    if (!psz_filename)
        return EIGHTKDVD_FORMAT_UNKNOWN;
    
    const char *psz_ext = strrchr(psz_filename, '.');
    if (!psz_ext)
        return EIGHTKDVD_FORMAT_UNKNOWN;
    
    if (strcasecmp(psz_ext, EVO8_EXTENSION) == 0)
        return EIGHTKDVD_FORMAT_EVO8;
    else if (strcasecmp(psz_ext, EVO4_EXTENSION) == 0)
        return EIGHTKDVD_FORMAT_EVO4;
    else if (strcasecmp(psz_ext, EVOH_EXTENSION) == 0)
        return EIGHTKDVD_FORMAT_EVOH;
    else if (strcasecmp(psz_ext, THREED4_EXTENSION) == 0)
        return EIGHTKDVD_FORMAT_3D4;
    
    return EIGHTKDVD_FORMAT_UNKNOWN;
}

/**
 * Get format name string
 */
static const char* GetFormatName(eightkdvd_format_t format)
{
    switch (format) {
        case EIGHTKDVD_FORMAT_EVO8: return "8K Ultra HD";
        case EIGHTKDVD_FORMAT_EVO4: return "4K Ultra HD";
        case EIGHTKDVD_FORMAT_EVOH: return "1080p HD";
        case EIGHTKDVD_FORMAT_3D4:  return "3D Anaglyph";
        default: return "Unknown";
    }
}

/**
 * Parse HLS playlist to find available streams
 */
static int ParseHLSPlaylist(eightkdvd_container_t *p_container)
{
    char *psz_playlist_content = NULL;
    FILE *p_file = NULL;
    long l_size;
    char *psz_line = NULL;
    size_t line_len = 0;
    
    msg_Dbg(NULL, "Parsing HLS playlist: %s", p_container->psz_playlist_path);
    
    p_file = vlc_fopen(p_container->psz_playlist_path, "rb");
    if (!p_file) {
        msg_Err(NULL, "Cannot open playlist file");
        return VLC_EGENERIC;
    }
    
    // Get file size
    fseek(p_file, 0, SEEK_END);
    l_size = ftell(p_file);
    fseek(p_file, 0, SEEK_SET);
    
    // Read file content
    psz_playlist_content = malloc(l_size + 1);
    if (!psz_playlist_content) {
        fclose(p_file);
        return VLC_EGENERIC;
    }
    
    if (fread(psz_playlist_content, 1, l_size, p_file) != l_size) {
        msg_Err(NULL, "Failed to read playlist file");
        free(psz_playlist_content);
        fclose(p_file);
        return VLC_EGENERIC;
    }
    
    psz_playlist_content[l_size] = '\0';
    fclose(p_file);
    
    // Parse playlist line by line
    psz_line = psz_playlist_content;
    p_container->i_stream_count = 0;
    
    while (psz_line && p_container->i_stream_count < 4) {
        char *psz_next_line = strchr(psz_line, '\n');
        if (psz_next_line) {
            *psz_next_line = '\0';
            psz_next_line++;
        }
        
        // Skip empty lines and comments
        if (psz_line[0] != '\0' && psz_line[0] != '#') {
            // This is a stream file
            char *psz_filename = strrchr(psz_line, '/');
            if (psz_filename) {
                psz_filename++; // Skip the '/'
                
                eightkdvd_format_t format = GetFormatFromFilename(psz_filename);
                if (format != EIGHTKDVD_FORMAT_UNKNOWN) {
                    eightkdvd_stream_t *p_stream = &p_container->streams[p_container->i_stream_count];
                    
                    p_stream->psz_filename = strdup(psz_filename);
                    p_stream->format = format;
                    p_stream->b_available = true;
                    
                    // Set default bandwidth and resolution based on format
                    switch (format) {
                        case EIGHTKDVD_FORMAT_EVO8:
                            p_stream->i_bandwidth = 100000000;
                            p_stream->i_width = 7680;
                            p_stream->i_height = 4320;
                            break;
                        case EIGHTKDVD_FORMAT_EVO4:
                            p_stream->i_bandwidth = 20000000;
                            p_stream->i_width = 3840;
                            p_stream->i_height = 2160;
                            break;
                        case EIGHTKDVD_FORMAT_EVOH:
                            p_stream->i_bandwidth = 10000000;
                            p_stream->i_width = 1920;
                            p_stream->i_height = 1080;
                            break;
                        case EIGHTKDVD_FORMAT_3D4:
                            p_stream->i_bandwidth = 20000000;
                            p_stream->i_width = 3840;
                            p_stream->i_height = 2160;
                            break;
                        default:
                            break;
                    }
                    
                    msg_Info(NULL, "Found %s stream: %s (%dx%d, %d bps)", 
                            GetFormatName(format), psz_filename, 
                            p_stream->i_width, p_stream->i_height, p_stream->i_bandwidth);
                    
                    p_container->i_stream_count++;
                }
            }
        }
        
        psz_line = psz_next_line;
    }
    
    free(psz_playlist_content);
    
    msg_Info(NULL, "Parsed %d available streams", p_container->i_stream_count);
    return VLC_SUCCESS;
}

/**
 * Check if stream file exists
 */
static bool CheckStreamAvailability(eightkdvd_container_t *p_container, eightkdvd_stream_t *p_stream)
{
    char *psz_full_path = NULL;
    bool b_exists = false;
    
    if (asprintf(&psz_full_path, "%s/%s", p_container->psz_stream_path, p_stream->psz_filename) == -1)
        return false;
    
    b_exists = (vlc_stat(psz_full_path, NULL) == 0);
    
    if (b_exists) {
        msg_Dbg(NULL, "Stream file exists: %s", psz_full_path);
    } else {
        msg_Dbg(NULL, "Stream file missing: %s", psz_full_path);
    }
    
    free(psz_full_path);
    return b_exists;
}

/**
 * Initialize 8KDVD container parser
 */
static int Init8KDVDContainer(eightkdvd_container_t *p_container, const char *psz_disc_path)
{
    // Initialize container context
    memset(p_container, 0, sizeof(eightkdvd_container_t));
    
    p_container->psz_disc_path = strdup(psz_disc_path);
    if (!p_container->psz_disc_path)
        return VLC_ENOMEM;
    
    // Build stream path
    if (asprintf(&p_container->psz_stream_path, "%s/8KDVD_TS/%s", psz_disc_path, STREAM_FOLDER) == -1) {
        free(p_container->psz_disc_path);
        return VLC_ENOMEM;
    }
    
    // Build playlist path
    if (asprintf(&p_container->psz_playlist_path, "%s/8KDVD_TS/%s/%s", psz_disc_path, PLAYLIST_FOLDER, MAIN_PLAYLIST) == -1) {
        free(p_container->psz_disc_path);
        free(p_container->psz_stream_path);
        return VLC_ENOMEM;
    }
    
    // Parse HLS playlist to find available streams
    if (ParseHLSPlaylist(p_container) != VLC_SUCCESS) {
        msg_Err(NULL, "Failed to parse HLS playlist");
        free(p_container->psz_disc_path);
        free(p_container->psz_stream_path);
        free(p_container->psz_playlist_path);
        return VLC_EGENERIC;
    }
    
    // Check availability of each stream
    for (int i = 0; i < p_container->i_stream_count; i++) {
        eightkdvd_stream_t *p_stream = &p_container->streams[i];
        p_stream->b_available = CheckStreamAvailability(p_container, p_stream);
        
        if (p_stream->b_available) {
            msg_Info(NULL, "✓ %s stream available: %s", 
                    GetFormatName(p_stream->format), p_stream->psz_filename);
        } else {
            msg_Warn(NULL, "✗ %s stream missing: %s", 
                    GetFormatName(p_stream->format), p_stream->psz_filename);
        }
    }
    
    return VLC_SUCCESS;
}

/**
 * Cleanup 8KDVD container parser
 */
static void Cleanup8KDVDContainer(eightkdvd_container_t *p_container)
{
    if (!p_container)
        return;
    
    // Free stream filenames
    for (int i = 0; i < p_container->i_stream_count; i++) {
        free(p_container->streams[i].psz_filename);
    }
    
    // Free paths
    free(p_container->psz_disc_path);
    free(p_container->psz_stream_path);
    free(p_container->psz_playlist_path);
    
    memset(p_container, 0, sizeof(eightkdvd_container_t));
}

/**
 * Get available streams
 */
static int GetAvailableStreams(eightkdvd_container_t *p_container, eightkdvd_stream_t **pp_streams, int *pi_count)
{
    if (!p_container || !pp_streams || !pi_count)
        return VLC_EGENERIC;
    
    *pp_streams = p_container->streams;
    *pi_count = p_container->i_stream_count;
    
    return VLC_SUCCESS;
}

/**
 * Select stream by format
 */
static int SelectStream(eightkdvd_container_t *p_container, eightkdvd_format_t format)
{
    if (!p_container)
        return VLC_EGENERIC;
    
    for (int i = 0; i < p_container->i_stream_count; i++) {
        eightkdvd_stream_t *p_stream = &p_container->streams[i];
        
        if (p_stream->format == format && p_stream->b_available) {
            p_container->p_current_stream = p_stream;
            msg_Info(NULL, "Selected %s stream: %s", 
                    GetFormatName(format), p_stream->psz_filename);
            return VLC_SUCCESS;
        }
    }
    
    msg_Err(NULL, "No available stream found for format %d", format);
    return VLC_EGENERIC;
}

/**
 * Get current stream
 */
static eightkdvd_stream_t* GetCurrentStream(eightkdvd_container_t *p_container)
{
    return p_container ? p_container->p_current_stream : NULL;
}
