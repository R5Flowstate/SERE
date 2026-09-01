# SERE — RUI Node Editor Guide

## What is SERE?

SERE is a visual node editor for creating custom RUI (Respawn UI) assets. You design UI elements by connecting nodes in a graph, preview them in real-time, then export to a compiled RPak that runs in-game.

**Target:** Season 21. Export stamps `packageVersion=2`, **`ruiVersion=42`**, widget sizes `[28,50,30,30,48,14]` (S21 engine / V42.1 class), S16 transform strides. Preview loads S21 paks: `ui.rpak`, `ui_main_menu.rpak`, `ui_lobby.rpak`, `ui_arenas.rpak`, `ui_sdk.rpak` (UIIA + font v12).

**Workflow:** Design in SERE → Export → Auto-builds DLL + RPak → Deploy to game → Load with `pak_requestload`

---

## Quick Start

### 1. First Launch
- Set your game path in **Settings** to the S21 install (`s21-full` or LIVE) — loads uiia/font atlases for preview
- Set **repak.exe** path in Settings to your RePak build (`repak.exe`)
- Canvas: **1920x1080** (matches in-game fullscreen RUI)

### 2. Create Your First RUI
1. Right-click canvas → **Transform** → **Transform 2** (positions your widget)
2. Right-click → **Image Render** → **Render Image** (draws an image)
3. Right-click → **Constant** → **Asset Constant** (pick an image)
4. Right-click → **Constant** → **Size Constant** (set width/height)
5. Wire: Size → Transform.Size, Transform.Out → Image.Transform, Asset → Image.Main Asset
6. Set Size to `(400, 0, 0, 300)` for a 400x300 rectangle
7. **File → Export** → saves .ruip + compiles DLL + builds RPak + deploys

### 3. Load In-Game
```
pak_requestload my_rui_name
script_client CreateFullscreenRui($"ui/my_rui_name.rpak")
```

---

## Navigation

| Action | Control |
|--------|---------|
| Pan canvas | Right-click drag / Middle-mouse drag |
| Zoom | Mouse wheel |
| Reset zoom | R key |
| Add node | Right-click on canvas |
| Delete node | Right-click on node → Delete |
| Connect pins | Drag from output pin to input pin |

---

## Node Reference

### Constants

| Node | Output | Description |
|------|--------|-------------|
| **Float Constant** | Float | Single number with min/max slider |
| **Vector2 Constant** | Float2 | X,Y pair |
| **Color Constant** | Color | RGBA color picker |
| **String Constant** | String | Text string |
| **Size Constant** | Size | Transform size as 2x2 matrix: `(Width, 0, 0, Height)` |
| **Asset Constant** | Asset | Image from loaded RPak atlases |

### Globals (update every frame from engine state)

| Node | Output | Description |
|------|--------|-------------|
| **Current Time** | Float | Game time in seconds (drives all animations) |
| **ADS Fraction** | Float | 0.0 = hip fire, 1.0 = fully aimed down sights |
| **Screen Width/Height** | Float | Current viewport dimensions |

### Math

| Node | Inputs | Output | Description |
|------|--------|--------|-------------|
| **Add** | A, B | Res | A + B |
| **Subtract** | A, B | Res | A - B |
| **Multiply** | A, B | Res | A * B |
| **Divide** | A, B | Res | A / B (error on zero) |
| **Modulo** | A, B | Res | fmod(A, B) |
| **Sine** | A | Res | sin(A) |
| **Cosine** | A | Res | cos(A) |
| **Absolute** | A | Res | abs(A) |
| **Exponent** | A, B | Res | pow(A, B) |
| **Floor/Ceil/Round/Truncate** | A | Res | Rounding operations |

### Split / Merge

| Node | Description |
|------|-------------|
| **Split Vector2** | Float2 → X, Y floats |
| **Merge Vector2** | X, Y floats → Float2 |
| **Split Color RGB** | Color → R, G, B, A floats |
| **Merge Color RGB** | R, G, B, A floats → Color |
| **Merge Color HSV** | H, S, V, A floats → Color |
| **Split/Merge Size** | Split or merge transform size components |

### Conditionals

| Node | Description |
|------|-------------|
| **Greater / Less** | A > B or A < B → Bool |
| **Equal Float / String** | Equality check → Bool |
| **Conditional Float** | Bool, A, B → A if true, B if false |
| **Not / And / Or** | Boolean logic gates |

### Transforms (position and size widgets on screen)

| Node | Type | Description |
|------|------|-------------|
| **Transform 2** | 1-pin | Place widget at position within parent. **Most common.** |
| **Transform 3** | 1-pin | Scale-independent (preserves shape in stretched parent) |
| **Transform 4** | 1-pin | Proportional (scales with parent) |
| **Transform 5/6** | 1-pin | Screen-aware horizontal/vertical |
| **2 Pin Scale** | 2-pin | Stretch between two anchor points |
| **2 Pin Pinch** | 2-pin | Aspect-ratio-aware stretch |
| **2 Pin Stretch** | 2-pin | Rotated stretch with rsqrt precision |
| **3 Pin Transform** | 3-pin | Full affine from three points |
| **Rotate Transform** | Rotation | Rotates parent by angle around center |
| **Copy Transform** | Utility | Duplicates a transform for modification |

