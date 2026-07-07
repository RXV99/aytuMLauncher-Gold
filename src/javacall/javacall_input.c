#include "../input/input.h"
#include <psp2/ctrl.h>

// -----------------------------------------------------------------------------
// Javacall Input HAL (mapping SceCtrl to phoneME Canvas keys)
// -----------------------------------------------------------------------------

typedef unsigned int javacall_result;
#define JAVACALL_OK 0
#define JAVACALL_FAIL 1

// These functions would normally be provided by phoneME to inject events
extern void javacall_events_send_key(int key, int is_pressed);
extern void javacall_events_send_pointer(int x, int y, int is_pressed);

// This function will be called from input_poll() in input.c
void vitajme_dispatch_key_event(uint32_t vita_btn, bool pressed) {
    // Find mapping
    for (int i = 0; i < KEYMAP_COUNT; i++) {
        if (g_keymap[i].vita_btn == vita_btn) {
            // Found it, send to JVM
            javacall_events_send_key(g_keymap[i].midp_key, pressed);
            break;
        }
    }
}
