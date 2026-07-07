cmake_minimum_required(VERSION 3.10)

# ---------------------------------------------------------------------------
# VitaSDK Toolchain Setup
# ---------------------------------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
            CACHE PATH "VitaSDK toolchain file")
    else()
        message(FATAL_ERROR
            "VITASDK environment variable not set!\n"
            "Install VitaSDK and set: export VITASDK=/usr/local/vitasdk")
    endif()
endif()

project(vitajme C CXX)

# VitaSDK CMake helpers (vita_create_self, vita_create_vpk macros)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

# ---------------------------------------------------------------------------
# App Metadata
# ---------------------------------------------------------------------------
set(VITA_APP_NAME  "VitaJME")
set(VITA_TITLEID   "VJME00001")   # Exactly 9 chars: 4 letters + 5 digits
set(VITA_VERSION   "01.00")

# ---------------------------------------------------------------------------
# Compiler Flags
# ---------------------------------------------------------------------------
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -O2 -std=c11 -ffast-math")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -std=c++14 -ffast-math \
                     -fno-exceptions -fno-rtti")

# Enable ARM NEON SIMD for audio mixing / pixel scaling
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mfpu=neon -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")

# ---------------------------------------------------------------------------
# Source Files — VitaJME Native
# ---------------------------------------------------------------------------
set(VITAJME_SOURCES
    # Entry point
    src/main.c

    # Launcher UI
    src/launcher/launcher.c
    src/launcher/jad_parser.c

    # JAR/ZIP loader
    src/loader/jar_loader.c

    # libvita2d rendering layer
    src/renderer/renderer.c

    # Input handling (SceCtrl + SceTouch)
    src/input/input.c

    # phoneME javacall HAL — Vita port layer
    src/javacall/javacall_os.c
    src/javacall/javacall_lcd.c
    src/javacall/javacall_input.c
    src/javacall/javacall_file.c
    src/javacall/javacall_network.c
    src/javacall/javacall_audio.c

    # Third-party — single-file libraries
    third_party/miniz/miniz.c       # ZIP/JAR parsing (MIT license)
    # TinySoundFont and minimp3 are header-only — included in javacall_audio.c
)

# ---------------------------------------------------------------------------
# phoneME Feature CLDC-HI Sources
# (Subset extracted from phoneME; build system replaced with CMake)
# ---------------------------------------------------------------------------
# The phoneME interpreter core — only the portable C files needed.
# After cloning phoneME as a git submodule, these glob patterns capture the
# interpreter, memory manager, class loader, and GC.
# Adjust paths based on the actual phoneME source layout you obtain.

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/phoneme")
    file(GLOB_RECURSE PHONEME_INTERP
        "third_party/phoneme/cldc/src/vm/share/interpreter/*.cpp"
        "third_party/phoneme/cldc/src/vm/share/memory/*.cpp"
        "third_party/phoneme/cldc/src/vm/share/runtime/*.cpp"
        "third_party/phoneme/cldc/src/vm/share/utilities/*.cpp"
        "third_party/phoneme/cldc/src/vm/share/handles/*.cpp"
        "third_party/phoneme/cldc/src/vm/share/compiler/*.cpp"
        "third_party/phoneme/cldc/src/vm/cpu/arm/*.cpp"
        "third_party/phoneme/cldc/src/vm/cpu/arm/*.c"
        "third_party/phoneme/cldc/src/vm/os/javacall/*.cpp"
    )
    # Exclude files that need OS-specific replacements
    list(FILTER PHONEME_INTERP EXCLUDE REGEX ".*Win32.*")
    list(FILTER PHONEME_INTERP EXCLUDE REGEX ".*linux_i386.*")

    set(VITAJME_SOURCES ${VITAJME_SOURCES} ${PHONEME_INTERP})
    message(STATUS "phoneME sources: ${CMAKE_CURRENT_LIST_DIR}/third_party/phoneme")