**Transform 2 pins:**
- **Val_0** (Float2): Position within parent (0-1 normalized). `(0.5, 0.5)` = center.
- **Val_3** (Float2): Anchor point on self (0-1). `(0.5, 0.5)` = center-anchored.
- **Size**: Widget dimensions as `(Width, 0, 0, Height)`.
- **Parent**: Parent transform (defaults to full-screen identity).

**Rotate Transform pins:**
- **Parent**: Transform to rotate.
- **Rotation** (Float): Angle. `0.0` = 0 degrees, `1.0` = 360 degrees. Feed `Time * speed` for continuous spin.
- **Rotation Origin** (Float2): Center of rotation `(0.5, 0.5)` = widget center.
- **Size**: Inherits parent size if not connected.

### Render Nodes

| Node | Description |
|------|-------------|
| **Render Image** | Draws an image with full mask/clip support |
| **Render Ellipse** | Draws circles and rings with slice controls |
| **Text Render** | Draws text (requires Text Size + Text Style) |
| **Text Size** | Computes text dimensions for auto-sizing transforms |
| **Text Style** | Font, size, color, shadow, and all style properties |

### Style Properties (on Image, Ellipse, and Text Style nodes)

| Property | Default | Description |
|----------|---------|-------------|
| **Main Color** | White | Primary color/tint |
| **Blend** | 1.0 | Blend factor |
| **Premul** | 0.0 | Premultiplied alpha mode |
| **Tint** | (1,1,1,1) | Multiplicative color overlay. Dims or colorizes without changing main color. |
| **Hue** | 0.0 | Color wheel rotation (additive) |
| **Saturation** | 1.0 | Color intensity. 0 = grayscale, 1 = full color. |
| **Lightness** | 0.0 | Brightness shift (additive) |
| **Kerning** | 0.0 | Letter spacing (text only) |

---

## Common Patterns

### Centered Image
```
Size(400, 0, 0, 300) → Transform2(val0=0.5,0.5) → ImageRender
```

### Auto-Sized Text
```
TextStyle → TextSize("Hello") → TextRender
TextSize.Size → Transform2.Size  ← auto-sizes to text dimensions
Transform2 → TextRender.Parent
```

### Spinning Element
```
Time → Multiply(×speed) → RotateTransform.Rotation
Transform2 → RotateTransform.Parent → Render
```

### RGB Rainbow Color
```
Time×6 → Sine → ×0.5+0.5 → MergeColor.R
Time×6+2.094 → Sine → ×0.5+0.5 → MergeColor.G
Time×6+4.189 → Sine → ×0.5+0.5 → MergeColor.B
MergeColor → TextStyle.mainColor
```

### Pulsing Alpha
```
Time×3 → Sine → ×0.3+0.7 → MergeColor.Alpha
White + pulsing alpha → TextStyle.mainColor
```

### ADS-Reactive Element
```
ADSFrac × 0.8 + 0.1 → MergeVector2.X (slides in from left)
0.5 → MergeVector2.Y (centered vertically)
MergeVector2 → Transform2.Val_0
ADSFrac → MergeColor.Alpha (invisible when not aiming)
```

### Grayscale Toggle
```
Saturation = 0.0 → grayscale image (great for "disabled" state)
Saturation = 1.0 → full color
```

---

## Transform Size Format

The Size input is a 2x2 matrix packed as 4 floats: `(m00, m10, m01, m11)`.

For axis-aligned rectangles (most common):
```
(Width, 0, 0, Height)
```

Example: 600x400 rectangle → Size Constant with `Value_0=600, Value_1=0, Value_2=0, Value_3=400`

The engine only uses elements [0] (width) and [3] (height) for standard transforms. Elements [1] and [2] enable rotation/skew but are rarely needed.

---

## Position Coordinates

All positions are **normalized 0-1** relative to the canvas (1920x1080):

| Position | Meaning |
|----------|---------|
| (0, 0) | Top-left corner |
| (0.5, 0.5) | Center of screen |
| (1, 1) | Bottom-right corner |
| (0.5, 0) | Top-center |
| (0, 1) | Bottom-left |

The **Val_3** (anchor) pin determines which point of the widget is placed at the position:
- `(0, 0)` = top-left corner of widget at position
- `(0.5, 0.5)` = center of widget at position (most common)
- `(1, 1)` = bottom-right corner at position

---

## Export Pipeline

When you click **File → Export**:

1. **RUIP** generated (binary RUI package with transforms, styles, widgets)
2. **C++ DLL** compiled (ruiFunc that sets dynamic values each frame)
3. **RuiHeaders.h** generated (type definitions for the DLL)
4. **RPak** built via repak.exe (packages RUIP + DLL)
5. **Deployed** to `{GamePath}/paks/Win64/`

The DLL runs every frame. It reads `globals->currentTime` for animations, `globals->globalAdsFrac` for gameplay state, calls `funcs->LoadAsset()` for images, and `funcs->executeTransform()` to process the transform bytecode.

---

## Tips

- **Preview is live** — animations (Time, Sin, Rotation) run in the editor
- **ADS Fraction** only has meaningful values during gameplay (aim down sights)
- **Font hash 19** = ArameMono (the default font available in most RPaks)
- **String pointers** are automatically fixed up by the RPak loader
- **Multiple text elements** work — each gets its own style, transform, and render node
- **Layer order** on render nodes controls draw order (lower = drawn first = behind)
- **Right-click drag** to pan the node editor canvas
- **Mouse wheel** to zoom
