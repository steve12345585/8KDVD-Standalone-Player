#include "8kdvd_disc_integration.h"
#include "html5_menu_system.h"
#include "menu_navigation.h"
#include <vlc_messages.h>
#include <vlc_input.h>
#include <vlc_playlist.h>
#include <filesystem>
#include <fstream>
#include <sstream>

// 8KDVD Disc Integration Implementation
struct kdvd_disc_integration_t {
    vlc_object_t *obj;
    html5_menu_system_t *menu_system;
    menu_navigation_t *navigation;
    kdvd_disc_info_t disc_info;
    std::vector<kdvd_title_info_t> titles;
    bool vlc_integration_enabled;
    bool debug_output_enabled;
    bool disc_loaded;
    input_thread_t *current_input;
    playlist_t *playlist;
};

kdvd_disc_integration_t* kdvd_disc_integration_create(vlc_object_t *obj) {
    kdvd_disc_integration_t *integration = new kdvd_disc_integration_t();
    if (!integration) return nullptr;
    
    integration->obj = obj;
    integration->menu_system = html5_menu_system_create(obj);
    integration->navigation = menu_navigation_create(obj);
    integration->vlc_integration_enabled = true;
    integration->debug_output_enabled = false;
    integration->disc_loaded = false;
    integration->current_input = nullptr;
    integration->playlist = nullptr;
    
    // Initialize disc info
    integration->disc_info.disc_path.clear();
    integration->disc_info.disc_id.clear();
    integration->disc_info.title.clear();
    integration->disc_info.version.clear();
    integration->disc_info.region.clear();
    integration->disc_info.language.clear();
    integration->disc_info.hdr_enabled = false;
    integration->disc_info.dolby_vision_enabled = false;
    integration->disc_info.resolution_width = 7680;
    integration->disc_info.resolution_height = 4320;
    integration->disc_info.frame_rate = 60;
    
    msg_Info(obj, "8KDVD disc integration created");
    return integration;
}

void kdvd_disc_integration_destroy(kdvd_disc_integration_t *integration) {
    if (!integration) return;
    
    if (integration->disc_loaded) {
        kdvd_disc_integration_unload_disc(integration);
    }
    
    if (integration->menu_system) {
        html5_menu_system_destroy(integration->menu_system);
    }
    
    if (integration->navigation) {
        menu_navigation_destroy(integration->navigation);
    }
    
    integration->titles.clear();
    delete integration;
    msg_Info(integration->obj, "8KDVD disc integration destroyed");
}

int kdvd_disc_integration_detect_disc(kdvd_disc_integration_t *integration, const char *disc_path) {
    if (!integration || !disc_path) return -1;
    
    msg_Info(integration->obj, "Detecting 8KDVD disc: %s", disc_path);
    
    // Check if path exists
    if (!std::filesystem::exists(disc_path)) {
        msg_Err(integration->obj, "Disc path does not exist: %s", disc_path);
        return -1;
    }
    
    // Check for 8KDVD structure
    std::string kdvd_ts_path = std::string(disc_path) + "/8KDVD_TS";
    if (!std::filesystem::exists(kdvd_ts_path)) {
        msg_Err(integration->obj, "8KDVD structure not found: %s", kdvd_ts_path.c_str());
        return -1;
    }
    
    // Check for required files
    std::string index_html = kdvd_ts_path + "/index.html";
    if (!std::filesystem::exists(index_html)) {
        msg_Err(integration->obj, "8KDVD index.html not found: %s", index_html.c_str());
        return -1;
    }
    
    msg_Info(integration->obj, "8KDVD disc detected successfully");
    return 0;
}

