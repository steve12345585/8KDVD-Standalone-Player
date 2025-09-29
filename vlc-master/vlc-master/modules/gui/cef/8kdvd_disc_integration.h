#ifndef VLC_8KDVD_DISC_INTEGRATION_H
#define VLC_8KDVD_DISC_INTEGRATION_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_playlist.h>
#include <string>
#include <vector>
#include <map>

// 8KDVD Disc Integration
typedef struct kdvd_disc_integration_t kdvd_disc_integration_t;

// 8KDVD Disc Information
typedef struct kdvd_disc_info_t {
    std::string disc_path;
    std::string disc_id;
    std::string title;
    std::string version;
    std::string region;
    std::string language;
    std::vector<std::string> audio_tracks;
    std::vector<std::string> subtitle_tracks;
    std::vector<std::string> chapters;
    bool hdr_enabled;
    bool dolby_vision_enabled;
    int resolution_width;
    int resolution_height;
    int frame_rate;
} kdvd_disc_info_t;

// 8KDVD Title Information
typedef struct kdvd_title_info_t {
    int title_id;
    std::string title_name;
    std::string description;
    int duration;
    std::string thumbnail;
    std::vector<std::string> chapters;
    bool playable;
} kdvd_title_info_t;

// 8KDVD Disc Integration Functions
kdvd_disc_integration_t* kdvd_disc_integration_create(vlc_object_t *obj);
void kdvd_disc_integration_destroy(kdvd_disc_integration_t *integration);

// Disc Detection and Loading
int kdvd_disc_integration_detect_disc(kdvd_disc_integration_t *integration, const char *disc_path);
int kdvd_disc_integration_load_disc(kdvd_disc_integration_t *integration, const char *disc_path);
int kdvd_disc_integration_unload_disc(kdvd_disc_integration_t *integration);

// Disc Information
kdvd_disc_info_t kdvd_disc_integration_get_disc_info(kdvd_disc_integration_t *integration);
std::vector<kdvd_title_info_t> kdvd_disc_integration_get_titles(kdvd_disc_integration_t *integration);
kdvd_title_info_t kdvd_disc_integration_get_title_info(kdvd_disc_integration_t *integration, int title_id);

// Playback Control
int kdvd_disc_integration_play_title(kdvd_disc_integration_t *integration, int title_id);
int kdvd_disc_integration_stop_playback(kdvd_disc_integration_t *integration);
int kdvd_disc_integration_pause_playback(kdvd_disc_integration_t *integration);
int kdvd_disc_integration_resume_playback(kdvd_disc_integration_t *integration);

// Menu Integration
int kdvd_disc_integration_load_main_menu(kdvd_disc_integration_t *integration);
int kdvd_disc_integration_load_settings_menu(kdvd_disc_integration_t *integration);
int kdvd_disc_integration_load_about_menu(kdvd_disc_integration_t *integration);

// VLC Integration
void kdvd_disc_integration_set_vlc_integration(kdvd_disc_integration_t *integration, bool enable);
void kdvd_disc_integration_set_debug_output(kdvd_disc_integration_t *integration, bool enable);

// 8KDVD Specific Functions
int kdvd_disc_integration_validate_disc(kdvd_disc_integration_t *integration, const char *disc_path);
int kdvd_disc_integration_extract_metadata(kdvd_disc_integration_t *integration, const char *disc_path);
int kdvd_disc_integration_setup_playback(kdvd_disc_integration_t *integration, int title_id);

#endif // VLC_8KDVD_DISC_INTEGRATION_H
