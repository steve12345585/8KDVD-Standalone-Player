/**
 * 8KDVD Detection Test
 * 
 * Tests detection on the actual "A Town Solves A Problem 8KDVD" disc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

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
 * Check if a file exists
 */
bool FileExists(const char* path) {
#ifdef _WIN32
    return _access(path, 0) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

/**
 * Check if a directory exists
 */
bool DirectoryExists(const char* path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

/**
 * Test 8KDVD detection on a specific path
 */
int Test8KDVDDetection(const char* disc_path) {
    char cert_path[512];
    char ts_path[512];
    char adv_obj_path[512];
    char stream_path[512];
    char playlist_path[512];
    char clipinf_path[512];
    char launcher_path[512];
    char licenseinfo_path[512];
    
    printf("🔍 Testing 8KDVD detection on: %s\n", disc_path);
    printf("==========================================\n");
    
    // Check for CERTIFICATE/Certificate.html
    snprintf(cert_path, sizeof(cert_path), "%s/%s/%s", disc_path, CERTIFICATE_FOLDER, CERTIFICATE_FILE);
    if (!FileExists(cert_path)) {
        printf("✗ CERTIFICATE/Certificate.html not found\n");
        return 0;
    }
    printf("✓ CERTIFICATE/Certificate.html found\n");
    
    // Check for 8KDVD_TS folder
    snprintf(ts_path, sizeof(ts_path), "%s/%s", disc_path, EIGHTKDVD_TS_FOLDER);
    if (!DirectoryExists(ts_path)) {
        printf("✗ 8KDVD_TS folder not found\n");
        return 0;
    }
    printf("✓ 8KDVD_TS folder found\n");
    
    // Check for 8KDVD_TS/ADV_OBJ folder
    snprintf(adv_obj_path, sizeof(adv_obj_path), "%s/%s/%s", disc_path, EIGHTKDVD_TS_FOLDER, ADV_OBJ_FOLDER);
    if (!DirectoryExists(adv_obj_path)) {
        printf("✗ 8KDVD_TS/ADV_OBJ folder not found\n");
        return 0;
    }
    printf("✓ 8KDVD_TS/ADV_OBJ folder found\n");
    
    // Check for 8KDVD_TS/STREAM folder
    snprintf(stream_path, sizeof(stream_path), "%s/%s/%s", disc_path, EIGHTKDVD_TS_FOLDER, STREAM_FOLDER);
    if (!DirectoryExists(stream_path)) {
        printf("✗ 8KDVD_TS/STREAM folder not found\n");
        return 0;
    }
    printf("✓ 8KDVD_TS/STREAM folder found\n");
    
    // Check for 8KDVD_TS/PLAYLIST folder
    snprintf(playlist_path, sizeof(playlist_path), "%s/%s/%s", disc_path, EIGHTKDVD_TS_FOLDER, PLAYLIST_FOLDER);
    if (!DirectoryExists(playlist_path)) {
        printf("✗ 8KDVD_TS/PLAYLIST folder not found\n");
        return 0;
    }
    printf("✓ 8KDVD_TS/PLAYLIST folder found\n");
    
    // Check for 8KDVD_TS/CLIPINF folder
    snprintf(clipinf_path, sizeof(clipinf_path), "%s/%s/%s", disc_path, EIGHTKDVD_TS_FOLDER, CLIPINF_FOLDER);
    if (!DirectoryExists(clipinf_path)) {
        printf("✗ 8KDVD_TS/CLIPINF folder not found\n");
        return 0;
    }
    printf("✓ 8KDVD_TS/CLIPINF folder found\n");
    
    // Check for Universal_web_launcher.html
    snprintf(launcher_path, sizeof(launcher_path), "%s/%s", disc_path, UNIVERSAL_LAUNCHER);
    if (!FileExists(launcher_path)) {
        printf("✗ Universal_web_launcher.html not found\n");
        return 0;
    }
    printf("✓ Universal_web_launcher.html found\n");
    
    // Check for LICENSEINFO folder (optional)
    snprintf(licenseinfo_path, sizeof(licenseinfo_path), "%s/%s", disc_path, LICENSEINFO_FOLDER);
    if (DirectoryExists(licenseinfo_path)) {
        printf("✓ LICENSEINFO folder found (optional)\n");
    } else {
        printf("- LICENSEINFO folder not found (optional)\n");
    }
    
    printf("\n🎬 8KDVD disc detected successfully!\n");
    return 1;
}

int main(int argc, char* argv[]) {
    printf("🎬 8KDVD Detection Test\n");
    printf("=======================\n\n");
    
    if (argc < 2) {
        printf("Usage: %s <disc_path>\n", argv[0]);
        printf("Example: %s \"H:\\Dropbox\\8KFLIKZ\\8KDVD Player for windows\\Procect supporting\\A Town Solves A Problem 8KDVD\"\n", argv[0]);
        return 1;
    }
    
    const char* disc_path = argv[1];
    
    if (Test8KDVDDetection(disc_path)) {
        printf("\n✅ 8KDVD detection successful!\n");
        return 0;
    } else {
        printf("\n❌ 8KDVD detection failed!\n");
        return 1;
    }
}
