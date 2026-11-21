# Photo Assistant (CYD)

A pocket photography assistant for an ESP32 "Cheap Yellow Display": reciprocity and exposure compensation, a pinhole calculator, and sunrise/golden-hour times. Built for pinhole, general and large format work.

## Status

| Part | State |
|---|---|
| `lib/PhotoCore` (film data, reciprocity, pinhole math, sun calculator) | Written, passes host tests |
| `src/main.cpp` UI | Scaffold only: home menu + sun screen. Not yet compiled or run on hardware |
| Exposure and Pinhole screens | Designed in `docs/SCREENS.md`, not built |

## Run the tests (no hardware needed)

```
make -C test/host
```

## Layout

```
lib/PhotoCore/   portable C++ (no Arduino): films, exposure, solar
src/main.cpp     TFT_eSPI + touch UI
test/host/       unit tests run on your PC
docs/            FILM_DATA.md (sources, caveats), SCREENS.md (screen designs)
```

## Hardware notes

`platformio.ini` assumes the common ILI9341 CYD (ESP32-2432S028R). If your board is the "2-USB" ST7789 variant, swap the driver defines (`-DST7789_DRIVER`, `-DTFT_INVERSION_ON`) and verify the pins. Touch ranges in `main.cpp` need calibrating on your unit.

## Workflow

`git pull` before you start; `git add .`, `git commit`, `git push` when you finish.
