# Photo Assistant (CYD) - context for Claude Code

ESP32 "Cheap Yellow Display" photography assistant for Howard: pinhole, general and large format. Read README.md, docs/FILM_DATA.md and docs/SCREENS.md first.

## Session workflow

- Start: `git pull`. Then run the host tests to confirm the core still passes before touching firmware: `make -C test/host`, or on Windows `cd test/host; powershell -File run.ps1` (needs g++, e.g. WinLibs; open a new terminal after installing so it is on PATH).
- Build and flash: `python -m platformio run -t upload --upload-port COM4` (the board is a CH340 on COM4 on Howard's PC; check Device Manager if it moved).
- End: `git add .`, `git commit`, `git push`. Remind Howard if he forgets.

## State of the project

| Part | State |
|---|---|
| `lib/PhotoCore/` | Done, 165 host checks. Portable C++, no Arduino includes: films, exposure, exposure_state, pinhole_state, countdown, numentry, textentry, solar. |
| UI (`src/`) | Built and running on the ILI9341 CYD in portrait: Home, Exposure (4 pages incl. film tuning), Pinhole, countdown timer, Sun, custom location editor + keypad, touch calibration, Settings (time zone, sync, 3 WiFi slots) with an on-screen keyboard. |
| Clock | WiFi + NTP at boot (`src/clock_sync.h`). Networks are entered in Settings (flash) and tried first; the gitignored `src/wifi_secrets.h` (template: `wifi_secrets.example.h`) is an optional fallback. |
| Phase 2 | Not started: light meter, GPS, RTC, shot log, shutter trigger.  |

## Rules

1. **Keep logic out of the UI.** New calculation or state logic goes in `lib/PhotoCore` with a host test in `test/host/test_core.cpp`. `main.cpp` and the `*_screen.h` files only draw and read touch. State struct and tests first, drawing on top.
2. **Never invent film data.** Numbers in `films.cpp` come from published sources, listed in docs/FILM_DATA.md. If a value is guessed or unverified, say so in code and docs. Color films are a ballpark (P = 1.30) and must show an experimental warning. Table films flag `beyondData` past their last point. Box ISOs in `films.cpp` are the manufacturers' published speeds; verify them against the data sheets.
3. **Check results against known values** (Ilford's own HP5+ example 10 s -> 20.4 s, the Kodak table anchors). A test that merely reproduces the code's output is not verification; work expected values by hand.
4. **Touch-first UI**: tap targets of at least 40 px; Howard uses it outdoors, sometimes gloved.

## Hardware (verified)

- Board: ILI9341 CYD (ESP32-2432S028), CH340 USB serial. The display is on the HSPI bus (`-DUSE_HSPI_PORT`); the touch controller (XPT2046: CS 33, IRQ 36, CLK 25, MOSI 32, MISO 39) uses the default VSPI bus via `SPI.begin(...)` and `ts.begin()`. Howard also has CYDs running other firmware and a "2-USB" ST7789 variant (needs `-DST7789_DRIVER`, `-DTFT_INVERSION_ON`, different pins); ask which unit before flashing something else.
- Touch uses a two-point calibration saved to flash (hold a finger on the screen at boot to redo it).
- No buzzer fitted: the countdown flashes the backlight.

## Gotchas learned

- TFT_eSPI font 6 (big digits) has no `+` glyph; draw it as shapes. Font 6 and 7 are digits only, so text with letters tops out at font 4 (use `setTextSize(2)` to enlarge).
- `ESP32 NVS` keys are limited to 15 characters; per-film tuning is keyed by film id, so keep ids short.
- `exposureInit` and `pinholeInit` must run before `loadTune()` and `loadState()` in `setup()`. Exposure/Pinhole settings and the selected location are saved to flash (`src/state_store.h`) 3 s after the last tap; loaded data always goes through `exposureSanitize`/`pinholeSanitize`.

## Decisions already made

- Phase 1 scope (done): exposure math with reciprocity and compensation, pinhole calculator, sun/golden hour, countdown. Phase 2: light meter (BH1750 or TSL2591), GPS (NEO-6M), DS3231 RTC, shot log, shutter-head trigger.
- Clock: sync over WiFi (home or phone hotspot) with NTP. No RTC yet.
- Locations: presets (Edmond default, Broken Bow, Okarche, Tulsa) plus one custom lat/lon (and UTC offset, DST on/off) saved to flash. A future GPS fills the same fields.
- Films: Kodak T-Max 100/400, Tri-X; Ilford Delta 100, FP4+, HP5+, Pan F+, Ortho+; Kentmere 100/400 (on his first list, delete if unused); color ballpark for Gold 200, Ektar 100, Portra 160. X-ray film was dropped.
- Pinholes (6x6): 50 mm / 0.30 mm = f/167, and 35 mm / 0.26 mm = f/135. Lens focal lengths offered for bellows compensation: 18, 35, 50, 85 mm. Aperture list includes 135, 137, 167, 215.
- Film tuning is a multiplier on the stops reciprocity adds (x0.50 to x2.00, saved per film), not an override of P.

## Next task

Not decided. Candidates: starting phase 2 (light meter first), or editing the film list (Howard will ask if he starts shooting new film; until then leave the list alone).

When the light meter is added: the Home Settings tile took the light meter's placeholder slot. Move Settings to a small gear in the Home header (or go to a 2x3 grid) and restore the tile. The Settings keyboard keys are about 27 px wide (below the 40 px rule) on purpose; it is a setup-only screen.
