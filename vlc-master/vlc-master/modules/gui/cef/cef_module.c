#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>

#include "cef_wrapper.h"

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int  Open(vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_shortname("CEF")
    set_description("Chromium Embedded Framework for HTML5 Menus")
    set_category(CAT_INTERFACE)
    set_subcategory(SUBCAT_INTERFACE_MAIN)
    set_capability("interface", 0)
    set_callbacks(Open, Close)
vlc_module_end()

/*****************************************************************************
 * Interface structure
 *****************************************************************************/
typedef struct {
    cef_wrapper_t *cef;
} intf_sys_t;

/*****************************************************************************
 * Open: initialize and create interface
 *****************************************************************************/
static int Open(vlc_object_t *p_this) {
    intf_thread_t *p_intf = (intf_thread_t *)p_this;
    intf_sys_t *p_sys;
    
    p_sys = malloc(sizeof(intf_sys_t));
    if (!p_sys)
        return VLC_ENOMEM;
        
    p_intf->p_sys = p_sys;
    
    // Initialize CEF wrapper
    p_sys->cef = cef_wrapper_create(p_this);
    if (!p_sys->cef) {
        free(p_sys);
        return VLC_EGENERIC;
    }
    
    if (cef_wrapper_initialize(p_sys->cef) != 0) {
        cef_wrapper_destroy(p_sys->cef);
        free(p_sys);
        return VLC_EGENERIC;
    }
    
    msg_Info(p_intf, "CEF interface initialized successfully");
    return VLC_SUCCESS;
}

/*****************************************************************************
 * Close: destroy interface
 *****************************************************************************/
static void Close(vlc_object_t *p_this) {
    intf_thread_t *p_intf = (intf_thread_t *)p_this;
    intf_sys_t *p_sys = p_intf->p_sys;
    
    if (p_sys) {
        if (p_sys->cef) {
            cef_wrapper_destroy(p_sys->cef);
        }
        free(p_sys);
    }
    
    msg_Info(p_intf, "CEF interface closed");
}
