#include "xml_parser.h"
#include <vlc_messages.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

// XML Parser Implementation
struct xml_parser_t {
    vlc_object_t *obj;
    disc_info_t disc_info;
    std::vector<title_info_t> titles;
    std::vector<chapter_info_t> chapters;
    bool vlc_integration_enabled;
    bool debug_output_enabled;
};

xml_parser_t* xml_parser_create(vlc_object_t *obj) {
    xml_parser_t *parser = new xml_parser_t();
    if (!parser) return nullptr;
    
    parser->obj = obj;
    parser->vlc_integration_enabled = true;
    parser->debug_output_enabled = false;
    
    // Initialize disc info
    parser->disc_info.title.clear();
    parser->disc_info.version.clear();
    parser->disc_info.region.clear();
    parser->disc_info.language.clear();
    parser->disc_info.resolution.clear();
    parser->disc_info.frame_rate = 60;
    parser->disc_info.hdr_enabled = false;
    parser->disc_info.dolby_vision_enabled = false;
    
    msg_Info(obj, "XML parser created");
    return parser;
}

void xml_parser_destroy(xml_parser_t *parser) {
    if (!parser) return;
    
    parser->titles.clear();
    parser->chapters.clear();
    delete parser;
    msg_Info(parser->obj, "XML parser destroyed");
}

int xml_parser_parse_disc_info(xml_parser_t *parser, const char *xml_path) {
    if (!parser || !xml_path) return -1;
    
    msg_Info(parser->obj, "Parsing disc info XML: %s", xml_path);
    
    // Check if file exists
    if (!std::filesystem::exists(xml_path)) {
        msg_Err(parser->obj, "Disc info XML not found: %s", xml_path);
        return -1;
    }
    
    try {
        std::ifstream file(xml_path);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        // Parse XML content
        if (xml_parser_parse_disc_info_content(parser, content) != 0) {
            return -1;
        }
        
        msg_Info(parser->obj, "Disc info XML parsed successfully");
        return 0;
        
    } catch (const std::exception& e) {
        msg_Err(parser->obj, "Failed to parse disc info XML: %s", e.what());
        return -1;
    }
}

int xml_parser_parse_titles(xml_parser_t *parser, const char *xml_path) {
    if (!parser || !xml_path) return -1;
    
    msg_Info(parser->obj, "Parsing titles XML: %s", xml_path);
    
    // Check if file exists
    if (!std::filesystem::exists(xml_path)) {
        msg_Err(parser->obj, "Titles XML not found: %s", xml_path);
        return -1;
    }
    
    try {
        std::ifstream file(xml_path);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        // Parse XML content
        if (xml_parser_parse_titles_content(parser, content) != 0) {
            return -1;
        }
        
        msg_Info(parser->obj, "Titles XML parsed successfully (%zu titles)", parser->titles.size());
        return 0;
        
    } catch (const std::exception& e) {
        msg_Err(parser->obj, "Failed to parse titles XML: %s", e.what());
        return -1;
    }
}

int xml_parser_parse_chapters(xml_parser_t *parser, const char *xml_path) {
    if (!parser || !xml_path) return -1;
    
    msg_Info(parser->obj, "Parsing chapters XML: %s", xml_path);
    
    // Check if file exists
    if (!std::filesystem::exists(xml_path)) {
        msg_Err(parser->obj, "Chapters XML not found: %s", xml_path);
        return -1;
    }
    
    try {
        std::ifstream file(xml_path);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        // Parse XML content
        if (xml_parser_parse_chapters_content(parser, content) != 0) {
            return -1;
        }
        
        msg_Info(parser->obj, "Chapters XML parsed successfully (%zu chapters)", parser->chapters.size());
        return 0;
        
    } catch (const std::exception& e) {
        msg_Err(parser->obj, "Failed to parse chapters XML: %s", e.what());
        return -1;
    }
}

disc_info_t xml_parser_get_disc_info(xml_parser_t *parser) {
    if (parser) {
        return parser->disc_info;
    }
    
    disc_info_t empty_info = {0};
    return empty_info;
}

std::vector<title_info_t> xml_parser_get_titles(xml_parser_t *parser) {
    if (parser) {
        return parser->titles;
    }
    
    return std::vector<title_info_t>();
}

title_info_t xml_parser_get_title(xml_parser_t *parser, int title_id) {
    if (parser) {
        for (const auto &title : parser->titles) {
            if (title.id == title_id) {
                return title;
            }
        }
    }
    
    title_info_t empty_title = {0};
    return empty_title;
}

std::vector<chapter_info_t> xml_parser_get_chapters(xml_parser_t *parser) {
    if (parser) {
        return parser->chapters;
    }
    
    return std::vector<chapter_info_t>();
}

