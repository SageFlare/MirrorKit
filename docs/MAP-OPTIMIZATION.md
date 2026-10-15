# Designing maps for mirror performance

Chivalry 2 is competitive, and many players run 144-240+ FPS. Every live mirror renders the scene again, so a mirror-heavy map can lose a lot of frame rate. This guide covers what a map designer can do about it. The advice is based on measurements, listed at the end.

## The one rule: count captures, not mirrors

Each time a live mirror updates, it renders the scene once. That is a *capture*. Frame time grows with the number of **captures per frame**. Mirror size and render-target resolution matter much less.

- Measured: roughly **0.2-0.3 ms per capture** on the test PC (Resolution 0.35-0.5). FPS followed captures-per-frame second by second.
- Lowering resolution from 0.5 to 0.35 saved only about 0.14 ms per frame once captures were reduced.

Everything below reduces captures, makes each capture cheaper, or spreads captures evenly across frames.

## 1. Bake your lighting (biggest single gain)

Movable lights are recalculated in **every** view, so each mirror capture redoes the lighting. With static (baked) lights, walls and floors read precomputed light maps, which is cheap in the main view and in every capture.

- Measured: **+23% walking, +31% standing** (243 → 298 and 239 → 313 FPS on the test PC). Captures per frame stayed the same; each one just got cheaper.
- How:
  - Set lights to **Static**.
  - In World Settings, turn off **Force No Precomputed Lighting**.
  - Make sure meshes have a lightmap UV channel.
  - Build lighting.
  - Set decoration meshes without lightmap UVs (unlit trims, decals, floor guides) to **Movable**.
- Shadows on static lights are free at runtime. Turning them on gives real dark corners and contact shadows.
- Large single meshes (a whole floor or ceiling) get coarse light maps that show as soft squares and grain. Some find this adds character; split the mesh into tiles if you want it smooth.
- Trade-offs: lights cannot change in game. Players are lit from stored light samples rather than direct light, so they look slightly flatter.
- General baking pitfall, not mirror-specific: overlapping faces in the same plane (for example a wall running into a pillar of equal width) flicker once baked.
- Do not try to switch lights off only inside mirrors. Chivalry 2 is a shipping build, and the engine ignores light-type show flags there.

## 2. Use fewer, longer mirrors

A wall built from several mirror pieces captures once per piece. One mirror covering the same flat run captures once.

- Measured on a 463-piece maze:

  | Merge | Mirrors | Walking FPS | Standing FPS |
  | --- | --- | --- | --- |
  | None | 463 | 223 | 220 |
  | Touching pieces merged | 326 | 246 | 231 |
  | Also merged across pillar and junction gaps | 243 | **252** | **258** |

  Merging across pillar gaps cut captures per frame by 25-35%.
- Pieces must lie in the same plane, face the same way and share their settings.
- A mirror may run **behind a pillar**: the pillar hides that part, and reflections still show the pillar.
- **Do not run a mirror behind the end of a wall that meets the mirror face.** The wall end sits inside the mirror's render-panel layer (the first few centimetres behind the face) and flickers, which is most visible with baked shadows. Split the mirror at such junctions instead.
- Very long mirrors spread one render target over more wall, so they look slightly softer. Resolution 0.5 compensates cheaply.

## 3. Update every other frame, staggered

- `UpdateEveryNFrames = 2` halves capture work. Measured: about −2.5 to −3.3 ms per frame (136 → 208 FPS walking at the time). Fast motion in reflections is slightly less smooth; testers noticed it only when looking for it.
- On its own, every mirror updates on the same frame, so frames alternate heavy and light, and the worst frames (1% low) do not improve.
- Add `StaggerMode = Distance` (or `Distributed`) to spread mirrors across frames. Measured: 1% low improved from about 110 to 138 FPS walking (187 standing), with the best average as well.
- Staggering is phase-based: `(FrameCounter + phase) % N == 0`, with the phase taken from the mirror's position.

## 4. Settings worth knowing

| Setting | Effect measured | Visual cost | Recommendation |
| --- | --- | --- | --- |
| `Resolution` 0.5 → 0.35 | about +3% (8 FPS walking) after the steps above | softer reflections | 0.5 is a good default; drop only if GPU-bound with many mirrors |
| Ambient occlusion off in captures | about −1 ms standing in early tests; overlaps with resolution | not noticeable in mirrors | needs a preset; not a per-mirror setting yet |
| `CaptureDistance` 15 m | −1.75 ms walking | distant mirrors visibly **freeze** | prefer staggering and merging; keep the default 25 m unless the layout hides distant mirrors |
| `OffscreenUpdateEveryNFrames` 0 | +11 FPS walking, +21 standing in a corner-heavy maze | reflections of mirrors outside your view stop updating inside other mirrors | good for mirror-in-mirror heavy layouts; `4` gave little |
| `MaxViewDistance` | small (+6-9 FPS at 25-15 m from a ~40 FPS baseline, older kit) | removes distant geometry from reflections | rarely worth it in closed layouts |
| `Depth` above 1 (partners) | each level is another full render | infinite-tunnel effect | use only where the effect is the point |

`CaptureDistance`, `DistanceStaggerStart` and `DistanceThrottleStart` measure to the nearest point of the mirror face (0.3.6+), so long mirrors stay live near their ends.

## 5. Layout habits

- Corners where two mirrors face each other keep both live even when one is only visible inside the other.
- Mirrors behind the player, or outside the view, still count while they appear in other reflections. `OffscreenUpdateEveryNFrames` handles the out-of-view part.
- Mirrors inside the view but hidden behind walls can still capture when they appear in a reflection; the engine does not tell Blueprints about main-view occlusion. Breaking long sight lines between mirrors helps.
- Decorative surfaces that do not need a live player reflection should use baked tiles (`BP_MirrorTile`) or `M_MirrorEnvironment` instead of live mirrors.

## How these numbers were measured

- **Map:** the MirrorMaze test map (463 mirror pieces, 81 point lights), one PC, solo, frame-rate cap off.
- **Recording:** NVIDIA App performance logs. Each case is recorded as 30 s standing at spawn plus one fixed walking route, repeated in alternating order (2-3 rounds).
- **Telemetry:** captures per frame come from an in-game telemetry actor reading each mirror's `CaptureCount` (0.3.9+). See `mods/MirrorMazeRenderTests` in the development workspace. Absolute FPS differs per PC; use the relative changes.

| Step (cumulative, walking) | FPS |
| --- | --- |
| Baseline, every frame, Resolution 0.5 | 136 |
| AO off + Resolution 0.35 ("Lite") | 144 |
| + every 2 frames + Distance stagger | 222 |
| + merged mirrors (243) | 252 |
| Resolution back to 0.5 | 243 |
| + baked static lighting | **298** |
| (no mirrors at all) | 379 |
