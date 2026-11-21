# Screens - 240x320 portrait, touch-first

Large tap targets (>= 40 px) for cold or gloved fingers. Core math is in `lib/PhotoCore`; screens only call it. Colors: dark navy background, amber accents, results in green.

## Home
2x2 grid of tiles (Exposure, Pinhole, Sun / Golden, Settings), a location chip under the header, and a footer with the clock status ("Clock: synced 10:41 AM", "syncing...", or "sync failed - tap" to retry) and the version.

## Shared row UI (`src/rowpage.h`)
Used by the Exposure, Pinhole and location editor screens.
- Result panel on top (0-110 px).
- Three rows of 50 px high buttons: `-` (60 px) | value | `+` (60 px), 6 px gaps. Tapping the middle of a row can open a keypad (location editor) or reset (tuning page).
- Bottom: Home, `<` previous page, `>` next page.
- Tap the panel to start the countdown when the exposure is 1 s or longer.

## Exposure (four pages)
The stack applies in order: metered time -> shoot aperture -> ISO -> filter/ND -> bellows -> reciprocity (tuned).
1. **SETUP**: Film, Film speed (EI, defaults to the box speed), Shoot at aperture
2. **METER**: Metered time, Metered at aperture, Meter ISO
3. **COMPENSATION**: Filter/ND (1/3 stops), Bellows extension (jumps past the focal length on the first +), Lens focal length (18, 35, 50, 85 mm)
4. **FILM TUNING**: Film, Reciprocity tune (x0.50 to x2.00, saved per film), Reset this film

Aperture list (metered and shoot): full stops from f/1 to f/256 plus 135, 137, 167, 215.

Panel: final time in large green digits, stops added (plus development change for films with data), and warnings in red: "Color film: experimental, expect shift", "Beyond published data: extrapolated". The stops line turns amber and reads "tuned" when a film's tuning is not x1.00.

## Pinhole (three pages)
Shoot aperture = focal length / hole diameter, then the same stack as Exposure.
1. **PINHOLE**: preset (50 mm / 0.30 mm = f/167, 35 mm / 0.26 mm = f/135), focal length (5 mm steps), hole diameter (0.01 mm steps). Editing either value shows the preset as "custom".
2. **FILM + METER**: Film, Metered time, Metered at aperture
3. **ISO + FILTER**: Meter ISO, Film speed (EI), Filter/ND

Panel: final time, f-number and field of view (along one side of a 56 mm 6x6 frame), optimal hole (Rayleigh, `1.9 * sqrt(f * 550 nm)`) and stops added. Film tuning set on the Exposure screen applies here too.

## Countdown timer
Opens from the green time. READY -> START -> PAUSE/RESUME, STOP resets, BACK returns to the screen that started it. Digits are `M:SS` (or `H:MM:SS`, tenths under 10 s), amber under 10 s, with a progress bar. At zero the backlight and digits flash for 6 s.

## Sun / Golden hour
`<` / `>` cycle the presets (Edmond default, Broken Bow, Okarche, Tulsa) and the saved Custom location. Rows: civil dawn, sunrise, golden hour ends, golden hour starts, sunset, blue hour ends. The date comes from the synced clock; if the clock never synced the firmware build date is used and the screen says so in red.

Definitions: sunrise/sunset at -0.833 deg; golden hour = sunrise/sunset to +6 deg; blue hour = -4 to -8 deg; civil twilight = -6 deg.

## Custom location
Sun screen -> **Custom...**. Page 1: latitude and longitude (`-`/`+` nudge 0.01 deg, about 1 km; tap the value for a numeric keypad, with south and west negative). Page 2: UTC offset (standard time) and US daylight saving on/off. The panel previews today's sunrise and sunset. **Home** saves it to flash and selects it. A future GPS fills the same fields.

## Touch calibration
Two crosshair taps on first boot (saved to flash). Hold a finger on the screen while booting to recalibrate.

## Settings
Home -> Settings. Two pages of rows (same row UI as Exposure):
1. **CLOCK**: Time zone (`-`/`+`: US Eastern, Central, Mountain, Arizona, Pacific, Alaska, Hawaii, UTC; saved to flash), Sync clock now (tap the middle of the row), Clock status. The top panel shows the current time, date and zone.
2. **WIFI**: three network slots; tap a slot to edit. The name is typed first, then the password (the old password is kept if you retype the same name). An empty name + OK clears the slot. Saving a network starts a sync straight away.

On-screen keyboard: 8 x 5 keys (about 27 px wide, below the 40 px rule because this screen is only used for setup), three layers (lowercase, UPPERCASE, symbols; the layer button shows the layer you get next), plus Space, Del, OK and Cancel. All printable ASCII characters can be typed.

Layout note: the Settings tile took the place of the dimmed "Light Meter" placeholder. When the light meter is built, move Settings to a small gear button in the Home header and give the tile back, or switch Home to a 2x3 grid.

## Phase 2 (not in this scope)
Light meter (BH1750 / TSL2591), DS3231 RTC, GPS (NEO-6M), shot log to SD, shutter-head trigger. 
