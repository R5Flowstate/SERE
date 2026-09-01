# SERE ( R5Flowstate/S21 )

Node editor for authoring Respawn RUI assets.

Agents view included: CLAUDE.md

## What this fork adds

- Targets **ruiVersion 42**: widget table, layout opcodes, transform
  matrices, style descriptors and string tables all match what the engine
  reads, rather than the earlier uimg-era layout.
- 28 global nodes, and Tint / Hue / Saturation / Lightness / Kerning pins on
  every widget type.
- Screen blur shaped by a painted mask, plus widget clip pins and
  per-instance pin prototypes.
- Live preview against real game art: loads uiia and font atlases straight
  out of the game's paks, with BC-compressed textures decoded for display.
- Export builds the RUI pak and its module in one step, so a graph goes from
  the editor to a loadable asset without a separate packing pass.
- Sessions, left-click panning, dark theme, and auto-generated RUI headers.

## Usage

Set the game path in **Settings** so the editor can load atlases for
preview. Build a graph, then export to produce the pak and module.

## Building

```
git clone --recursive <repo>
cmake -B build -A x64 && cmake --build build --config Release
```

`--recursive` matters: the file dialog is a submodule.

Upstream: [RoyalBlue1/SERE](https://github.com/RoyalBlue1/SERE)
