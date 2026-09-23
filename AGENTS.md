# SERE — agent notes

Human overview: `README.md`. Editor walkthrough: `docs/SERE_GUIDE.md`.

Target is **S21**: `packageVersion=2`, **`ruiVersion=42`**, widget sizes
`[28,50,30,30,48,14]`. Exported `RuiFunctions_t` is the S21 native UI_* table,
48 slots / `0x180` bytes.

## Build

CMake + Visual Studio, Release. Output under `build/` or `build_2026/`.
Needs a RePak `repak.exe` on the export path (Settings). SERE holds the game
paks open while running — close it before redeploying anything it loaded.

---

## What the engine actually requires

These are measured against the shipping client. Do not "fix" them from
first principles.

### Widgets

Draw types are **0–5 only**: text 28, material 50, ellipse 30, video 30,
camera 48, nested 14. There is no index 6+ — GROUP / FLEX / BLANK are ruiFunc
sizing helpers, not widget types.

Every widget header is `type, drawFlags, xfrmIdx, clipXfrmIdx` (u16 each).

**`drawFlags` at +0x02 is a data offset, not a bitfield.** The engine culls the
widget when `(instanceData[drawFlags] & 3) != 0`, so it must point at a float
`0.0f`. Packing a literal `0` makes it read the first literal in the blob, and
a non-zero low byte there means the widget silently never draws. This is the
most common cause of an invisible RUI.

Material and camera share one shape — `image0, image1, mins, maxs, uvMin,
uvMax`, then the mask block. They are **not** interleaved.

Style descriptors are a 68-byte union. Every style field, `fontHash` included,
is a data offset rather than a value.

### Layout opcodes

The layout stream dispatches through a **14-entry** table, 0–13:

`0 ZeroPin_AutoSize, 1 HalfPin_Clone, 2 OnePin_PosOnly, 3 OnePin_CopyScale,
4 OnePin_Rigid, 5 RigidCopyX, 6 RigidCopyY, 7 RigidLetterbox, 8 TwoPin_Scale,
9 TwoPin_Pinch, 10 TwoPin_Stretch, 11 ThreePin, 12 Rotate, 13 FinishAutoSize`

Clone stream shape: `[type u8][count u8][srcXfrmIdx u16 × count]`.

A normal parented rectangle wants **opcode 4**, not 3 — opcode 3 inherits the
parent's scale and discards Size, turning a small element into a screen-sized
quad.

### ruiFunc slots

Slot **0 is Die** (sets shouldDie *and* shouldHide); slot **1 is Hide**.
Calling slot 0 to hide something destroys the instance and it never returns.
Slot 3 is the `transformSize[]` base. `Animate1D..4D` take
`(inst, int animIdx, float time)` — a **float** time.

Assert the fixed offsets in generated headers: `GetTextSize` 0x20,
`executeTransform` 0x30, `NestedUiSize` 0xB0, `LoadAsset` 0xB8, `HashString`
0x140, `GetLocString0` 0x148, `sizeof == 0x180`.

### Args

Sixteen types, 0–15. WALLTIME is a **uint64**, UIHANDLE is a plain 4-byte int
(not an image + LoadAsset), FONT_HASH is a u16 in a 4-byte slot.

Arg lookup hash — exact, or `RuiSetString` silently no-ops:

```
h = (h >> 20) ^ (bias + h * scale + c)   per character
slot       = h & (numArgs - 1)
validation = h >> 4
```

No extra `h ^= h >> 4`. `varNames` may be null; the hash *is* the lookup.

The arg cluster struct is the engine's symbol table (`varHashBegin/Count`,
magic scale/bias, literal begin/size, `runtimeDataSize`, `maxElemsInArray`,
`widgetBeginByteOfs`, `widgetCount`).

### Text — all four font slots are dereferenced

`GetTextSize` reads `style[fontStyleIdx[i]].fontHash` for **i = 0..3** and
dereferences the font table for each one *before reading a character*. An
undriven slot left on font face 0 — a face that was never loaded — is an
immediate access violation, whether or not the string uses that slot.

**Undriven style slots must reuse slot 0's descriptor.** That removes the
crash and keeps the descriptor count down; style indices are `uint8` and
truncate silently past 255.

Every text widget also needs its **own** `GetTextSize` cache index. Draw walks
`textCache[cacheTextOffset + n]`, so two widgets sharing index 0 makes the
second read a neighbouring slot and fault on a garbage nested-UI handle.

### Screen blur

Negative image indices are code textures: `-1 invalid, -2/-3 raw,
-4 screen blur, -5 camera slot, -6 video slot, -7 custom image`.

- **image0 = -4** forces full-rect UVs and clip — a solid rectangle of blur,
  by design. Only right for a deliberate fullscreen blur.
- **image1 = -4** is taken by the draw path but the widget does **not render**.
  Do not use it.

Blur cannot be shaped by art — only by the widget it sits on. That widget is **not**
limited to an axis-aligned rectangle:

- `UI_ApplyRotate` (opcode 12) writes a full non-diagonal 2x2 into `grad`, so a
  transform — and its quad — can rotate. SERE's Rotate Transform node.
- Every widget carries `clipXfrmIdx`, and the draw path intersects its quad with
  **another transform's quad**, which may itself be rotated. The result is a
  convex polygon of up to 8 verts.