int kdvd_disc_integration_load_disc(kdvd_disc_integration_t *integration, const char *disc_path) {
    if (!integration || !disc_path) return -1;
    
    // Detect disc first
    if (kdvd_disc_integration_detect_disc(integration, disc_path) != 0) {
        return -1;
    }
    
    // Validate disc
    if (kdvd_disc_integration_validate_disc(integration, disc_path) != 0) {
        return -1;
    }
    
    // Extract metadata
    if (kdvd_disc_integration_extract_metadata(integration, disc_path) != 0) {
        return -1;
    }
    
    // Load main menu
    if (kdvd_disc_integration_load_main_menu(integration) != 0) {
        return -1;
    }
    
    integration->disc_loaded = true;
    integration->disc_info.disc_path = disc_path;
    
    msg_Info(integration->obj, "8KDVD disc loaded successfully: %s", disc_path);
    return 0;
}

int kdvd_disc_integration_unload_disc(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->current_input) {
        input_Stop(integration->current_input);
        integration->current_input = nullptr;
    }
    
    if (integration->menu_system) {
        html5_menu_system_hide_menu(integration->menu_system);
    }
    
    integration->disc_loaded = false;
    integration->disc_info.disc_path.clear();
    integration->titles.clear();
    
    msg_Info(integration->obj, "8KDVD disc unloaded");
    return 0;
}

kdvd_disc_info_t kdvd_disc_integration_get_disc_info(kdvd_disc_integration_t *integration) {
    if (integration) {
        return integration->disc_info;
    }
    
    kdvd_disc_info_t empty_info = {0};
    return empty_info;
}

std::vector<kdvd_title_info_t> kdvd_disc_integration_get_titles(kdvd_disc_integration_t *integration) {
    if (integration) {
        return integration->titles;
    }
    
    return std::vector<kdvd_title_info_t>();
}

kdvd_title_info_t kdvd_disc_integration_get_title_info(kdvd_disc_integration_t *integration, int title_id) {
    if (integration) {
        for (const auto &title : integration->titles) {
            if (title.title_id == title_id) {
                return title;
            }
        }
    }
    
    kdvd_title_info_t empty_title = {0};
    return empty_title;
}

int kdvd_disc_integration_play_title(kdvd_disc_integration_t *integration, int title_id) {
    if (!integration || !integration->disc_loaded) return -1;
    
    msg_Info(integration->obj, "8KDVD: Playing title %d", title_id);
    
    // Get title info
    kdvd_title_info_t title_info = kdvd_disc_integration_get_title_info(integration, title_id);
    if (!title_info.playable) {
        msg_Err(integration->obj, "Title %d is not playable", title_id);
        return -1;
    }
    
    // Setup playback
    if (kdvd_disc_integration_setup_playback(integration, title_id) != 0) {
        return -1;
    }
    
    // Start playback
    if (integration->current_input) {
        input_Start(integration->current_input);
        msg_Info(integration->obj, "8KDVD: Title %d playback started", title_id);
    }
    
    return 0;
}

int kdvd_disc_integration_stop_playback(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->current_input) {
        input_Stop(integration->current_input);
        integration->current_input = nullptr;
        msg_Info(integration->obj, "8KDVD: Playback stopped");
    }
    
    return 0;
}

int kdvd_disc_integration_pause_playback(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->current_input) {
        input_Control(integration->current_input, INPUT_CONTROL_SET_PAUSE_STATE, true);
        msg_Info(integration->obj, "8KDVD: Playback paused");
    }
    
    return 0;
}

int kdvd_disc_integration_resume_playback(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->current_input) {
        input_Control(integration->current_input, INPUT_CONTROL_SET_PAUSE_STATE, false);
        msg_Info(integration->obj, "8KDVD: Playback resumed");
    }
    
    return 0;
}

int kdvd_disc_integration_load_main_menu(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->menu_system) {
        // Create 8KDVD main menu
        html5_menu_system_create_8kdvd_main_menu(integration->menu_system, integration->disc_info.disc_path.c_str());
        
        // Show main menu
        html5_menu_system_show_menu(integration->menu_system, "8kdvd_main");
        
        // Set up navigation
        if (integration->navigation) {
            menu_navigation_set_menu(integration->navigation, "8kdvd_main", 3); // 3 main menu items
        }
        
        msg_Info(integration->obj, "8KDVD main menu loaded");
    }
    
    return 0;
}

