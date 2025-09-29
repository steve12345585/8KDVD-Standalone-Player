#ifndef VLC_CEF_WRAPPER_H
#define VLC_CEF_WRAPPER_H

#include <vlc_common.h>
#include <vlc_interface.h>
#include <vlc_window.h>

#ifdef __cplusplus
extern "C" {
#endif

// CEF Wrapper Module for VLC 8KDVD HTML5 Menu Integration
typedef struct cef_wrapper_t cef_wrapper_t;

// CEF Initialization and Management
cef_wrapper_t* cef_wrapper_create(vlc_object_t *obj);
void cef_wrapper_destroy(cef_wrapper_t *wrapper);
int cef_wrapper_initialize(cef_wrapper_t *wrapper);
void cef_wrapper_shutdown(cef_wrapper_t *wrapper);

// HTML5 Menu Functions
int cef_wrapper_load_menu(cef_wrapper_t *wrapper, const char *menu_url);
int cef_wrapper_load_8kdvd_menu(cef_wrapper_t *wrapper, const char *disc_path);

// Browser Navigation Functions
int cef_wrapper_navigate_back(cef_wrapper_t *wrapper);
int cef_wrapper_navigate_forward(cef_wrapper_t *wrapper);
int cef_wrapper_reload(cef_wrapper_t *wrapper);

// 8KDVD Integration Functions
int cef_wrapper_handle_menu_selection(cef_wrapper_t *wrapper, int selection_id);
void cef_wrapper_set_size(cef_wrapper_t *wrapper, int width, int height);

#ifdef __cplusplus
}
#endif

#endif // VLC_CEF_WRAPPER_H
