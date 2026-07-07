#include "input.h"
#include <string.h>

static SceCtrlData g_pad;
static SceCtrlData g_old_pad;
// Touch might be added later

void input_init(void) {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    memset(&g_pad, 0, sizeof(g_pad));
    memset(&g_old_pad, 0, sizeof(g_old_pad));
}

// Simple edge detection helper
static bool btn_pressed(uint32_t btn) {
    return (g_pad.buttons & btn) && !(g_old_pad.buttons & btn);
}

static bool btn_released(uint32_t btn) {
    return !(g_pad.buttons & btn) && (g_old_pad.buttons & btn);
}

void input_poll(void) {
    g_old_pad = g_pad;
    sceCtrlReadBufferPositive(0, &g_pad, 1);
    
    // TODO: Map to phoneME input events
    // This will involve calling functions like javacall_events_send_key()
    // We will hook this up when javacall_input is implemented.
    
    // For now, we only need basic input for the launcher, which we handled in main.c/launcher.c
}
