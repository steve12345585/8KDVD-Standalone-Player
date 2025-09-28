/**
 * 8KDVD Container Parser Test
 * 
 * Tests the container parser on the actual "A Town Solves A Problem 8KDVD" disc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

// Include our container parser functions
#include "modules/demux/8kdvd/container_parser.c"

/**
 * Test 8KDVD container parser
 */
int Test8KDVDContainerParser(const char* disc_path) {
    eightkdvd_container_t container;
    int i_result = VLC_EGENERIC;
    
    printf("🔍 Testing 8KDVD Container Parser\n");
    printf("================================\n");
    printf("Disc Path: %s\n\n", disc_path);
    
    // Initialize container parser
    if (Init8KDVDContainer(&container, disc_path) != VLC_SUCCESS) {
        printf("❌ Failed to initialize 8KDVD container parser\n");
        return 0;
    }
    
    printf("✅ 8KDVD container parser initialized successfully\n\n");
    
    // Get available streams
    eightkdvd_stream_t *p_streams = NULL;
    int i_stream_count = 0;
    
    if (GetAvailableStreams(&container, &p_streams, &i_stream_count) == VLC_SUCCESS) {
        printf("📺 Found %d available streams:\n", i_stream_count);
        printf("================================\n");
        
        for (int i = 0; i < i_stream_count; i++) {
            eightkdvd_stream_t *p_stream = &p_streams[i];
            printf("Stream %d:\n", i + 1);
            printf("  Format: %s\n", GetFormatName(p_stream->format));
            printf("  Filename: %s\n", p_stream->psz_filename);
            printf("  Resolution: %dx%d\n", p_stream->i_width, p_stream->i_height);
            printf("  Bandwidth: %d bps\n", p_stream->i_bandwidth);
            printf("  Available: %s\n", p_stream->b_available ? "Yes" : "No");
            printf("\n");
        }
        
        // Test stream selection
        printf("🎯 Testing stream selection:\n");
        printf("============================\n");
        
        // Try to select 8K stream
        if (SelectStream(&container, EIGHTKDVD_FORMAT_EVO8) == VLC_SUCCESS) {
            eightkdvd_stream_t *p_current = GetCurrentStream(&container);
            if (p_current) {
                printf("✅ Selected 8K stream: %s\n", p_current->psz_filename);
            }
        } else {
            printf("❌ Failed to select 8K stream\n");
        }
        
        // Try to select 4K stream
        if (SelectStream(&container, EIGHTKDVD_FORMAT_EVO4) == VLC_SUCCESS) {
            eightkdvd_stream_t *p_current = GetCurrentStream(&container);
            if (p_current) {
                printf("✅ Selected 4K stream: %s\n", p_current->psz_filename);
            }
        } else {
            printf("❌ Failed to select 4K stream\n");
        }
        
        // Try to select HD stream
        if (SelectStream(&container, EIGHTKDVD_FORMAT_EVOH) == VLC_SUCCESS) {
            eightkdvd_stream_t *p_current = GetCurrentStream(&container);
            if (p_current) {
                printf("✅ Selected HD stream: %s\n", p_current->psz_filename);
            }
        } else {
            printf("❌ Failed to select HD stream\n");
        }
        
        // Try to select 3D stream
        if (SelectStream(&container, EIGHTKDVD_FORMAT_3D4) == VLC_SUCCESS) {
            eightkdvd_stream_t *p_current = GetCurrentStream(&container);
            if (p_current) {
                printf("✅ Selected 3D stream: %s\n", p_current->psz_filename);
            }
        } else {
            printf("❌ Failed to select 3D stream\n");
        }
        
    } else {
        printf("❌ Failed to get available streams\n");
    }
    
    // Cleanup
    Cleanup8KDVDContainer(&container);
    
    printf("\n🎬 8KDVD Container Parser Test Complete!\n");
    return 1;
}

int main(int argc, char* argv[]) {
    printf("🎬 8KDVD Container Parser Test\n");
    printf("==============================\n\n");
    
    if (argc < 2) {
        printf("Usage: %s <disc_path>\n", argv[0]);
        printf("Example: %s \"H:\\Dropbox\\8KFLIKZ\\8KDVD Player for windows\\Procect supporting\\A Town Solves A Problem 8KDVD\"\n", argv[0]);
        return 1;
    }
    
    const char* disc_path = argv[1];
    
    if (Test8KDVDContainerParser(disc_path)) {
        printf("\n✅ 8KDVD Container Parser test successful!\n");
        return 0;
    } else {
        printf("\n❌ 8KDVD Container Parser test failed!\n");
        return 1;
    }
}
