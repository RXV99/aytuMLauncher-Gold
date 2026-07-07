# Aytum Launcher

A native PS Vita homebrew application that interprets Java ME (MIDP 2.0 / CLDC 1.1) `.jar` and `.jad` MIDlet files directly on Vita hardware via vitasdk.

## Features
- **phoneME CLDC-HI Core**: Uses the fast HotSpot ARM JIT engine originally designed by Sun for feature phones.
- **Hardware Acceleration**: Translates Java `Graphics` API calls to `libvita2d` (GPU accelerated).
- **MIDI Synthesis**: High quality MIDI playback using TinySoundFont in a dedicated thread.
- **Multiple Scaling Modes**: Supports integer, letterboxed, and stretched scaling.
- **Remappable Controls**: Full physical button mapping to Java ME key codes.

## Installation
1. Install `vitajme.vpk` on your jailbroken Vita using VitaShell.
2. Place your Java ME `.jar` (and optional `.jad`) files in `ux0:data/java/`.
3. Launch VitaJME from the LiveArea and select your game.

## Building from Source

Prerequisites:
- [VitaSDK](https://vitasdk.org/) installed
- `vdpm` packages installed: `libvita2d`

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake ..
make
```

This will produce `vitajme.vpk` in the `build/` directory.

## License
Aytum Launcher wrapper code is MIT licensed.
The phoneME Feature core is licensed under GPL v2.
miniz and TinySoundFont are public domain / MIT.
