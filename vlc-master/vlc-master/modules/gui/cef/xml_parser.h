#ifndef VLC_8KDVD_XML_PARSER_H
#define VLC_8KDVD_XML_PARSER_H

#include <vlc_common.h>
#include <string>
#include <vector>
#include <map>

// XML Parser for 8KDVD Disc Metadata
typedef struct xml_parser_t xml_parser_t;

// Disc Information Structure
typedef struct disc_info_t {
    std::string title;
    std::string version;
    std::string region;
    std::string language;
    std::string resolution;
    int frame_rate;
    bool hdr_enabled;
    bool dolby_vision_enabled;
    std::vector<std::string> audio_tracks;
    std::vector<std::string> subtitle_tracks;
} disc_info_t;

// Title Information Structure
typedef struct title_info_t {
    int id;
    std::string name;
    std::string description;
    int duration;
    std::string thumbnail;
    std::vector<std::string> chapters;
} title_info_t;

// Chapter Information Structure
typedef struct chapter_info_t {
    int id;
    int title_id;
    std::string name;
    int start_time;
    int end_time;
    std::string thumbnail;
} chapter_info_t;

// XML Parser Functions
xml_parser_t* xml_parser_create(vlc_object_t *obj);
void xml_parser_destroy(xml_parser_t *parser);

// Disc Information Parsing
int xml_parser_parse_disc_info(xml_parser_t *parser, const char *xml_path);
disc_info_t xml_parser_get_disc_info(xml_parser_t *parser);

// Title Information Parsing
int xml_parser_parse_titles(xml_parser_t *parser, const char *xml_path);
std::vector<title_info_t> xml_parser_get_titles(xml_parser_t *parser);
title_info_t xml_parser_get_title(xml_parser_t *parser, int title_id);

// Chapter Information Parsing
int xml_parser_parse_chapters(xml_parser_t *parser, const char *xml_path);
std::vector<chapter_info_t> xml_parser_get_chapters(xml_parser_t *parser);
std::vector<chapter_info_t> xml_parser_get_chapters_for_title(xml_parser_t *parser, int title_id);

// Menu Data Generation
std::string xml_parser_generate_menu_data(xml_parser_t *parser);
std::string xml_parser_generate_title_data(xml_parser_t *parser);
std::string xml_parser_generate_chapter_data(xml_parser_t *parser);

// VLC Integration
void xml_parser_set_vlc_integration(xml_parser_t *parser, bool enable);
void xml_parser_set_debug_output(xml_parser_t *parser, bool enable);

#endif // VLC_8KDVD_XML_PARSER_H