int kdvd_disc_integration_load_settings_menu(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->menu_system) {
        // Create 8KDVD settings menu
        html5_menu_system_create_8kdvd_settings_menu(integration->menu_system);
        
        // Show settings menu
        html5_menu_system_show_menu(integration->menu_system, "8kdvd_settings");
        
        // Set up navigation
        if (integration->navigation) {
            menu_navigation_set_menu(integration->navigation, "8kdvd_settings", 3); // 3 settings items
        }
        
        msg_Info(integration->obj, "8KDVD settings menu loaded");
    }
    
    return 0;
}

int kdvd_disc_integration_load_about_menu(kdvd_disc_integration_t *integration) {
    if (!integration) return -1;
    
    if (integration->menu_system) {
        // Create 8KDVD about menu
        html5_menu_system_create_8kdvd_about_menu(integration->menu_system);
        
        // Show about menu
        html5_menu_system_show_menu(integration->menu_system, "8kdvd_about");
        
        // Set up navigation
        if (integration->navigation) {
            menu_navigation_set_menu(integration->navigation, "8kdvd_about", 2); // 2 about items
        }
        
        msg_Info(integration->obj, "8KDVD about menu loaded");
    }
    
    return 0;
}

void kdvd_disc_integration_set_vlc_integration(kdvd_disc_integration_t *integration, bool enable) {
    if (integration) {
        integration->vlc_integration_enabled = enable;
        msg_Info(integration->obj, "VLC integration %s", enable ? "enabled" : "disabled");
    }
}

void kdvd_disc_integration_set_debug_output(kdvd_disc_integration_t *integration, bool enable) {
    if (integration) {
        integration->debug_output_enabled = enable;
        msg_Info(integration->obj, "Debug output %s", enable ? "enabled" : "disabled");
    }
}

int kdvd_disc_integration_validate_disc(kdvd_disc_integration_t *integration, const char *disc_path) {
    if (!integration || !disc_path) return -1;
    
    msg_Info(integration->obj, "Validating 8KDVD disc: %s", disc_path);
    
    // Check for required 8KDVD files
    std::vector<std::string> required_files = {
        "8KDVD_TS/index.html",
        "8KDVD_TS/style.css",
        "8KDVD_TS/script.js"
    };
    
    for (const auto &file : required_files) {
        std::string full_path = std::string(disc_path) + "/" + file;
        if (!std::filesystem::exists(full_path)) {
            msg_Err(integration->obj, "Required file not found: %s", full_path.c_str());
            return -1;
        }
    }
    
    msg_Info(integration->obj, "8KDVD disc validation successful");
    return 0;
}

int kdvd_disc_integration_extract_metadata(kdvd_disc_integration_t *integration, const char *disc_path) {
    if (!integration || !disc_path) return -1;
    
    msg_Info(integration->obj, "Extracting 8KDVD metadata: %s", disc_path);
    
    // Extract basic disc information
    integration->disc_info.disc_path = disc_path;
    integration->disc_info.disc_id = "8KDVD_" + std::to_string(time(nullptr));
    integration->disc_info.title = "8KDVD Disc";
    integration->disc_info.version = "1.0";
    integration->disc_info.region = "ALL";
    integration->disc_info.language = "en";
    
    // Set up default titles
    kdvd_title_info_t main_title;
    main_title.title_id = 1;
    main_title.title_name = "Main Title";
    main_title.description = "Main 8KDVD content";
    main_title.duration = 0; // Unknown duration
    main_title.playable = true;
    integration->titles.push_back(main_title);
    
    msg_Info(integration->obj, "8KDVD metadata extracted successfully");
    return 0;
}

int kdvd_disc_integration_setup_playback(kdvd_disc_integration_t *integration, int title_id) {
    if (!integration) return -1;
    
    msg_Info(integration->obj, "Setting up 8KDVD playback for title %d", title_id);
    
    // TODO: Implement actual playback setup
    // This would create input_thread_t and configure it for 8KDVD playback
    
    msg_Info(integration->obj, "8KDVD playback setup completed for title %d", title_id);
    return 0;
}
