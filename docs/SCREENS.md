# Screens (phase 1) - 320x240 landscape, touch-first

Large tap targets (>= 40 px) for cold or gloved fingers. Core math is in `lib/PhotoCore`; screens only call it.

## Home
```
+--------------------------------------+
| Photo Assistant            Edmond, OK|
| +--------------+ +-----------------+ |
| |   Exposure   | |     Pinhole     | |
| +--------------+ +-----------------+ |
| +--------------+ +-----------------+ |
| | Sun / Golden | |   (phase 2)     | |
| +--------------+ +-----------------+ |
| WiFi: synced 10:41   [Settings]      |
+--------------------------------------+
```

## Exposure (reciprocity + compensation stack)
Input is the time you metered (handheld meter or phone), then the stack applies in order:
metered time -> aperture change -> ISO change -> filter/ND stops -> bellows -> reciprocity.
```
+--------------------------------------+
| Film: [Ilford FP4+  v]  ISO 125      |
| Metered: 1/60 @ f/16   ISO 100  [-][+]|
| Filter/ND: +0 stops      [-][+]      |
| Bellows ext: 0 mm        [-][+]      |
|--------------------------------------|
|  FINAL:  4 m 05 s     (+3.3 stops)   |
|  Dev: -20%     [!] beyond data        |
+--------------------------------------+
```
- Warning banner: color films ("experimental, expect color shift"); `beyondData` ("extrapolated").
- Tap the final time to start a countdown (big digits, buzzer/backlight flash at end).

## Pinhole
```
+--------------------------------------+
| Focal 50mm   Hole 0.30mm   [presets] |
| f/167   optimal 0.315mm   FOV 58 deg|
|--------------------------------------|
| Meter at: f/16  1/125  ISO 100       |
| -> Pinhole time (metered): 14.1 s    |
| Film: FP4+ -> corrected 4 m 05 s     |
+--------------------------------------+
```
Presets: 50 mm / 0.30 mm (f/167) and 35 mm / 0.26 mm (f/135). The optimal diameter uses the Rayleigh formula `1.9 * sqrt(f * 550 nm)`, which matches both of your pinholes.

## Sun / Golden hour
```
+--------------------------------------+
| < Edmond, OK >        Sat Nov 21     |
| Civil dawn   6:44   Blue  6:34-6:55  |
| Sunrise      7:12   Golden -> 7:50   |
| Golden eve   4:42   Sunset  5:20     |
| Blue eve     5:37-5:58   Dusk 5:47   |
| [Custom lat/lon]                     |
+--------------------------------------+
```
Presets: Edmond (default), Broken Bow, Okarche, Tulsa. **Custom**: type lat/lon and a UTC offset, saved to flash. A future GPS fills the same lat/lon fields.

Definitions used: sunrise/sunset at -0.833 deg; golden hour = sunrise/sunset to +6 deg; blue hour = -4 to -8 deg; civil twilight = -6 deg.

## Settings
- WiFi (home or phone hotspot), NTP sync on boot when WiFi is available
- Timezone rule (default US Central with DST)
- Film profile tuning (P multiplier per film)
- Custom locations

## Phase 2 (not in this scope)
Light meter (BH1750 / TSL2591), DS3231 RTC, GPS (NEO-6M), shot log to SD, shutter-head trigger.
