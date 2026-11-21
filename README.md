# Photo Assistant (CYD)

A pocket photography assistant for an ESP32 "Cheap Yellow Display" (ESP32-2432S028): reciprocity and exposure compensation with a countdown timer, a pinhole calculator, and sunrise/golden-hour times. Built for pinhole, general and large format work. Portrait, touch-first.

## Status

| Part | State |
|---|---|
| `lib/PhotoCore` (films, exposure stack, pinhole, countdown, keypad entry, sun) | Done, 132 host checks pass |
| Home, Exposure, Pinhole, countdown timer, Sun, custom location | Built and flashed to the ILI9341 CYD |
| Touch calibration, WiFi + NTP clock, per-film tuning | Done |
| Phase 2 (light meter, GPS, RTC, shot log, shutter trigger) | Not started |

## Run the tests (no hardware needed)

Linux/macOS (needs g++ and make):

```
make -C test/host
```

Windows (g++ from WinLibs, no make needed):

```
cd test/host
powershell -File run.ps1
```

## Build and flash

PlatformIO (`pip install platformio`):

```
python -m platformio run -t upload --upload-port COM4
```

Use your own COM port. On first boot the board asks you to tap two crosshairs to calibrate touch; hold a finger on the screen while it boots to redo that.

## WiFi clock setup

The clock syncs over WiFi with NTP at boot, then WiFi switches off. Copy `src/wifi_secrets.example.h` to `src/wifi_secrets.h` and enter your networks (home first, phone hotspot second). `wifi_secrets.h` is gitignored. Without it the Home footer says "no WiFi set up" and the Sun screen uses the firmware build date.

## Layout

```
lib/PhotoCore/   portable C++ (no Arduino): films, exposure, exposure_state, pinhole_state,
                 countdown, numentry, solar
src/main.cpp     setup, touch calibration, Home screen, main loop
src/*_screen.h   screens (Exposure, Pinhole, Countdown, Sun, location editor + keypad)
src/rowpage.h    shared "pages of big +/- rows" UI
src/clock_sync.h WiFi + NTP state machine
test/host/       unit tests run on your PC
docs/            FILM_DATA.md (sources, caveats), SCREENS.md (screen designs)
```

## Hardware notes

`platformio.ini` is set up for the ILI9341 CYD with the display on the HSPI bus. The "2-USB" ST7789 variant needs `-DST7789_DRIVER`, `-DTFT_INVERSION_ON` and different pins. The XPT2046 touch controller uses its own SPI pins (CS 33, IRQ 36, CLK 25, MOSI 32, MISO 39). The CYD has no buzzer fitted, so the countdown alerts with a backlight flash.

## Workflow

`git pull` before you start; `git add .`, `git commit`, `git push` when you finish.
