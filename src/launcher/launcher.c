#include "launcher.h"
#include "jad_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/ctrl.h>
#include <vita2d.h>

#define MAX_MIDLETS 100

typedef struct {
    char jar_path[256];
    JadInfo info;
} MidletEntry;

static MidletEntry g_midlets[MAX_MIDLETS];
static int g_midlet_count = 0;
static int g_selected_idx = 0;
static vita2d_pgf *g_font = NULL;

void launcher_scan_midlets(void) {
    g_midlet_count = 0;
    
    // Ensure directory exists
    sceIoMkdir(JAVA_DATA_ROOT, 0777);
    
    SceUID dfd = sceIoDopen(JAVA_DATA_ROOT);
    if (dfd < 0) {
        VJME_ERR("Failed to open %s", JAVA_DATA_ROOT);
        return;
    }
    
    SceIoDirent dir;
    memset(&dir, 0, sizeof(dir));
    
    while (sceIoDread(dfd, &dir) > 0 && g_midlet_count < MAX_MIDLETS) {
        if (!SCE_S_ISDIR(dir.d_stat.st_mode)) {
            char *ext = strrchr(dir.d_name, '.');
            if (ext && (strcmp(ext, ".jar") == 0 || strcmp(ext, ".JAR") == 0)) {
                MidletEntry *entry = &g_midlets[g_midlet_count];
                snprintf(entry->jar_path, sizeof(entry->jar_path), "%s%s", JAVA_DATA_ROOT, dir.d_name);
                
                // Set defaults
                snprintf(entry->info.name, sizeof(entry->info.name), "%s", dir.d_name);
                strcpy(entry->info.vendor, "Unknown");
                strcpy(entry->info.version, "1.0");
                
                // Try to find a matching .jad file
                char jad_path[256];
                snprintf(jad_path, sizeof(jad_path), "%s", entry->jar_path);
                char *jad_ext = strrchr(jad_path, '.');
                if (jad_ext) strcpy(jad_ext, ".jad");
                
                if (jad_parse(jad_path, true, &entry->info)) {
                    // Parsed JAD successfully
                } else {
                    // TODO: Extract MANIFEST.MF from JAR using jar_loader and parse it
                    // For now, it will just use the default filename as the name
                }
                
                g_midlet_count++;
            }
        }
        memset(&dir, 0, sizeof(dir));
    }
    
    sceIoDclose(dfd);
    VJME_LOG("Scanned %d MIDlets", g_midlet_count);
}

void launcher_render(void) {
    if (!g_font) {
        g_font = vita2d_load_default_pgf();
    }
    
    // Deep charcoal gray background
    vita2d_set_clear_color(RGBA8(28, 28, 30, 255));
    
    // Draw Aytum Launcher title (Dark Yellow/Mustard)
    vita2d_pgf_draw_text(g_font, 20, 40, RGBA8(204, 163, 0, 255), 1.5f, "Aytum Launcher");
    
    if (g_midlet_count == 0) {
        vita2d_pgf_draw_text(g_font, 20, 100, RGBA8(200, 200, 200, 255), 1.2f, "No .jar files found in ux0:data/java/");
        return;
    }
    
    // Draw list
    int start_y = 80;
    int item_height = 60;
    
    for (int i = 0; i < g_midlet_count; i++) {
        int y = start_y + (i * item_height);
        
        // Highlight selected item with dark yellow
        if (i == g_selected_idx) {
            vita2d_draw_rectangle(10, y - 20, 940, 30, RGBA8(204, 163, 0, 100));
        }
        
        // Draw text (White for unselected, bright yellow for selected)
        unsigned int color = (i == g_selected_idx) ? RGBA8(255, 215, 0, 255) : RGBA8(255, 255, 255, 255);
        MidletEntry *entry = &g_midlets[i];
        vita2d_pgf_draw_text(g_font, 20, y + 25, color, 1.2f, entry->info.name);
        
        char subtext[256];
        snprintf(subtext, sizeof(subtext), "%s | v%s", entry->info.vendor, entry->info.version);
        vita2d_pgf_draw_text(g_font, 20, y + 45, RGBA8(150, 150, 150, 255), 1.0f, subtext);
    }
    
    // Draw instructions
    vita2d_pgf_draw_text(g_font, 20, VITA_SCREEN_H - 20, RGBA8(200, 200, 200, 255), 1.0f, "Cross: Launch | Select: Settings");
}

void launcher_handle_input(uint32_t buttons_pressed) {
    if (g_midlet_count == 0) return;
    
    if (buttons_pressed & SCE_CTRL_UP) {
        g_selected_idx--;
        if (g_selected_idx < 0) g_selected_idx = g_midlet_count - 1;
    }
    if (buttons_pressed & SCE_CTRL_DOWN) {
        g_selected_idx++;
        if (g_selected_idx >= g_midlet_count) g_selected_idx = 0;
    }
    
    if (buttons_pressed & SCE_CTRL_CROSS) {
        // Launch selected MIDlet
        MidletEntry *entry = &g_midlets[g_selected_idx];
        
        strncpy(g_app.midlet_jar_path, entry->jar_path, sizeof(g_app.midlet_jar_path) - 1);
        strncpy(g_app.midlet_name, entry->info.name, sizeof(g_app.midlet_name) - 1);
        strncpy(g_app.midlet_vendor, entry->info.vendor, sizeof(g_app.midlet_vendor) - 1);
        strncpy(g_app.midlet_version, entry->info.version, sizeof(g_app.midlet_version) - 1);
        strncpy(g_app.midlet_main_class, entry->info.main_class, sizeof(g_app.midlet_main_class) - 1);
        
        VJME_LOG("Launching: %s", g_app.midlet_name);
        
        // Change state
        g_app.state = APP_STATE_RUNNING_MIDLET;
        
        // TODO: Init JVM and load JAR
    }
}
