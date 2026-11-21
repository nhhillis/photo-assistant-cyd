# Photo Assistant (CYD) - context for Claude Code

ESP32 "Cheap Yellow Display" photography assistant for Howard: pinhole, general and large format. Read README.md, docs/FILM_DATA.md and docs/SCREENS.md first.

## Session workflow

- Start: `git pull`. Then run `make -C test/host` to confirm the core still passes before touching firmware.
- End: `git add .`, `git commit`, `git push`. Remind Howard if he forgets.

## State of the project

| Part | State |
|---|---|
| `lib/PhotoCore/` (films, exposure, solar) | Written and tested on PC (40 checks). Portable C++, no Arduino includes. |
| `src/main.cpp` | **Scaffold only.** Home menu and a Sun screen. Never compiled, never run on hardware. |
| Exposure and Pinhole screens | Designed in docs/SCREENS.md, **not built**. |

You are the first to build and flash this. Expect compile errors and wrong touch calibration; fix them, do not assume the scaffold is right.

## Rules

1. **Keep logic out of the UI.** New calculation or state logic goes in `lib/PhotoCore` with a host test in `test/host/test_core.cpp`. `main.cpp` only draws and reads touch. Build the Exposure screen as a testable state struct first (film, metered time, ISO, filter stops, bellows), then the drawing on top.
2. **Never invent film data.** Numbers in `films.cpp` come from published sources, listed in docs/FILM_DATA.md. If a value is guessed or unverified, say so in code and docs. Color films are a ballpark (P = 1.30) and must show an experimental warning. Table films flag `beyondData` past their last point.
3. **Check results against known values** (Ilford's own HP5+ example 10 s -> 20.4 s, the Kodak table anchors). A test that merely reproduces the code's output is not verification.
4. **Touch-first UI**: tap targets of at least 40 px; Howard uses it outdoors, sometimes gloved.

## Hardware assumptions to verify first

- Board: assumed the common ILI9341 CYD (ESP32-2432S028R). Howard also has CYDs running other firmware, so ask which unit this is. The "2-USB" variant is ST7789 and needs `-DST7789_DRIVER`, `-DTFT_INVERSION_ON`, different pins.
- Touch (XPT2046) is on its own SPI: CS 33, IRQ 36, CLK 25, MOSI 32, MISO 39. The raw-to-screen `map()` ranges in `main.cpp` are placeholders; calibrate on the unit.

## Decisions already made

- Phase 1 scope: exposure math with reciprocity and compensation, pinhole calculator, sun/golden hour. Phase 2: light meter (BH1750 or TSL2591), GPS (NEO-6M), DS3231 RTC, shot log, shutter-head trigger.
- Clock: sync over WiFi (home or phone hotspot) with NTP. No RTC yet.
- Locations: presets (Edmond default, Broken Bow, Okarche, Tulsa) plus a custom lat/lon saved to flash with `Preferences`. A future GPS fills the same fields.
- Films: Kodak T-Max 100/400, Tri-X; Ilford Delta 100, FP4+, HP5+, Pan F+, Ortho+; Kentmere 100/400 (on his first list, delete if unused); color ballpark for Gold 200, Ektar 100, Portra 160. X-ray film was dropped.
- Pinholes (6x6): 50 mm / 0.30 mm = f/167, and 35 mm / 0.26 mm = f/135.
- Every film profile should become user-tunable (a P multiplier saved to flash) because published reciprocity data is approximate.

## Next task

Build the Exposure screen (docs/SCREENS.md): state struct and host tests first, then the screen, then the countdown timer. Then the Pinhole screen, then WiFi/NTP time and custom locations.