So an axis-aligned quad clipped by a rotated quad is an **exact trapezoid in one
widget** — no tiling, no seams. That is how a slanted panel should be built.
`size_xxyy` constrains what *Size* contributes to a transform; it does not make
quads axis-aligned.

**Do not tile a shape out of quads.** Each quad anti-aliases its own edges, so
every boundary is a visible line, and overlapping them double-composites the blur
into a brighter band instead of hiding the seam. Seam count is the defect.

Shape it from a painted mask rather than by hand:

1. Paint a mask at the element size — solid colour where blur belongs.
2. `tools/mask_to_rects.py <mask.png> --exact` → a rect list.
3. One blur widget per rect: `Transform 2` with `Val_0` = normalised centre,
   `Val_3` = `(0.5, 0.5)`, `Size` = `(w, 0, 0, h)`; material widget with
   `Main Asset = Screen Blur`. Art over the top on a higher layer.

`--exact` is one rect per maximal run of identical row-spans (pixel-exact);
`--coverage`/`--max` give a greedy maximal-rect approximation with far fewer
quads. Always read the reported **coverage and spill** — spill is the failure
that shows.

If you do overlap quads, gate the growth on the pixels beyond the edge being
**`.all()`** inside the mask, never `.any()` — `.any()` grows a whole edge for one
row's neighbour and leaks blur outside the shape.

A high quad count is cheap — all blur quads share an image pair so they batch
into one draw bundle, and fill cost is area, not count. The limit is styles:
widgets index them with a `uint8`, so route every descriptor through `AddStyle`.

`borderMode` selects 9-slice border data only (`0` = image0's, `2` = shared
image1's). It is **not** a mask, and there is no mode where image1's alpha
punches image0.

Blur is refused unless `(uint16)(stage - 1) > 5` — dead in stages 1–6
including HUD and cockpit, fine in VGUI and 7+ — and requires a
screen-aligned plane topology.

---

## Packaging

- The **ui disk header is 112 bytes**, `codeCRC` at +0x68. Verify a built pak
  by its SF_HEAD page size at file offset **+0x88**: it must equal
  `112 × numUiAssets`. A multiple of 104 means the pak writer is stale — fix
  the writer, never hex-patch the pak.
- Pak header flags must include **0x20** or the loader refuses the pak. Set it
  explicitly in the manifest; `hasDynamicLibrary` does not imply it.
- One pak, one module. The engine resolves code with
  `Pak_GetProcAddress(pak, ui->name)`, so a single module serves any number of
  RUIs — one `extern "C" __declspec(dllexport)` per asset name. A module-bearing
  pak is a code-execution surface, so the client allowlists one first-party
  name rather than one per RUI.
- **rpak and module ship as a matched pair.** Data-struct offsets and `codeCRC`
  move together.
- The **asset name is not the pak name**. `ui/<name>.rpak` is the path inside
  the pak, so adding a RUI never moves a `.res` field or a script asset path.

---

## Graphs and the control bridge

SERE serves a JSON-RPC bridge on loopback (`health`, `graph.get/set/load/save`,
`args.set/get`, `preview.png`, `assets.find`, `settings.get`, `export`), with
an MCP wrapper in `mcp/`.

The graph JSON round-trips, and **carries its own canvas size** as
`RuiWidth`/`RuiHeight`. The element size is baked into the asset, so a
fullscreen RUI and a panel RUI cannot share one editor setting — the graph
owns it.

Authoring loop:

1. `graph.set`, then `health` — **assert the node count matches**. A silent
   mismatch means you are about to export an empty asset.
2. `args.set` the real defaults. Whatever is set at export time is **baked as
   the asset's defaults**, so clear test data first.
3. `preview.png`, and look at it.
4. `export`.

**Parse the export result for the `.ruip` path.** Guessing the output directory
picks up a stale package from an earlier run and bakes its old defaults in.
Check the file was rewritten by this run, not merely that it exists.

Preview draws video, camera, nested and screen-blur widgets as a flat quad or
nothing. **Preview silence is not an export failure** — verify packed bytes
instead.

---

## Contract for new nodes

- Every registered node must emit something. A node with an empty `Export`
  contributes no opcode and never registers its out hash, so every child
  silently falls back to transform 0.
- Write the `data->` variables before the first `GetTextSize`. Font-index
  variables live past `dataStructInitSize` and are uninitialised until then;
  that ordering is load-bearing.
- Emit the full `RuiGlobals` struct. A partial emit crashes GlobalNodes pin
  setup.
- Route every style descriptor through `AddStyle`, never
  `styleDescriptor.push_back` — identical descriptors must share a slot,
  because widgets index them with a `uint8`.
- Serialize the canvas (`RuiWidth`/`RuiHeight`) with the graph. Anything that
  rewrites a graph must carry them, or the export bakes whatever the editor
  last had and the asset comes out the wrong size while every normalised
  position still looks right.

## Do not

- Target uimg-era or older `ruiVersion`.
- Draw the entire uiia map in ImGui (cap the draw list).
- Insert dummy atlas entries for unresolved image names — that path hangs on
  the full map. Missing names before hashes resolve are normal.
- Hardcode machine paths in Settings docs, source, or this file.
- Rewrite deployed game text (`.res`, `.rson`, `.cfg`) with a PowerShell text
  cmdlet — it writes a UTF-8 BOM and the engine's parsers reject it. Use a
  binary-mode write and verify no BOM, unchanged line endings, balanced braces.
- Commit `build/`, `build_2026/`, `work/`, or game UI paks.
