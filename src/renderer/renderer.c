#include "renderer.h"
#include <string.h>

static vita2d_texture *g_midlet_fb = NULL;
static vita2d_pgf *g_sys_font = NULL;
static int g_vw = 0;
static int g_vh = 0;

void renderer_init(JmeResolution res) {
    if (g_midlet_fb) {
        vita2d_free_texture(g_midlet_fb);
        g_midlet_fb = NULL;
    }
    
    if (res < 0 || res >= RES_COUNT) res = RES_240x320;
    
    g_vw = g_resolutions[res].width;
    g_vh = g_resolutions[res].height;
    
    // Create uncompressed texture (RGBA8888) for the MIDlet to render into
    g_midlet_fb = vita2d_create_empty_texture_format(g_vw, g_vh, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR);
    
    if (!g_sys_font) {
        g_sys_font = vita2d_load_default_pgf();
    }
    
    VJME_LOG("Renderer initialized virtual framebuffer: %dx%d", g_vw, g_vh);
}

void renderer_fini(void) {
    if (g_midlet_fb) {
        vita2d_free_texture(g_midlet_fb);
        g_midlet_fb = NULL;
    }
    if (g_sys_font) {
        vita2d_free_pgf(g_sys_font);
        g_sys_font = NULL;
    }
}

void* renderer_get_framebuffer(void) {
    if (!g_midlet_fb) return NULL;
    return vita2d_texture_get_datap(g_midlet_fb);
}

int renderer_get_width(void) {
    return g_vw;
}

int renderer_get_height(void) {
    return g_vh;
}

void renderer_draw_midlet(void) {
    if (!g_midlet_fb) return;
    
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    int x_off = 0;
    int y_off = 0;
    
    switch (g_app.scale_mode) {
        case SCALE_FIT_LETTERBOX:
        {
            // Calculate best fit scale preserving aspect ratio
            float aspect_screen = (float)VITA_SCREEN_W / (float)VITA_SCREEN_H;
            float aspect_midlet = (float)g_vw / (float)g_vh;
            
            if (aspect_screen > aspect_midlet) {
                // Height is the limiting factor
                scale_y = (float)VITA_SCREEN_H / (float)g_vh;
                scale_x = scale_y;
            } else {
                // Width is the limiting factor
                scale_x = (float)VITA_SCREEN_W / (float)g_vw;
                scale_y = scale_x;
            }
            
            // Center the output
            x_off = (VITA_SCREEN_W - (int)(g_vw * scale_x)) / 2;
            y_off = (VITA_SCREEN_H - (int)(g_vh * scale_y)) / 2;
            
            // Optional: Enable linear filtering for non-integer scaling
            vita2d_texture_set_filters(g_midlet_fb, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
            break;
        }
        case SCALE_STRETCH:
        {
            scale_x = (float)VITA_SCREEN_W / (float)g_vw;
            scale_y = (float)VITA_SCREEN_H / (float)g_vh;
            x_off = 0;
            y_off = 0;
            vita2d_texture_set_filters(g_midlet_fb, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
            break;
        }
        case SCALE_INTEGER_2X:
        {
            scale_x = 2.0f;
            scale_y = 2.0f;
            x_off = (VITA_SCREEN_W - (int)(g_vw * scale_x)) / 2;
            y_off = (VITA_SCREEN_H - (int)(g_vh * scale_y)) / 2;
            vita2d_texture_set_filters(g_midlet_fb, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
            break;
        }
        case SCALE_INTEGER_MAX:
        {
            int scale = VITA_SCREEN_H / g_vh;
            int scale_w = VITA_SCREEN_W / g_vw;
            if (scale_w < scale) scale = scale_w;
            if (scale < 1) scale = 1; // Fallback
            
            scale_x = (float)scale;
            scale_y = (float)scale;
            x_off = (VITA_SCREEN_W - (int)(g_vw * scale_x)) / 2;
            y_off = (VITA_SCREEN_H - (int)(g_vh * scale_y)) / 2;
            vita2d_texture_set_filters(g_midlet_fb, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
            break;
        }
    }
    
    // Draw the scaled texture
    vita2d_draw_texture_scale(g_midlet_fb, x_off, y_off, scale_x, scale_y);
}

void renderer_draw_text(int x, int y, unsigned int color, float scale, const char *text) {
    if (g_sys_font) {
        vita2d_pgf_draw_text(g_sys_font, x, y, color, scale, text);
    }
}
