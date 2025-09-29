#include <vlc_common.h>
#include <vlc_vout.h>
#include <vlc_picture.h>
#include <vlc_fourcc.h>
#include <vlc_window.h>
#include <vlc_opengl.h>
#include <vlc_gl.h>
#include "8k_video_renderer.h"

// 8KDVD Video Output Module for VLC
typedef struct vout_sys_t {
    kdvd_8k_renderer_t *renderer;
    bool initialized;
    bool debug_enabled;
} vout_sys_t;

// Module descriptor
vlc_module_begin()
    set_shortname("8KDVD Vout")
    set_description("8KDVD 8K Video Output")
    set_capability("vout", 10)
    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VOUT)
    set_callbacks(Open, Close)
    add_shortcut("8kdvd", "8k_vout")
vlc_module_end()

// Forward declarations
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);
static int Display(vout_display_t *, picture_t *);
static int Control(vout_display_t *, int, va_list);

// Module functions
static int Open(vlc_object_t *obj) {
    vout_display_t *vout = (vout_display_t *)obj;
    vout_sys_t *sys = calloc(1, sizeof(vout_sys_t));
    if (!sys) {
        msg_Err(vout, "Failed to allocate video output system");
        return VLC_EGENERIC;
    }
    
    msg_Info(vout, "8KDVD video output module opening");
    
    // Check if this is an 8K video output
    if (vout->fmt.video.i_width != 7680 || vout->fmt.video.i_height != 4320) {
        msg_Err(vout, "Not an 8K video output: %ux%u (expected 7680x4320)", 
               vout->fmt.video.i_width, vout->fmt.video.i_height);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Create 8K renderer
    sys->renderer = kdvd_8k_renderer_create(vout);
    if (!sys->renderer) {
        msg_Err(vout, "Failed to create 8K renderer");
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Configure 8K renderer
    kdvd_8k_render_config_t render_config = {
        .width = vout->fmt.video.i_width,
        .height = vout->fmt.video.i_height,
        .bit_depth = vout->fmt.video.i_bits_per_pixel,
        .frame_rate = vout->fmt.video.i_frame_rate,
        .hdr_enabled = (vout->fmt.video.i_chroma == VLC_CODEC_VP9_HDR),
        .dolby_vision_enabled = (vout->fmt.video.i_bits_per_pixel == 12),
        .hardware_acceleration = true,
        .color_space = 1,  // BT.2020 for HDR
        .color_range = 1,  // Full range for HDR
        .chroma_subsampling = 1,  // 4:2:0
        .vsync_enabled = true,
        .triple_buffering = true,
        .max_fps = 60,
        .adaptive_sync = true
    };
    
    if (kdvd_8k_renderer_configure(sys->renderer, &render_config) != 0) {
        msg_Err(vout, "Failed to configure 8K renderer");
        kdvd_8k_renderer_destroy(sys->renderer);
        free(sys);
        return VLC_EGENERIC;
    }
    
    // Enable 8K mode
    kdvd_8k_renderer_enable_8k_rendering(sys->renderer, true);
    kdvd_8k_renderer_optimize_for_8k(sys->renderer);
    
    // Set up video output
    vout->p_sys = sys;
    vout->pf_display = Display;
    vout->pf_control = Control;
    
    // Set up video format
    vout->fmt.video.i_width = 7680;
    vout->fmt.video.i_height = 4320;
    vout->fmt.video.i_bits_per_pixel = 10;
    vout->fmt.video.i_frame_rate = 60;
    vout->fmt.video.i_frame_rate_base = 1;
    vout->fmt.video.i_sar_num = 1;
    vout->fmt.video.i_sar_den = 1;
    vout->fmt.video.i_chroma = VLC_CODEC_I420_10L;
    
    sys->initialized = true;
    sys->debug_enabled = false;
    
    msg_Info(vout, "8KDVD video output module opened successfully");
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj) {
    vout_display_t *vout = (vout_display_t *)obj;
    vout_sys_t *sys = vout->p_sys;
    
    if (!sys) return;
    
    msg_Info(vout, "8KDVD video output module closing");
    
    if (sys->renderer) {
        kdvd_8k_renderer_destroy(sys->renderer);
    }
    
    free(sys);
    vout->p_sys = NULL;
    
    msg_Info(vout, "8KDVD video output module closed");
}

static int Display(vout_display_t *vout, picture_t *picture) {
    vout_sys_t *sys = vout->p_sys;
    if (!sys || !picture) return 0;
    
    if (!sys->initialized) {
        msg_Err(vout, "8KDVD video output not initialized");
        return -1;
    }
    
    // Render 8K frame
    if (kdvd_8k_renderer_render_frame(sys->renderer, picture) != 0) {
        msg_Err(vout, "Failed to render 8K frame");
        return -1;
    }
    
    // Present frame to display
    if (kdvd_8k_renderer_present_frame(sys->renderer) != 0) {
        msg_Err(vout, "Failed to present 8K frame");
        return -1;
    }
    
    if (sys->debug_enabled) {
        kdvd_8k_render_stats_t stats = kdvd_8k_renderer_get_stats(sys->renderer);
        msg_Dbg(vout, "8K frame displayed: %llu frames, %.2f FPS", 
               stats.frames_rendered, stats.average_fps);
    }
    
    return 0;
}

static int Control(vout_display_t *vout, int query, va_list args) {
    vout_sys_t *sys = vout->p_sys;
    if (!sys) return VLC_EGENERIC;
    
    switch (query) {
        case VOUT_DISPLAY_CHANGE_FULLSCREEN: {
            bool fullscreen = va_arg(args, bool);
            msg_Info(vout, "8KDVD fullscreen %s", fullscreen ? "enabled" : "disabled");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_WINDOW_STATE: {
            unsigned state = va_arg(args, unsigned);
            msg_Info(vout, "8KDVD window state changed: %u", state);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_DISPLAY_SIZE: {
            unsigned width = va_arg(args, unsigned);
            unsigned height = va_arg(args, unsigned);
            msg_Info(vout, "8KDVD display size changed: %ux%u", width, height);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_SOURCE_ASPECT: {
            unsigned num = va_arg(args, unsigned);
            unsigned den = va_arg(args, unsigned);
            msg_Info(vout, "8KDVD source aspect changed: %u:%u", num, den);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_SOURCE_CROP: {
            unsigned left = va_arg(args, unsigned);
            unsigned top = va_arg(args, unsigned);
            unsigned right = va_arg(args, unsigned);
            unsigned bottom = va_arg(args, unsigned);
            msg_Info(vout, "8KDVD source crop changed: (%u,%u,%u,%u)", left, top, right, bottom);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_FILTER: {
            const char *filter = va_arg(args, const char *);
            msg_Info(vout, "8KDVD filter changed: %s", filter ? filter : "none");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_ZOOM: {
            float zoom = va_arg(args, double);
            msg_Info(vout, "8KDVD zoom changed: %.2f", zoom);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_VIEWPOINT: {
            const vout_display_cfg_t *cfg = va_arg(args, const vout_display_cfg_t *);
            msg_Info(vout, "8KDVD viewpoint changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_SUBPICTURE: {
            subpicture_t *subpicture = va_arg(args, subpicture_t *);
            msg_Info(vout, "8KDVD subpicture changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_STRING: {
            const char *osd_string = va_arg(args, const char *);
            msg_Info(vout, "8KDVD OSD string: %s", osd_string ? osd_string : "none");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_SLIDER: {
            int position = va_arg(args, int);
            int type = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD slider: position=%d type=%d", position, type);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_ICON: {
            int icon = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD icon: %d", icon);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_MESSAGE: {
            const char *message = va_arg(args, const char *);
            msg_Info(vout, "8KDVD OSD message: %s", message ? message : "none");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_META: {
            const vlc_meta_t *meta = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD meta changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_INTEGER: {
            int value = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD integer: %d", value);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_TIME: {
            mtime_t time = va_arg(args, mtime_t);
            msg_Info(vout, "8KDVD OSD time: %lld", time);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_POSITION: {
            float position = va_arg(args, double);
            msg_Info(vout, "8KDVD OSD position: %.2f", position);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_CHAPTER: {
            int chapter = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD chapter: %d", chapter);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_PROGRAM: {
            int program = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD program: %d", program);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_TITLE: {
            int title = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD title: %d", title);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_SEEKPOINT: {
            int seekpoint = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD seekpoint: %d", seekpoint);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_TRACK: {
            int track = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD track: %d", track);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_AUDIO: {
            int audio = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD audio: %d", audio);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_VIDEO: {
            int video = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD video: %d", video);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_SUBTITLE: {
            int subtitle = va_arg(args, int);
            msg_Info(vout, "8KDVD OSD subtitle: %d", subtitle);
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_CHAPTERS: {
            const vlc_meta_t *chapters = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD chapters changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_PROGRAMS: {
            const vlc_meta_t *programs = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD programs changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_TITLES: {
            const vlc_meta_t *titles = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD titles changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_SEEKPOINTS: {
            const vlc_meta_t *seekpoints = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD seekpoints changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_TRACKS: {
            const vlc_meta_t *tracks = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD tracks changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_AUDIOS: {
            const vlc_meta_t *audios = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD audios changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_VIDEOS: {
            const vlc_meta_t *videos = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD videos changed");
            return VLC_SUCCESS;
        }
        
        case VOUT_DISPLAY_CHANGE_OSD_SUBTITLES: {
            const vlc_meta_t *subtitles = va_arg(args, const vlc_meta_t *);
            msg_Info(vout, "8KDVD OSD subtitles changed");
            return VLC_SUCCESS;
        }
        
        default:
            return VLC_EGENERIC;
    }
}
