#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "vitajme.h"

/**
 * Scans the ux0:data/java directory for installed MIDlets.
 */
void launcher_scan_midlets(void);

/**
 * Renders the launcher UI using vita2d.
 */
void launcher_render(void);

/**
 * Handles input events while in the launcher state.
 */
void launcher_handle_input(uint32_t buttons_pressed);

#endif // LAUNCHER_H