std::vector<chapter_info_t> xml_parser_get_chapters_for_title(xml_parser_t *parser, int title_id) {
    std::vector<chapter_info_t> title_chapters;
    
    if (parser) {
        for (const auto &chapter : parser->chapters) {
            if (chapter.title_id == title_id) {
                title_chapters.push_back(chapter);
            }
        }
    }
    
    return title_chapters;
}

std::string xml_parser_generate_menu_data(xml_parser_t *parser) {
    if (!parser) return "{}";
    
    std::ostringstream json;
    json << "{";
    json << "\"disc_info\":{";
    json << "\"title\":\"" << parser->disc_info.title << "\",";
    json << "\"version\":\"" << parser->disc_info.version << "\",";
    json << "\"region\":\"" << parser->disc_info.region << "\",";
    json << "\"language\":\"" << parser->disc_info.language << "\",";
    json << "\"resolution\":\"" << parser->disc_info.resolution << "\",";
    json << "\"frame_rate\":" << parser->disc_info.frame_rate << ",";
    json << "\"hdr_enabled\":" << (parser->disc_info.hdr_enabled ? "true" : "false") << ",";
    json << "\"dolby_vision_enabled\":" << (parser->disc_info.dolby_vision_enabled ? "true" : "false");
    json << "},";
    json << "\"titles\":[";
    
    for (size_t i = 0; i < parser->titles.size(); i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"id\":" << parser->titles[i].id << ",";
        json << "\"name\":\"" << parser->titles[i].name << "\",";
        json << "\"description\":\"" << parser->titles[i].description << "\",";
        json << "\"duration\":" << parser->titles[i].duration << ",";
        json << "\"thumbnail\":\"" << parser->titles[i].thumbnail << "\"";
        json << "}";
    }
    
    json << "]";
    json << "}";
    
    return json.str();
}

std::string xml_parser_generate_title_data(xml_parser_t *parser) {
    if (!parser) return "[]";
    
    std::ostringstream json;
    json << "[";
    
    for (size_t i = 0; i < parser->titles.size(); i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"id\":" << parser->titles[i].id << ",";
        json << "\"name\":\"" << parser->titles[i].name << "\",";
        json << "\"description\":\"" << parser->titles[i].description << "\",";
        json << "\"duration\":" << parser->titles[i].duration << ",";
        json << "\"thumbnail\":\"" << parser->titles[i].thumbnail << "\"";
        json << "}";
    }
    
    json << "]";
    return json.str();
}

std::string xml_parser_generate_chapter_data(xml_parser_t *parser) {
    if (!parser) return "[]";
    
    std::ostringstream json;
    json << "[";
    
    for (size_t i = 0; i < parser->chapters.size(); i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"id\":" << parser->chapters[i].id << ",";
        json << "\"title_id\":" << parser->chapters[i].title_id << ",";
        json << "\"name\":\"" << parser->chapters[i].name << "\",";
        json << "\"start_time\":" << parser->chapters[i].start_time << ",";
        json << "\"end_time\":" << parser->chapters[i].end_time << ",";
        json << "\"thumbnail\":\"" << parser->chapters[i].thumbnail << "\"";
        json << "}";
    }
    
    json << "]";
    return json.str();
}

void xml_parser_set_vlc_integration(xml_parser_t *parser, bool enable) {
    if (parser) {
        parser->vlc_integration_enabled = enable;
        msg_Info(parser->obj, "VLC integration %s", enable ? "enabled" : "disabled");
    }
}

void xml_parser_set_debug_output(xml_parser_t *parser, bool enable) {
    if (parser) {
        parser->debug_output_enabled = enable;
        msg_Info(parser->obj, "Debug output %s", enable ? "enabled" : "disabled");
    }
}

