#include "vitajme.h"
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>

#define TSF_IMPLEMENTATION
#include "tsf.h"

// -----------------------------------------------------------------------------
// Javacall Audio HAL (mapping SceAudio + TSF to phoneME)
// This is significantly better than PSPKVM because we use a dedicated audio
// thread and a proper SoundFont synth instead of basic waveforms.
// -----------------------------------------------------------------------------

typedef unsigned int javacall_result;
#define JAVACALL_OK 0
#define JAVACALL_FAIL 1

static int g_audio_port = -1;
static SceUID g_audio_thread = -1;
static bool g_audio_running = false;
static tsf* g_tsf = NULL;

#define AUDIO_SAMPLES 1024
static short g_audio_buf[AUDIO_SAMPLES * 2]; // Stereo

static int audio_thread_func(SceSize args, void *argp) {
    while (g_audio_running) {
        if (g_tsf && g_app.audio_enabled) {
            // Render MIDI to buffer
            tsf_render_short(g_tsf, g_audio_buf, AUDIO_SAMPLES, 0);
            
            // Output to Vita hardware (blocking call)
            int vols[2] = {g_app.audio_volume, g_app.audio_volume};
            sceAudioOutOutput(g_audio_port, g_audio_buf);
        } else {
            // Silence
            memset(g_audio_buf, 0, sizeof(g_audio_buf));
            sceAudioOutOutput(g_audio_port, g_audio_buf);
        }
    }
    return 0;
}

javacall_result javacall_audio_init(void) {
    // Open Vita audio port (44.1 kHz, stereo)
    g_audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, AUDIO_SAMPLES, 44100, SCE_AUDIO_OUT_MODE_STEREO);
    if (g_audio_port < 0) return JAVACALL_FAIL;
    
    // Load TinySoundFont with bundled gm.sf2
    g_tsf = tsf_load_filename(SF2_PATH);
    if (g_tsf) {
        tsf_set_output(g_tsf, TSF_STEREO_INTERLEAVED, 44100, 0);
    } else {
        VJME_ERR("Failed to load soundfont: %s", SF2_PATH);
        // We don't fail init, just run silent
    }
    
    // Start dedicated audio thread at high priority
    g_audio_running = true;
    g_audio_thread = sceKernelCreateThread("vjme_audio", audio_thread_func, 0x30, 0x10000, 0, 0, NULL);
    sceKernelStartThread(g_audio_thread, 0, NULL);
    
    VJME_LOG("Audio subsystem initialized (TSF + SceAudio)");
    return JAVACALL_OK;
}

// These would map to phoneME's MMAPI functions for MIDI playback
void vitajme_audio_note_on(int preset, int key, float vel) {
    if (g_tsf) tsf_note_on(g_tsf, preset, key, vel);
}

void vitajme_audio_note_off(int preset, int key) {
    if (g_tsf) tsf_note_off(g_tsf, preset, key);
}