else()
    message(WARNING
        "third_party/phoneme/ not found.\n"
        "Run: git submodule update --init third_party/phoneme\n"
        "Building without phoneME (skeleton only).")
endif()

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(vitajme ${VITAJME_SOURCES})

# ---------------------------------------------------------------------------
# Include Paths
# ---------------------------------------------------------------------------
target_include_directories(vitajme PRIVATE
    src/
    src/javacall/
    third_party/miniz/
    third_party/tsf/         # TinySoundFont header-only
    third_party/minimp3/     # minimp3 header-only
)

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/phoneme")
    target_include_directories(vitajme PRIVATE
        third_party/phoneme/cldc/src/include
        third_party/phoneme/cldc/src/vm/share/interpreter
        third_party/phoneme/cldc/src/vm/share/runtime
        third_party/phoneme/cldc/src/vm/os/javacall/include
        third_party/phoneme/javacall/interface/include
        third_party/phoneme/javacall/interface/midp
    )
endif()

# ---------------------------------------------------------------------------
# Compile Definitions
# ---------------------------------------------------------------------------
target_compile_definitions(vitajme PRIVATE
    # phoneME build flags
    ENABLE_CLDC_11=1              # CLDC 1.1 (float support, generics-era)
    MIDP_20=1                     # MIDP 2.0 profile
    VITA_PLATFORM=1               # Our custom platform guard
    ARM=1                         # ARM processor
    HARDWARE_LITTLE_ENDIAN=1      # Vita is little-endian
    # TinySoundFont
    TSF_IMPLEMENTATION=1
    # minimp3
    MINIMP3_IMPLEMENTATION=1
    MINIMP3_ONLY_MP3=1
)

# ---------------------------------------------------------------------------
# Linked Libraries
# ---------------------------------------------------------------------------
target_link_libraries(vitajme
    # 2D graphics
    vita2d
    # GPU / display
    SceGxm_stub
    SceDisplay_stub
    # Input
    SceCtrl_stub
    SceTouch_stub
    # Audio
    SceAudio_stub
    SceAudioIn_stub
    # File I/O
    SceIofilemgr_stub
    # Networking
    SceNet_stub
    SceNetCtl_stub
    SceHttp_stub
    SceHttps_stub
    # System
    SceKernel_stub
    SceSysmodule_stub
    SceAppMgr_stub
    ScePower_stub
    # Font
    ScePgf_stub
    SceCommonDialog_stub
    # Math + C runtime (newlib)
    m
    c
)

# ---------------------------------------------------------------------------
# Build Targets
# ---------------------------------------------------------------------------

# Step 1: ELF → SELF (signed executable, eboot.bin)
vita_create_self(vitajme.self vitajme UNSAFE)

# Step 2: SELF → VPK (ZIP with sce_sys assets)
vita_create_vpk(vitajme.vpk ${VITA_TITLEID} vitajme.self
    VERSION ${VITA_VERSION}
    NAME    "${VITA_APP_NAME}"
    # System assets
    FILE sce_sys/icon0.png                          sce_sys/icon0.png
    FILE sce_sys/pic0.png                           sce_sys/pic0.png
    FILE sce_sys/livearea/contents/template.xml     sce_sys/livearea/contents/template.xml
    FILE sce_sys/livearea/contents/bg0.png          sce_sys/livearea/contents/bg0.png
    FILE sce_sys/livearea/contents/startup.png      sce_sys/livearea/contents/startup.png
    # Bundled soundfont for MIDI playback
    FILE assets/gm.sf2                              data/gm.sf2
)

# ---------------------------------------------------------------------------
# Print build summary
# ---------------------------------------------------------------------------
message(STATUS "==============================================")
message(STATUS "  VitaJME Build Configuration")
message(STATUS "  App Name  : ${VITA_APP_NAME}")
message(STATUS "  Title ID  : ${VITA_TITLEID}")
message(STATUS "  Version   : ${VITA_VERSION}")
message(STATUS "  Toolchain : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "==============================================")
