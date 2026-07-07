#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/power.h>
#include <vita2d.h>

#include "vitajme.h"

// Global app state
AppConfig g_app = {
    .state = APP_STATE_LAUNCHER,
    .resolution = RES_240x320,
    .scale_mode = SCALE_FIT_LETTERBOX,
    .heap_size_mb = 64,
    .stack_size_kb = 256,
    .audio_enabled = true,
    .audio_volume = 32768,
    .vsync_enabled = true,
    .show_fps = false,
    .error_msg = {0}
};

KeyMapping g_keymap[KEYMAP_COUNT] = {
    { SCE_CTRL_UP,       -1, "D-Pad Up" },
    { SCE_CTRL_DOWN,     -2, "D-Pad Down" },
    { SCE_CTRL_LEFT,     -3, "D-Pad Left" },
    { SCE_CTRL_RIGHT,    -4, "D-Pad Right" },
    { SCE_CTRL_CROSS,    -5, "Cross (Select/Fire)" }, // 5 = FIRE
    { SCE_CTRL_SQUARE,   -6, "Square (Soft 1)" },
    { SCE_CTRL_TRIANGLE, -7, "Triangle (Soft 2)" },
    { SCE_CTRL_CIRCLE,   -8, "Circle (Clear/Back)" },
    { SCE_CTRL_L1,       -9, "L (Game A)" },
    { SCE_CTRL_R1,      -10, "R (Game B)" },
    { SCE_CTRL_START,   -11, "Start" },
    { SCE_CTRL_SELECT,  -12, "Select" }
};

int main(int argc, char *argv[]) {
    // Maximize performance for JVM
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);

    // Initialize rendering
    vita2d_init();
    vita2d_set_clear_color(RGBA8(40, 40, 40, 255)); // Dark grey background

    // Initialize input
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    // Load networking (needed later for MIDlet HTTP)
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);

    // TODO: Init launcher UI
    // TODO: Init JVM (phoneME)

    SceCtrlData pad;
    bool running = true;

    while (running) {
        sceCtrlReadBufferPositive(0, &pad, 1);

        if (pad.buttons & SCE_CTRL_START) {
            // Temp exit condition
            running = false;
        }

        vita2d_start_drawing();
        vita2d_clear_screen();

        if (g_app.state == APP_STATE_LAUNCHER) {
            // TODO: render launcher
        } else if (g_app.state == APP_STATE_RUNNING_MIDLET) {
            // TODO: render scaled MIDlet framebuffer
        } else if (g_app.state == APP_STATE_ERROR) {
            // TODO: render error screen
        }

        vita2d_end_drawing();

        if (g_app.vsync_enabled) {
            vita2d_swap_buffers();
        }
    }

    // Cleanup
    // TODO: Shutdown JVM
    vita2d_fini();
    
    sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);

    sceKernelExitProcess(0);
    return 0;
}
