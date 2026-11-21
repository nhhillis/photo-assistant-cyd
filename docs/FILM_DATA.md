# Film data and sources

Single source of truth in code: `lib/PhotoCore/films.cpp`. Edit there to add or tune a film.

## Models

- **Power** (Ilford style): `Tc = Tm^P` for `Tm > 1 s`. No correction at 1 s or less.
- **Table** (Kodak style): log-log interpolation between published points, anchored at 1/10 s = no correction (Kodak shows "None" up to 1/10 s). Past the last point the final segment's slope is extended and the result is flagged `beyondData`.
- **Color**: Power model with P = 1.30 as a ballpark only. Kodak publishes no color-negative correction past 1 s. Always shown with an "experimental, expect color shift" warning.

## Table

| Film | Model | Data | Notes |
|---|---|---|---|
| Kodak T-Max 100 | Table | 1 s -> 1.26 s; 10 s -> 15 s; 100 s -> 200 s | No development change (Kodak). Verify against printed sheet. |
| Kodak T-Max 400 | Table | 1 s -> 1.26 s; 10 s -> 15 s; 100 s -> 300 s | No development change. Verify against printed sheet. |
| Kodak Tri-X 400 | Table | 1 s -> 2 s; 10 s -> 50 s; 100 s -> 1200 s | Dev -10% / -20% / -30% at 1 / 10 / 100 s (Kodak F-4017). |
| Ilford Delta 100 | Power | P = 1.26 | Ilford tech sheet, Dec 2023 |
| Ilford FP4+ | Power | P = 1.26 | same |
| Ilford HP5+ | Power | P = 1.31 | same |
| Ilford Pan F+ | Power | P = 1.33 | same |
| Ilford Ortho+ | Power | P = 1.25 | same |
| Kentmere 100 | Power | P = 1.26 | same sheet (K100) |
| Kentmere 400 | Power | P = 1.30 | same sheet (K400) |
| Kodak Gold 200 | Power (ballpark) | P = 1.30 | Experimental |
| Kodak Ektar 100 | Power (ballpark) | P = 1.30 | Experimental; community tests suggest ~2x metered at long times |
| Kodak Portra 160 | Power (ballpark) | P = 1.30 | Experimental; community tests suggest ~3x metered at long times |

Kentmere is included because it was on your first list. Delete the two lines if you no longer shoot it.

## Known caveats (why every profile should be editable)

1. Ilford publishes near-identical charts for several films, and forum testers report real behavior can be worse than the exponent predicts (e.g. FP4+ at 30 s). Treat Ilford P values as a starting point and tune with your own test frames.
2. Kodak's tables end at 100 s. Pinhole exposures often go past that. Results beyond the table are extrapolated (Tri-X at 1000 s extrapolates to ~8 hours) and are shown with a warning.
3. Ilford's chart range is assumed to be ~100 s for the `beyondData` flag. Adjust `dataLimit` if you find a documented range.
4. Reciprocity depends on light level, not only time. Real exposure in changing light (sunset) drifts, so very long exposures are guidance, not guarantees.

## Planned: user tuning

Per-film `P` override (and multiplier) saved to flash with `Preferences`, so a test roll can adjust the curve without reflashing.
