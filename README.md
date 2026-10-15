# MirrorKit

Mirrors for Chivalry 2 Unchained maps: live reflections that show players, cheap shiny surfaces and breakable mirror tiles. MirrorKit is a **library**. It adds no map by itself; maps use its assets and require it to be installed.

Version **0.5.0**. Built with [ArgonSDK](https://github.com/Chiv2-Community/ArgonSDK) (Unreal Engine 4.25). The optional [MirrorLab](https://github.com/SageFlare/MirrorLab) map demonstrates every mirror type.

## What you need

| You are | Install | Where |
| --- | --- | --- |
| **Player** joining a server | `MirrorKit.pak` **and** the map's own PAK | Through the [Unchained Launcher](https://github.com/Chiv2-Community/UnchainedLauncher) Mod Manager, or copy both into `Chivalry 2/TBL/Content/Paks/`. Launch in Unchained multiplayer mode. |
| **Server host** | `MirrorKit.pak` **and** the map's PAK | The server's `TBL/Content/Paks/`, or install both mods in the launcher before "Launch Headless". The server does no mirror work; it only needs the files so the map's mirror actors load. |
| **Mapper** | `MirrorKit_Source.zip` and `MirrorKit.pak` | See [Making a map with mirrors](#making-a-map-with-mirrors). |

Without `MirrorKit.pak`, a correctly built map still loads, but every mirror, tile and stand-in is missing.

Unchained `mod.json` for MirrorKit: `"mod_type": "Shared"`, `"tags": ["Assets"]`. A map that uses MirrorKit lists it under `"dependencies"`.

## The mirrors

All live mirrors are `BP_CaptureMirror` or one of its presets. Presets differ only in their settings, and every setting can be changed on any placed mirror.

| Asset | What you see | Cost |
| --- | --- | --- |
| `BP_CaptureMirror` | True reflection: your full body (moving legs, head, weapon in hand), other players and the room. With a facing `Partner`, an infinite-looking tunnel | High |
| `BP_Mirror_Funhouse` | Magnified, gappy carnival look | High |
| `BP_Mirror_Headless` | Your reflection has no head (others look normal) | High |
| `BP_Mirror_StretchHead` | Your head stretches as you look up and down | High |
| `BP_Mirror_Ghost` | Your body is invisible; your weapons float | High |
| `BP_Mirror_TwoSided` | Reflects on both faces, e.g. a free-standing mirror wall | High (only the face you look at renders) |
| `BP_MirrorTileLive` + a `Tiled` mirror | A wall of breakable tiles showing a live reflection on both faces; break one and walk through | Medium (one reflection level) |
| `BP_MirrorTile` / `BP_MirrorTile_NoSnow` | Breakable tiles with a baked (still) reflection; the snow variant plays wind when broken | Very low |
| `M_MirrorEnvironment` | Material: shiny metal with a baked image of the room, plus screen-space reflections at shallow angles | Very low |
| `M_MirrorDarkCenter` | Material: reflects at shallow angles, black when looking straight at it | Very low |

Every effect is per viewer. A ghost or headless mirror changes only *your own* reflection as *you* see it; other players see you normally in their mirrors.

## Making a map with mirrors

### Install the kit in ArgonSDK

1. Unzip `MirrorKit_Source.zip`.
2. Copy its `Content` folder's contents to **`ArgonSDK/Content/Mods/MirrorKit/`**, so you get `ArgonSDK/Content/Mods/MirrorKit/Blueprints/BP_CaptureMirror.uasset`.
   This location matters. Unreal names assets by folder (`/Game/Mods/MirrorKit/...`), and `MirrorKit.pak` loads them under the same names in game. Do not move or rename kit assets; your map would then point at assets the shared PAK does not contain. A folder link is fine.
3. Copy `MirrorKit.pak` into your game's `TBL/Content/Paks/` to test.

### Place a live mirror

1. Drag `BP_CaptureMirror` (or a preset) into the level.
   - The mirror's **local +X axis points out of its reflective face**, and the face is at the actor's origin.
   - Switch the transform gizmo to **Local** to see it.
   - Turn wall mirrors with **Rotation Z (Yaw)**.
2. Size it with the actor **scale**: Y × 100 = width in cm, Z × 100 = height in cm, and keep X = 1. For example, (1, 12, 6) is a 12 × 6 m mirror. For a floor mirror, rotate it to face up.
3. Leave `Partner` empty for a normal mirror (see [Partners](#partners-and-tunnels) for the tunnel effect).
4. **Leave `Depth` + 3 cm clear behind the face** (4 cm at the default `Depth` 1). The reflection layers live there; geometry inside that space flickers. Selecting a mirror shows an orange `ClearanceGuide` box over this space; it has no collision and is hidden in game.
5. Mirrors are solid: the backing plate blocks players and weapons, except in `Tiled` mirrors, where the tiles collide. The back of a one-sided mirror shows plain metal. Use `TwoSided` for a free-standing mirror wall.
6. Nothing else to place. The stand-ins that draw your body in reflections (`BP_MirrorDouble*`) spawn automatically.

Two-sided spacing is automatic: `TwinGap` is clamped to at least `Depth` + 4, and `TwinGapWasAdjusted` records any correction. `Depth` is clamped to 1-10. MirrorKit cannot move your map geometry, so external clearance is up to you.

### Build your map so it works with the shared PAK

- **Use your own meshes and materials for structure.** Place only kit actors and kit mirror materials. A player without MirrorKit then still gets a playable map instead of holes in it.
- **Your PAK must not contain MirrorKit's files.** Cooking your map also writes cooked kit files to `Saved/Cooked/.../Content/Mods/MirrorKit/`; package only your own mod folder.
- **Keep the kit's enums out of native conversion.** If your project uses Blueprint nativization:
  - Add `/Game/Mods/MirrorKit/Blueprints/E_MirrorResolutionProfile` and `/Game/Mods/MirrorKit/Blueprints/E_MirrorStaggerMode` as `+ExcludedAssets` under `[BlueprintNativizationSettings]` in `Config/DefaultEditor.ini`.
  - Otherwise UE 4.25 can leave them out of the PAK.
  - Recook referring maps after changing this.
- Add MirrorKit as a dependency in your `mod.json`.

## Mirror settings

All settings are on every placed mirror. Defaults reproduce a single, sharp, every-frame mirror; the performance settings are opt-in.

**Look and body**

| Setting | Default | What it does |
| --- | --- | --- |
| `HeadSource` | 3 | Your body in the reflection: 3 full body; 1 head and torso only (stiff head); 2 stretching head; 0 headless |
| `HideBody` | off | Ghost: hide your character from this mirror |
| `PortalMode` | on | Off = the older screen-mapped (funhouse) look |
| `MatchViewFov` | on | Off = funhouse magnification |
| `FitVisible` | on | Render only the part of the mirror inside your view (sharper close up) |
| `ScreenLevel1` | off | Sharper large **floor** mirrors when looking straight down; use with `ClipBehind` off and nothing below the mirror |
| `ClipBehind` | on | Hide geometry behind the mirror (screen-mapped look) |
| `TwoSided` | off | Also reflect on the back face |
| `TwinGap` | 6 cm | Distance between the two faces of a two-sided mirror (minimum `Depth` + 4) |
| `Tiled` | off | The mirror is invisible and walk-through; tiles show its reflection |

**Partners and depth**

| Setting | Default | What it does |
| --- | --- | --- |
| `Partner` | none | Facing mirror for the tunnel effect; empty = independent mirror |
| `PartnerBackFace` | off | Target the partner's generated back face (the partner must be `TwoSided`) |
| `BackPartner` | none | Partner for this mirror's back face |
| `BackPartnerBackFace` | off | Make the back face target `BackPartner`'s back face |
| `Depth` | 1 | Reflection levels with a partner, 1-10. Each level is another full render; above 3 is not recommended |

**Resolution**

| Setting | Default | What it does |
| --- | --- | --- |
| `Resolution` | 1 | Overall scale. With the default profile, targets are 1280 × 720 at level 1 and 640 × 360 deeper. 0.5 is a good value for mirror-heavy maps |
| `ResolutionProfile` | FixedDeep | Deeper-level rule: `FixedDeep`, `Linear` or `Multiply` |
| `DeepResolutionScale` | 0.5 | `FixedDeep`: scale for every level after the first |
| `ResolutionDropPerLevel` | 0.15 | `Linear`: amount subtracted per level |
| `PerLevelResolutionMultiplier` | 0.5 | `Multiply`: factor applied per level |
| `MinimumDepthResolutionScale` | 0.125 | Lowest scale for deeper levels in any profile |

**Performance**

| Setting | Default | What it does |
| --- | --- | --- |
| `UpdateEveryNFrames` | 1 | Update every N frames; skipped frames reuse the last reflection. 2 roughly halves capture work |
| `StaggerMode` | None | Spread updates across frames: `Distributed` (all mirrors) or `Distance` (mirrors at or beyond `DistanceStaggerStart`). Needed with N > 1 to keep worst-case frames smooth |
| `DistanceStaggerStart` | 1000 cm | Face distance where `Distance` staggering begins |
| `DistanceThrottle` | off | Use `FarUpdateEveryNFrames` at or beyond `DistanceThrottleStart` |
| `DistanceThrottleStart` | 1500 cm | Face distance where throttling begins |
| `FarUpdateEveryNFrames` | 4 | Update interval for throttled (distant) mirrors |
| `CaptureDistance` | 2500 cm | Stop updating beyond this face distance (the mirror keeps its last image) |
| `MaxViewDistance` | 0 | Limit how far each capture renders into the reflected scene; 0 = unlimited |
| `VisibilityTolerance` | 0.2 s | Stop updating when neither this mirror nor its partner has been drawn for this long |
| `OffscreenUpdateEveryNFrames` | 1 | While the whole face is outside your view: 1 unchanged, N > 1 every N frames, 0 frozen |
| `DisableReflectionAO` | off | Skip ambient occlusion inside this mirror's reflections (cheaper, rarely visible) |

"Face distance" means the camera's distance to the nearest point of the mirror, not to its centre.

## Performance

A live mirror renders the scene again each time it updates. The cost grows with the number of these **captures per frame**, far more than with mirror size or resolution. Measured at roughly 0.2-0.3 ms per capture in the test maze.

For a map with many mirrors, start with:

- **Bake your lighting** (static lights). Each capture otherwise recalculates movable lights. This was the biggest single gain: +23-31% FPS.
- **Use fewer, longer mirrors** instead of many pieces on the same wall.
- `UpdateEveryNFrames` 2 with `StaggerMode` Distance, `Resolution` 0.5, `DisableReflectionAO`, and `OffscreenUpdateEveryNFrames` 0.

**[Designing maps for mirror performance](docs/MAP-OPTIMIZATION.md)** has the measurements, the visual cost of each option and layout advice. In the test maze, these steps took walking FPS from 136 to about 298 with every reflection still live.

Rough costs measured in MirrorLab (one PC, solo, uncapped):

| Setup | Frame rate | Extra per frame |
| --- | --- | --- |
| Facing pair, `Depth` 3, both in view | 120-200 fps | 2-5 ms |
| Floor + ceiling tunnel, `Depth` 3 | ~170 fps | ~3 ms |
| Two-sided tiled wall, `Depth` 1 | ~210 fps | ~2 ms |
| Baked tiles | 200-300 fps | < 1 ms |
| Screen-space floor/ceiling materials | 150-220 fps | ~1-3 ms |

## How a mirror decides to update

Each frame, a mirror goes through four steps:

1. **Schedule.** Every tick increments `FrameCounter`. An update is due when `(FrameCounter + phase) % N == 0`.
   - `N` is `UpdateEveryNFrames`, or `FarUpdateEveryNFrames` when throttled, or the off-screen interval.
   - `phase` is 0 unless the mirror is staggered; then it is `|round(X + 3Y + 7Z)| % N`, from the mirror's position.
2. **Eligibility.** All of these must hold:
   - you have a local camera;
   - the face distance is under `CaptureDistance`;
   - you are in front of the face;
   - the mirror (or its partner) was drawn within `VisibilityTolerance`.

   "Drawn" includes being seen inside another mirror. Distances check the centre distance first and compute the exact nearest point only when needed.
3. **Off screen.** If `OffscreenUpdateEveryNFrames` is not 1, the four face corners are tested against your view (with a 15% margin). Walls hiding a mirror in your view are not detected.
4. **Capture.** Level 1 is rendered (fitted to the visible part with `FitVisible`), then deeper partner levels.

Independent mirrors show other mirrors' images inside their reflection. Those images were rendered for your own viewpoint, so mirror-in-mirror repetitions are plausible but not geometrically exact. Partners give exact nested reflections, at the cost of one render per level.

`DisableReflectionAO` works through a stored post-process override (AO intensity 0) whose blend weight the checkbox raises at start-up. Two-sided mirrors copy their settings, including `OffscreenUpdateEveryNFrames` and `DisableReflectionAO`, to the generated back face.

A mirror's size and settings are read when play starts. Do not resize live mirrors at runtime.

## Partners and tunnels

- Set two facing mirrors as each other's `Partner` and raise `Depth` (usually 3) for an infinite-looking tunnel. Set relationships before play starts.
- Partners do not have to be parallel: each reflection is computed across each mirror's own plane.
- **Back faces:**
  - Enable `PartnerBackFace` to target a two-sided partner's back.
  - For links that start from a two-sided mirror's back, use `BackPartner` and `BackPartnerBackFace`.
  - Example: Funhouse front to TwoSided back uses `Funhouse.Partner = TwoSided`, `Funhouse.PartnerBackFace = on`, `TwoSided.BackPartner = Funhouse`, `TwoSided.BackPartnerBackFace = off`.
- **One partner per mirror.** Chains (A → B → C → A) get exact deeper reflections only into each mirror's own partner.

## Tiles and materials

**Breakable live tiles**

1. Place a `BP_CaptureMirror` covering the wall. Turn on `Tiled` (and `TwoSided` for both faces) and keep `Depth` 1.
2. Place `BP_MirrorTileLive` tiles across it, centred on the wall plane, with thickness about the `TwinGap`. For example, scale (1.5, 0.06, 2.0) is a 150 × 200 cm tile.
3. Set each tile's `Wall` to that mirror.

A broken tile disappears on both faces and leaves a walk-through hole, with one health bar per tile.

**Baked tiles.** Place `BP_MirrorTile` (snow wind on break) or `BP_MirrorTile_NoSnow` and scale to the tile size. They use `SM_MirrorTile` and `SM_MirrorTile_Cracked` by default, and never show players. To match your room's shine, use your own environment material instance on their `StaticMesh`.

**Environment material.** `M_MirrorEnvironment` shows a baked 360° image (cubemap) where screen-space reflections have nothing. To match your room:

1. Place a Scene Capture Cube at the surface, at head height.
2. Assign a Cube Render Target and capture.
3. Right-click the render target and choose **Create Static Texture**.
4. Create a Material Instance of `M_MirrorEnvironment` in your map's folder and set `Environment` (plus `EnvStrength`, `Tint` and `Roughness` if wanted).
5. Apply it, then delete the capture actor.

## Online play

Each player's game renders mirrors from their own camera, so everyone sees a correct reflection. Your stand-in exists only on your machine; other players see your real body. Breakable tiles use the game's own breakable windows: the server decides when a tile breaks, and everyone sees it. Dedicated servers do no mirror rendering and spawn no stand-ins.

## Known limitations

- Mirror-in-mirror images of independent mirrors are not geometrically exact (see above).
- Mirrors hidden behind walls but inside your view can still update when they appear in another reflection.
- Sheathed weapons do not appear in first-person reflections (weapons in hand do; third person shows everything).
- Head-only variants (`HeadSource` 1, stretch head) have a stiff head.
- Live tiles show only the first reflection level. Baked tiles never show players.
- Tile breaking can only be tested in game (the SDK's window Blueprints are placeholders).
- One partner per mirror. Do not resize live mirrors at runtime.

## Contents

| Path (`/Game/Mods/MirrorKit/`) | Assets |
| --- | --- |
| `Blueprints/` | `BP_CaptureMirror`; presets `BP_Mirror_Funhouse`, `BP_Mirror_Headless`, `BP_Mirror_StretchHead`, `BP_Mirror_Ghost`, `BP_Mirror_TwoSided`; tiles `BP_MirrorTileLive`, `BP_MirrorTile`, `BP_MirrorTile_NoSnow`; stand-ins `BP_MirrorDoubleFull`, `BP_MirrorDouble`, `BP_MirrorDoubleStretch` (spawned automatically); enums `E_MirrorResolutionProfile`, `E_MirrorStaggerMode` |
| `Materials/` | `M_MirrorEnvironment`, `M_MirrorPolished` (screen-space only), `M_MirrorDarkCenter`, `M_MirrorCracked`; internal: `M_CaptureMirror`, `M_Matte` |
| `Meshes/` | `SM_MirrorTile`, `SM_MirrorTile_Cracked`; internal: `SM_MKCube`, `SM_MirrorTileCore` |
| `Textures/` | `TC_MK_DefaultEnvironment` (default cubemap) |

Release files: `MirrorKit.pak` (+ `.sha256`), `MirrorKit_Source.zip` (`Content/`, README, `mod.json`, license and third-party notices), `README.md`.

## For maintainers

**Reading the Blueprint.** Open `BP_CaptureMirror`. EventGraph routes BeginPlay to `InitializeMirror` and Tick to `UpdateMirror`; double-click the calls to follow the work. Coloured `MirrorKit |` comment blocks explain each function, and functions are grouped by category:

| Category | Functions |
| --- | --- |
| Lifecycle | `InitializeMirror`, `UpdateMirror` |
| Setup | `ConfigureBackingAndDepth`, `CreateBackFace`, `ConfigureTileMode`, `InitializeCaptureResources`, `InitializeFirstCapture`, `InitializeDeepCapture`, `EnsurePlayerStandIn` |
| Scheduling | `ResolvePartner`, `AdvanceCaptureSchedule`, `CheckCaptureEligibility`, `IsWithinFaceDistance`, `DistanceToMirrorFace`, `UpdateOffscreenFlag` |
| Capture | `CaptureReflectionStack`, `CapturePortalFirst`, `CapturePortalDeep`, `CaptureLegacyView` |
| Projection | `ReflectViewAcrossPlane`, `ProjectMirrorRay`, `FitMirrorInterval`, `ComputeVisibleMirrorBounds` (pure maths) |
| Visibility | `RefreshCaptureHiddenActors`, `HideActorFromActiveCaptures`, `HidePartnerPanelsForCapture` |

**Diagnostics.** Each mirror counts its captures in `CaptureCount`. `TelemetryMark` is reserved for telemetry actors that read the counters. MirrorKit never reads either value.

**AgGraphTools.** [`Tools/AgGraphTools`](Tools/AgGraphTools) is the editor-only UE 4.25 plugin that MirrorLab's generator uses to build MirrorKit's Blueprint graphs from Python, a capability UE 4.25 Python lacks. Players, hosts and mappers do not need it. `Tools/annotate_blueprints.py` reapplies the comment blocks without changing logic. Cleanup validation is recorded in [docs/blueprint-cleanup-validation.md](docs/blueprint-cleanup-validation.md).

## Changelog

| Version | Changes |
| --- | --- |
| 0.5.0 | Performance and usability release (developed as internal builds after 0.4.0):<br>• Independent mirrors: no pairing needed; one capture per face.<br>• Cheaper per-frame work: checks before hidden-list maintenance, cached stand-ins, shared Blueprint functions, bounded loops (behaviour verified identical) and editor comment blocks.<br>• Distances measured to the nearest point of the mirror face, centre-first and only when a feature uses them.<br>• Exact update-slot formula `(FrameCounter + phase) % N`.<br>• New settings `OffscreenUpdateEveryNFrames` and `DisableReflectionAO`; two-sided back faces inherit them.<br>• `CaptureCount` / `TelemetryMark` diagnostics.<br>• README rewritten; map-design guide `docs/MAP-OPTIMIZATION.md`. |
| 0.4.0 | Front/back partner targeting for two-sided mirrors; spacing safeguards (`Depth` 1-10, `TwinGap` minimum, clearance guide); default depth 1 |
| 0.3.0 | First public release |

## License

Original MirrorKit work is licensed under the MIT License. See `LICENSE` and `THIRD_PARTY_NOTICES.md`.