// Helper functions for XML parsing
int xml_parser_parse_disc_info_content(xml_parser_t *parser, const std::string &content) {
    // Simple XML parsing using regex (in production, use a proper XML parser)
    std::regex title_regex(R"(<title>(.*?)</title>)");
    std::regex version_regex(R"(<version>(.*?)</version>)");
    std::regex region_regex(R"(<region>(.*?)</region>)");
    std::regex language_regex(R"(<language>(.*?)</language>)");
    std::regex resolution_regex(R"(<resolution>(.*?)</resolution>)");
    std::regex frame_rate_regex(R"(<frame_rate>(.*?)</frame_rate>)");
    std::regex hdr_regex(R"(<hdr_enabled>(.*?)</hdr_enabled>)");
    std::regex dolby_regex(R"(<dolby_vision_enabled>(.*?)</dolby_vision_enabled>)");
    
    std::smatch match;
    
    if (std::regex_search(content, match, title_regex)) {
        parser->disc_info.title = match[1].str();
    }
    
    if (std::regex_search(content, match, version_regex)) {
        parser->disc_info.version = match[1].str();
    }
    
    if (std::regex_search(content, match, region_regex)) {
        parser->disc_info.region = match[1].str();
    }
    
    if (std::regex_search(content, match, language_regex)) {
        parser->disc_info.language = match[1].str();
    }
    
    if (std::regex_search(content, match, resolution_regex)) {
        parser->disc_info.resolution = match[1].str();
    }
    
    if (std::regex_search(content, match, frame_rate_regex)) {
        parser->disc_info.frame_rate = std::stoi(match[1].str());
    }
    
    if (std::regex_search(content, match, hdr_regex)) {
        parser->disc_info.hdr_enabled = (match[1].str() == "true");
    }
    
    if (std::regex_search(content, match, dolby_regex)) {
        parser->disc_info.dolby_vision_enabled = (match[1].str() == "true");
    }
    
    if (parser->debug_output_enabled) {
        msg_Dbg(parser->obj, "Parsed disc info: %s v%s %s %s %s %d FPS HDR:%s Dolby:%s",
               parser->disc_info.title.c_str(),
               parser->disc_info.version.c_str(),
               parser->disc_info.region.c_str(),
               parser->disc_info.language.c_str(),
               parser->disc_info.resolution.c_str(),
               parser->disc_info.frame_rate,
               parser->disc_info.hdr_enabled ? "true" : "false",
               parser->disc_info.dolby_vision_enabled ? "true" : "false");
    }
    
    return 0;
}

int xml_parser_parse_titles_content(xml_parser_t *parser, const std::string &content) {
    // Simple XML parsing for titles
    std::regex title_regex(R"(<title id="(\d+)">(.*?)</title>)");
    std::regex name_regex(R"(<name>(.*?)</name>)");
    std::regex description_regex(R"(<description>(.*?)</description>)");
    std::regex duration_regex(R"(<duration>(.*?)</duration>)");
    std::regex thumbnail_regex(R"(<thumbnail>(.*?)</thumbnail>)");
    
    std::sregex_iterator iter(content.begin(), content.end(), title_regex);
    std::sregex_iterator end;
    
    parser->titles.clear();
    
    for (; iter != end; ++iter) {
        title_info_t title;
        title.id = std::stoi((*iter)[1].str());
        
        std::string title_content = (*iter)[2].str();
        
        std::smatch match;
        if (std::regex_search(title_content, match, name_regex)) {
            title.name = match[1].str();
        }
        if (std::regex_search(title_content, match, description_regex)) {
            title.description = match[1].str();
        }
        if (std::regex_search(title_content, match, duration_regex)) {
            title.duration = std::stoi(match[1].str());
        }
        if (std::regex_search(title_content, match, thumbnail_regex)) {
            title.thumbnail = match[1].str();
        }
        
        parser->titles.push_back(title);
        
        if (parser->debug_output_enabled) {
            msg_Dbg(parser->obj, "Parsed title %d: %s (%d seconds)",
                   title.id, title.name.c_str(), title.duration);
        }
    }
    
    return 0;
}

int xml_parser_parse_chapters_content(xml_parser_t *parser, const std::string &content) {
    // Simple XML parsing for chapters
    std::regex chapter_regex(R"(<chapter id="(\d+)" title_id="(\d+)">(.*?)</chapter>)");
    std::regex name_regex(R"(<name>(.*?)</name>)");
    std::regex start_time_regex(R"(<start_time>(.*?)</start_time>)");
    std::regex end_time_regex(R"(<end_time>(.*?)</end_time>)");
    std::regex thumbnail_regex(R"(<thumbnail>(.*?)</thumbnail>)");
    
    std::sregex_iterator iter(content.begin(), content.end(), chapter_regex);
    std::sregex_iterator end;
    
    parser->chapters.clear();
    
    for (; iter != end; ++iter) {
        chapter_info_t chapter;
        chapter.id = std::stoi((*iter)[1].str());
        chapter.title_id = std::stoi((*iter)[2].str());
        
        std::string chapter_content = (*iter)[3].str();
        
        std::smatch match;
        if (std::regex_search(chapter_content, match, name_regex)) {
            chapter.name = match[1].str();
        }
        if (std::regex_search(chapter_content, match, start_time_regex)) {
            chapter.start_time = std::stoi(match[1].str());
        }
        if (std::regex_search(chapter_content, match, end_time_regex)) {
            chapter.end_time = std::stoi(match[1].str());
        }
        if (std::regex_search(chapter_content, match, thumbnail_regex)) {
            chapter.thumbnail = match[1].str();
        }
        
        parser->chapters.push_back(chapter);
        
        if (parser->debug_output_enabled) {
            msg_Dbg(parser->obj, "Parsed chapter %d for title %d: %s (%d-%d seconds)",
                   chapter.id, chapter.title_id, chapter.name.c_str(),
                   chapter.start_time, chapter.end_time);
        }
    }
    
    return 0;
}
