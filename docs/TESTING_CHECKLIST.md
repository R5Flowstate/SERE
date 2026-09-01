# SERE In-Game Testing Checklist

## Priority 1 — Core Rendering (must work for any HUD)

### Widgets
- [x] Asset image — renders correctly sized and positioned
- [x] Asset image — correct UV mapping (full image, not cropped)
- [ ] Ellipse — basic circle renders
- [ ] Ellipse — ring (innerMask < 1.0)
- [ ] Ellipse — pie slice (sliceBegin/sliceEnd)
- [x] Text — renders with correct font
- [x] Text — multiple text elements in one RUI
- [ ] Video widget — packs 30B type 3, channel DWORD
- [ ] Camera widget — packs 48B type 4, slot < 10
- [ ] Nested widget — packs 14B type 5, ui handle DWORD
- [ ] Letterbox Transform — opcode 7, same pins as Rigid
- [ ] Text — drop shadow visible (hardness, offset, blur)
- [ ] Text — bold/thicken effect
- [ ] Text — outline (backgroundSize > 0)

### Transforms
- [x] Transform 2 — fixed-size positioning (pixel-space)
- [x] Rotate Transform — continuous rotation via Time
- [ ] Nested transforms — child Transform2 inside parent Transform2
- [ ] Transform 4 — proportional (child scales when parent resizes)
- [ ] Transform 3 — scale-independent (child stays fixed in stretched parent)

### Text Layout
- [x] Auto-sized text via TextSize → Transform.Size
- [ ] Text alignment (center, left, right via textAlign field)
- [ ] Line wrapping (long text with lineBreakWidth)
- [ ] Fit-to-width (text scales down to fit)

---

## Priority 2 — Style & Visual Effects (polish and aesthetics)

### Style Descriptors
- [ ] Tint — multiplicative color overlay on image
- [ ] Tint — multiplicative color overlay on text
- [ ] Saturation = 0 — image renders grayscale
- [ ] Saturation = 0 — text renders grayscale
- [ ] Hue rotation — color shifts on image
- [ ] Lightness — brightness adjustment
- [ ] Kerning — letter spacing on text
- [ ] Premul variations — different alpha compositing

### Colors
- [x] RGB rainbow cycling (sin chains → Merge Color → text)
- [x] Alpha pulsing (sin → alpha channel)
- [ ] Color tinting an image (Tint pin with non-white color)
- [ ] Secondary/tertiary colors on text (scndColor, tertColor)

---

## Priority 3 — Gameplay Integration (reactive HUDs)

### Globals
- [x] Current Time — continuous animation
- [x] ADS Fraction — responds to aim down sights
- [ ] Screen Width — reads correct viewport width
- [ ] Screen Height — reads correct viewport height
- [ ] Wall Time — reads wall clock (uint64)
- [ ] Walltime Arg — 8-byte uint64 slot, RuiSetWallTime
- [ ] Image Arg — type IMAGE + LoadAsset handle
- [ ] Font Face Arg / Font Hash Arg
- [ ] Ui Handle Arg — type UIHANDLE int, not IMAGE
- [ ] Localize / SNPrintF / SetHidden ruiFunc nodes

### Dynamic Behavior
- [x] Position driven by ADS (slides element)
- [x] Alpha driven by ADS (fades element)
- [ ] Element hidden when value = 0 (fully transparent)
- [ ] Element visible only during gameplay state
- [ ] Multiple globals driving different elements simultaneously

---

## Priority 4 — Advanced Transforms (complex layouts)

### Multi-Pin Transforms
- [ ] 2 Pin Scale — element stretches between two anchor points
- [ ] 2 Pin Pinch — aspect-ratio-aware stretch
- [ ] 2 Pin Stretch — rotated stretch between arbitrary points
- [ ] 3 Pin Transform — full affine from three reference points

### Screen-Aware
- [ ] Transform 5 — horizontal screen-aware sizing
- [ ] Transform 6 — vertical screen-aware sizing

### Composition
- [ ] 3+ nested transform levels (grandparent → parent → child)
- [ ] Multiple widgets sharing the same transform
- [ ] Widgets at different layers overlapping correctly

---

## Priority 5 — Engine Functions (advanced features)

### Vtable Functions (never tested)
- [ ] SNPrintF — formatted text in DLL
- [ ] Localize — localized string lookup
- [ ] ToUpper — uppercase conversion
- [ ] FormatNumber — number formatting
- [ ] SinNorm — engine-side normalized sine
- [ ] RandomFloat — random value generation
- [ ] SrgbToLinear — color space conversion
- [ ] ProjectWorldPoint — 3D world position to screen
- [ ] Animate1D/2D/3D/4D — keyframe animation curves
- [ ] StringToHash — runtime string hashing
- [ ] GetKeyColor — colorblind palette colors
- [ ] GetViewportScale — viewport DPI scale

### Architecture
- [ ] RUI arguments (script-settable via SetArgInt/Float from Squirrel)
- [ ] Hidden function (ruiHiddenFunc — runs when RUI is hidden)
- [ ] Multiple custom RUIs loaded simultaneously
- [ ] Hot-reload (re-export + pak_requestload without restart)

---

## Priority 6 — Edge Cases & Stress Testing

- [ ] 10+ widgets in a single RUI
- [ ] 20+ transform entries
- [ ] 5+ text elements with different styles
- [ ] Very long text string (100+ characters)
- [ ] Very small widget (10x10 pixels)
- [ ] Very large widget (full screen 1920x1080)
- [ ] Widget positioned partially off-screen
- [ ] Zero-size transform (should not crash)
- [ ] Missing image asset (LoadAsset returns -1)
- [ ] Multiple RPaks with custom RUIs loaded at once

---

## Test Procedure

For each item:
1. Create or modify the test RUI in SERE
2. Verify it looks correct in SERE preview
3. Export (auto-builds RPak + DLL)
4. In-game: `pak_requestload <name>` then `script_client CreateFullscreenRui($"ui/<name>.rpak")`
5. Compare in-game appearance with SERE preview
6. Mark checkbox when confirmed working
7. Note any discrepancies between preview and in-game
