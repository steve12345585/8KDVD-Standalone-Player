/**
 * 8KDVD Disc Detection Module
 * 
 * Detects actual 8KDVD discs based on real disc structure:
 * - CERTIFICATE/Certificate.html
 * - 8KDVD_TS/ folder with ADV_OBJ/, STREAM/, PLAYLIST/, CLIPINF/
 * - LICENSEINFO/ folder (optional)
 * - Universal_web_launcher.html
 * - Launch_8KDVD_Windows.bat
 */

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_stream.h>
#include <vlc_input.h>
#include <vlc_fs.h>
#include <vlc_url.h>

#define CERTIFICATE_FOLDER "CERTIFICATE"
#define CERTIFICATE_FILE "Certificate.html"
#define EIGHTKDVD_TS_FOLDER "8KDVD_TS"
#define ADV_OBJ_FOLDER "ADV_OBJ"
#define STREAM_FOLDER "STREAM"
#define PLAYLIST_FOLDER "PLAYLIST"
#define CLIPINF_FOLDER "CLIPINF"
#define LICENSEINFO_FOLDER "LICENSEINFO"
#define UNIVERSAL_LAUNCHER "Universal_web_launcher.html"
#define WINDOWS_LAUNCHER "Launch_8KDVD_Windows.bat"

/**
 * Check if a file exists on the disc
 */
static bool FileExists(const char* path)
{
    return vlc_stat(path, NULL) == 0;
}

/**
 * Check if a directory exists on the disc
 */
static bool DirectoryExists(const char* path)
{
    struct stat st;
    return vlc_stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/**
 * Probe for 8KDVD disc structure
 */
static int Probe8KDVD(stream_t *p_stream)
{
    char *psz_path = NULL;
    char *psz_cert_path = NULL;
    char *psz_ts_path = NULL;
    char *psz_adv_obj_path = NULL;
    char *psz_stream_path = NULL;
    char *psz_playlist_path = NULL;
    char *psz_clipinf_path = NULL;
    char *psz_launcher_path = NULL;
    int i_result = VLC_EGENERIC;
    
    if (!p_stream || !p_stream->psz_url)
        return VLC_EGENERIC;
    
    msg_Dbg(p_stream, "Probing for 8KDVD disc at: %s", p_stream->psz_url);
    
    // Get the base path
    psz_path = vlc_uri2path(p_stream->psz_url);
    if (!psz_path)
        return VLC_EGENERIC;
    
    // Check for CERTIFICATE/Certificate.html
    if (asprintf(&psz_cert_path, "%s/%s/%s", psz_path, CERTIFICATE_FOLDER, CERTIFICATE_FILE) == -1)
        goto cleanup;
    
    if (!FileExists(psz_cert_path)) {
        msg_Dbg(p_stream, "CERTIFICATE/Certificate.html not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ CERTIFICATE/Certificate.html found");
    
    // Check for 8KDVD_TS folder
    if (asprintf(&psz_ts_path, "%s/%s", psz_path, EIGHTKDVD_TS_FOLDER) == -1)
        goto cleanup;
    
    if (!DirectoryExists(psz_ts_path)) {
        msg_Dbg(p_stream, "8KDVD_TS folder not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ 8KDVD_TS folder found");
    
    // Check for 8KDVD_TS/ADV_OBJ folder
    if (asprintf(&psz_adv_obj_path, "%s/%s/%s", psz_path, EIGHTKDVD_TS_FOLDER, ADV_OBJ_FOLDER) == -1)
        goto cleanup;
    
    if (!DirectoryExists(psz_adv_obj_path)) {
        msg_Dbg(p_stream, "8KDVD_TS/ADV_OBJ folder not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ 8KDVD_TS/ADV_OBJ folder found");
    
    // Check for 8KDVD_TS/STREAM folder
    if (asprintf(&psz_stream_path, "%s/%s/%s", psz_path, EIGHTKDVD_TS_FOLDER, STREAM_FOLDER) == -1)
        goto cleanup;
    
    if (!DirectoryExists(psz_stream_path)) {
        msg_Dbg(p_stream, "8KDVD_TS/STREAM folder not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ 8KDVD_TS/STREAM folder found");
    
    // Check for 8KDVD_TS/PLAYLIST folder
    if (asprintf(&psz_playlist_path, "%s/%s/%s", psz_path, EIGHTKDVD_TS_FOLDER, PLAYLIST_FOLDER) == -1)
        goto cleanup;
    
    if (!DirectoryExists(psz_playlist_path)) {
        msg_Dbg(p_stream, "8KDVD_TS/PLAYLIST folder not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ 8KDVD_TS/PLAYLIST folder found");
    
    // Check for 8KDVD_TS/CLIPINF folder
    if (asprintf(&psz_clipinf_path, "%s/%s/%s", psz_path, EIGHTKDVD_TS_FOLDER, CLIPINF_FOLDER) == -1)
        goto cleanup;
    
    if (!DirectoryExists(psz_clipinf_path)) {
        msg_Dbg(p_stream, "8KDVD_TS/CLIPINF folder not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ 8KDVD_TS/CLIPINF folder found");
    
    // Check for Universal_web_launcher.html
    if (asprintf(&psz_launcher_path, "%s/%s", psz_path, UNIVERSAL_LAUNCHER) == -1)
        goto cleanup;
    
    if (!FileExists(psz_launcher_path)) {
        msg_Dbg(p_stream, "Universal_web_launcher.html not found - not an 8KDVD disc");
        goto cleanup;
    }
    msg_Dbg(p_stream, "✓ Universal_web_launcher.html found");
    
    msg_Info(p_stream, "8KDVD disc detected successfully");
    i_result = VLC_SUCCESS;
    
cleanup:
    free(psz_path);
    free(psz_cert_path);
    free(psz_ts_path);
    free(psz_adv_obj_path);
    free(psz_stream_path);
    free(psz_playlist_path);
    free(psz_clipinf_path);
    free(psz_launcher_path);
    
    return i_result;
}

/**
 * Initialize 8KDVD disc detection
 */
static int Open8KDVD(input_thread_t *p_input)
{
    stream_t *p_stream = p_input->p->input.p_sys->p_access;
    
    if (Probe8KDVD(p_stream) != VLC_SUCCESS) {
        msg_Err(p_input, "Failed to detect 8KDVD disc");
        return VLC_EGENERIC;
    }
    
    msg_Info(p_input, "8KDVD disc initialized successfully");
    return VLC_SUCCESS;
}

/**
 * VLC module descriptor
 */
vlc_module_begin()
    set_shortname("8KDVD")
    set_description("8KDVD Disc Detection")
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_ACCESS)
    set_capability("access", 0)
    set_callbacks(Open8KDVD, NULL)
vlc_module_end()