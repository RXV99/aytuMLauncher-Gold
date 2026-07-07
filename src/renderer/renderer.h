#ifndef RENDERER_H
#define RENDERER_H

#include "vitajme.h"
#include <vita2d.h>

/**
 * Initializes the renderer, allocates the virtual framebuffer for the MIDlet,
 * and loads system fonts.
 * 
 * @param res The resolution preset to use for the virtual framebuffer.
 */
void renderer_init(JmeResolution res);

/**
 * Cleans up renderer resources, frees the framebuffer and fonts.
 */
void renderer_fini(void);

/**
 * Returns a pointer to the raw pixels of the virtual framebuffer.
 * This is exposed so the javacall_lcd module can write MIDP graphics directly to it.
 * The format is RGBA8888 (0xAABBGGRR).
 */
void* renderer_get_framebuffer(void);

/**
 * Returns the width of the virtual framebuffer in pixels.
 */
int renderer_get_width(void);

/**
 * Returns the height of the virtual framebuffer in pixels.
 */
int renderer_get_height(void);

/**
 * Call this every frame to draw the virtual framebuffer to the screen,
 * applying the current scaling mode (letterbox, stretch, etc.).
 * Note: Must be called between vita2d_start_drawing() and vita2d_end_drawing().
 */
void renderer_draw_midlet(void);

/**
 * Draws text using the Vita system font (PGF). Useful for the launcher UI.
 */
void renderer_draw_text(int x, int y, unsigned int color, float scale, const char *text);

#endif // RENDERER_H
