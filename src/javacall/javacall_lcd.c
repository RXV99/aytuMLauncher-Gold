#include "../renderer/renderer.h"

// -----------------------------------------------------------------------------
// Javacall LCD HAL (mapping phoneME to Vita)
// These functions match the phoneME javacall_lcd interface.
// -----------------------------------------------------------------------------

typedef unsigned int javacall_result;
#define JAVACALL_OK 0

javacall_result javacall_lcd_init(void) {
    // We already initialized vita2d and the virtual framebuffer in renderer_init()
    return JAVACALL_OK;
}

javacall_result javacall_lcd_finalize(void) {
    return JAVACALL_OK;
}

javacall_result javacall_lcd_get_display_width(int *width) {
    if (width) *width = renderer_get_width();
    return JAVACALL_OK;
}

javacall_result javacall_lcd_get_display_height(int *height) {
    if (height) *height = renderer_get_height();
    return JAVACALL_OK;
}

javacall_result javacall_lcd_get_framebuffer(void **framebuffer) {
    if (framebuffer) *framebuffer = renderer_get_framebuffer();
    return JAVACALL_OK;
}

// Called by phoneME's GameCanvas.flushGraphics() or LCDUI repaint mechanism
javacall_result javacall_lcd_flush(int x, int y, int width, int height) {
    // The renderer loop in main.c calls renderer_draw_midlet() every frame
    // so we don't need to do synchronous flushes here, but we could set a dirty flag.
    return JAVACALL_OK;
}
