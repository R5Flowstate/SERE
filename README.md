# SERE (R5Flowstate / S21)

**S**implified **E**ditor for **R**ui **E**lements — visual node editor for
Respawn RUI. This fork targets **Season 21**: export stamps `ruiVersion=42`
(V42.1 widget sizes) and preview loads S21 `uiia` / font-v12 paks. Upstream
SERE (RoyalBlue1) targets older RUI / uimg-era paks.

Upstream: [RoyalBlue1/SERE](https://github.com/RoyalBlue1/SERE).

## What this fork adds

- Export: `packageVersion=2`, **`ruiVersion=42`**, widget sizes
  `[28,50,30,30,48,14]`, S16 transform strides.
- Preview: S21 `uiia` v2 images + font atlas v12 (not uimg).
- Auto RPak build via RePak after export.
- Transform / ellipse / asset / style-descriptor layouts fixed for S21.
- Full `RuiGlobals` emit, 28 global nodes, HSL preview, session system.

See `docs/SERE_GUIDE.md` for the editor workflow.

## Building

CMake + Visual Studio. Open the generated `SERE` project and build Release.
