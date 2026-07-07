/**
 * vitajme.h — Global types, constants, and configuration for VitaJME
 *
 * This header is included by all VitaJME modules. It defines the
 * resolution presets, scaling modes, platform-wide types, and the
 * shared application state structure.
 */
#ifndef VITAJME_H
#define VITAJME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <psp2/types.h>
#include <psp2/kernel/processmgr.h>

/* --------------------------------------------------------------------------
 * Screen constants
 * -------------------------------------------------------------------------- */
#define VITA_SCREEN_W   960
#define VITA_SCREEN_H   544

/* --------------------------------------------------------------------------
 * Resolution presets (MIDlet virtual canvas sizes)
 *
 * These are the 6 resolution options the user can select in the launcher.
 * The MIDlet renders into a virtual framebuffer of this size; VitaJME then
 * scales it up to the 960x544 physical screen.
 *
 * Chosen to cover the full historical range of Java ME devices:
 *   128x128  — Very old WAP/i-mode phones (Siemens, early Nokias)
 *   176x208  — Nokia Series 40 portrait (most classic games target this)
 *   240x320  — QVGA portrait (sweet spot — most Java ME games use this)
 *   320x240  — QVGA landscape (racing/action games)
 *   352x416  — Nokia N-Series / Sony Ericsson W-Series (hi-res)
 *   480x272  — PSP native — widest modern Java games & ports
 * -------------------------------------------------------------------------- */
typedef enum {
    RES_128x128  = 0,
    RES_176x208  = 1,
    RES_240x320  = 2,   /* default — most compatible */
    RES_320x240  = 3,
    RES_352x416  = 4,
    RES_480x272  = 5,
    RES_COUNT    = 6
} JmeResolution;

typedef struct {
    int         width;
    int         height;
    const char *label;
} ResolutionPreset;

static const ResolutionPreset g_resolutions[RES_COUNT] = {
    { 128, 128, "128x128  (WAP/i-mode)" },
    { 176, 208, "176x208  (Nokia S40)"  },
    { 240, 320, "240x320  (QVGA)"       },
    { 320, 240, "320x240  (QVGA Land.)" },
    { 352, 416, "352x416  (N-Series)"   },
    { 480, 272, "480x272  (PSP Wide)"   },
};

/* --------------------------------------------------------------------------
 * Scaling / letterbox modes
 * -------------------------------------------------------------------------- */
typedef enum {
    SCALE_FIT_LETTERBOX = 0,  /* Preserve aspect ratio, add bars     */
    SCALE_STRETCH       = 1,  /* Stretch to fill entire 960x544      */
    SCALE_INTEGER_2X    = 2,  /* Nearest-neighbour integer 2× only   */
    SCALE_INTEGER_MAX   = 3,  /* Largest integer scale that fits     */
} ScaleMode;

/* --------------------------------------------------------------------------
 * Key mapping — user-remappable via launcher settings
 * -------------------------------------------------------------------------- */
typedef struct {
    uint32_t vita_btn;    /* SCE_CTRL_* bitmask */
    int      midp_key;    /* MIDP Canvas key code */
    const char *label;
} KeyMapping;

/* Default key mapping (can be overridden in settings) */
#define KEYMAP_COUNT 14
extern KeyMapping g_keymap[KEYMAP_COUNT];

/* --------------------------------------------------------------------------
 * Application state
 * -------------------------------------------------------------------------- */
typedef enum {
    APP_STATE_LAUNCHER = 0,
    APP_STATE_RUNNING_MIDLET,
    APP_STATE_PAUSED,
    APP_STATE_ERROR,
} AppState;

typedef struct {
    AppState     state;
    JmeResolution resolution;
    ScaleMode    scale_mode;

    /* Currently loaded MIDlet info */
    char         midlet_jar_path[256];
    char         midlet_jad_path[256];
    char         midlet_name[128];
    char         midlet_vendor[128];
    char         midlet_version[32];
    char         midlet_main_class[256];

    /* JVM heap configuration (MB) */
    int          heap_size_mb;   /* default: 64 */
    int          stack_size_kb;  /* default: 256 */

    /* Audio settings */
    bool         audio_enabled;
    int          audio_volume;   /* 0–32768, SCE_AUDIO range */

    /* Display settings */
    bool         vsync_enabled;
    bool         show_fps;

    /* Error state */
    char         error_msg[512];
} AppConfig;

/* Global app config — defined in main.c */
extern AppConfig g_app;

/* --------------------------------------------------------------------------
 * Paths
 * -------------------------------------------------------------------------- */
#define JAVA_DATA_ROOT   "ux0:data/java/"
#define JAVA_RMS_SUBDIR  "rms/"
#define SF2_PATH         "ux0:app/VJME00001/data/gm.sf2"
#define SETTINGS_PATH    "ux0:data/java/vitajme.cfg"

/* --------------------------------------------------------------------------
 * Utility macros
 * -------------------------------------------------------------------------- */
#define VITAJME_VERSION_STR  "1.0.0"
#define VITAJME_TITLE        "VitaJME"

#define CLAMP(x, lo, hi)   ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#define ARRAY_LEN(a)        (sizeof(a) / sizeof((a)[0]))
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))

/* RGBA colour helper (vita2d format: 0xAABBGGRR) */
#ifndef RGBA8
#define RGBA8(r,g,b,a)  (((a)<<24) | ((b)<<16) | ((g)<<8) | (r))
#endif

/* Logging */
#ifdef DEBUG
#  include <stdio.h>
#  define VJME_LOG(fmt, ...) printf("[VitaJME] " fmt "\n", ##__VA_ARGS__)
#else
#  define VJME_LOG(fmt, ...) do {} while(0)
#endif
#define VJME_ERR(fmt, ...) \
    do { \
        snprintf(g_app.error_msg, sizeof(g_app.error_msg), \
                 "[ERROR] " fmt, ##__VA_ARGS__); \
        VJME_LOG("%s", g_app.error_msg); \
    } while(0)

#endif /* VITAJME_H */
