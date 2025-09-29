#include "cef_wrapper.h"
#include "cef_process_handler.h"
#include "cef_client.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"

struct cef_wrapper_t {
    vlc_object_t *obj;
    CefRefPtr<CefBrowser> browser;
    CefRefPtr<VLCCefClient> client;
    bool initialized;
};

extern "C" {

cef_wrapper_t* cef_wrapper_create(vlc_object_t *obj) {
    cef_wrapper_t *wrapper = (cef_wrapper_t*)malloc(sizeof(cef_wrapper_t));
    if (!wrapper) return NULL;
    
    wrapper->obj = obj;
    wrapper->initialized = false;
    return wrapper;
}

void cef_wrapper_destroy(cef_wrapper_t *wrapper) {
    if (wrapper) {
        if (wrapper->initialized) {
            cef_wrapper_shutdown(wrapper);
        }
        free(wrapper);
    }
}

int cef_wrapper_initialize(cef_wrapper_t *wrapper) {
    if (!wrapper || wrapper->initialized) return -1;

    CefMainArgs main_args(GetModuleHandle(NULL));
    CefRefPtr<VLCCefApp> app = new VLCCefApp();

    CefSettings settings;
    settings.no_sandbox = true;
    settings.multi_threaded_message_loop = true;
    
    // Configure CEF process spawning for 8KDVD
    settings.log_severity = LOGSEVERITY_INFO;
    settings.log_file = "cef_debug.log";
    settings.user_agent = "8KDVD-Player/1.0 CEF/140.1.14";
    
    // Set up process spawning
    CefString(&settings.browser_subprocess_path).FromASCII("cef_subprocess.exe");
    CefString(&settings.resources_dir_path).FromASCII("./resources");
    CefString(&settings.locales_dir_path).FromASCII("./locales");
    
    // Configure for 8KDVD rendering
    settings.windowless_rendering_enabled = true;
    settings.background_color = 0xFF000000; // Black background for 8K
    
    if (!CefInitialize(main_args, settings, app.get(), nullptr)) {
        msg_Err(wrapper->obj, "Failed to initialize CEF");
        return -1;
    }

    // Create CEF client
    wrapper->client = new VLCCefClient(wrapper->obj);
    wrapper->initialized = true;
    
    msg_Info(wrapper->obj, "CEF initialized successfully for 8KDVD");
    return 0;
}

void cef_wrapper_shutdown(cef_wrapper_t *wrapper) {
    if (wrapper && wrapper->initialized) {
        if (wrapper->browser) {
            wrapper->browser->GetHost()->CloseBrowser(true);
            wrapper->browser = nullptr;
        }
        CefShutdown();
        wrapper->initialized = false;
    }
}

int cef_wrapper_load_menu(cef_wrapper_t *wrapper, const char *menu_url) {
    if (!wrapper || !wrapper->initialized || !wrapper->client || !menu_url) return -1;
    
    // Create browser window for HTML5 menu (offscreen rendering for VLC integration)
    CefWindowInfo window_info;
    window_info.SetAsWindowless(nullptr);
    
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = 60; // 60 FPS for smooth 8K menu rendering
    
    wrapper->browser = CefBrowserHost::CreateBrowserSync(
        window_info, wrapper->client.get(), menu_url, browser_settings, nullptr, nullptr);
    
    return wrapper->browser ? 0 : -1;
}

int cef_wrapper_load_8kdvd_menu(cef_wrapper_t *wrapper, const char *disc_path) {
    if (!wrapper || !disc_path) return -1;
    
    // Load 8KDVD-specific HTML5 menu
    char menu_path[512];
    snprintf(menu_path, sizeof(menu_path), "file://%s/8KDVD_TS/index.html", disc_path);
    
    return cef_wrapper_load_menu(wrapper, menu_path);
}

// Browser navigation functions
int cef_wrapper_navigate_back(cef_wrapper_t *wrapper) {
    if (!wrapper || !wrapper->browser) return -1;
    
    if (wrapper->browser->CanGoBack()) {
        wrapper->browser->GoBack();
        msg_Info(wrapper->obj, "CEF browser: Navigate back");
        return 0;
    }
    return -1;
}

int cef_wrapper_navigate_forward(cef_wrapper_t *wrapper) {
    if (!wrapper || !wrapper->browser) return -1;
    
    if (wrapper->browser->CanGoForward()) {
        wrapper->browser->GoForward();
        msg_Info(wrapper->obj, "CEF browser: Navigate forward");
        return 0;
    }
    return -1;
}

int cef_wrapper_reload(cef_wrapper_t *wrapper) {
    if (!wrapper || !wrapper->browser) return -1;
    
    wrapper->browser->Reload();
    msg_Info(wrapper->obj, "CEF browser: Reload");
    return 0;
}

} // extern "C"
