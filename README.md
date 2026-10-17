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
| `BP_MirrorTileLive` + two `Tiled` mirrors | A wall of breakable tiles showing a live reflection on both faces; break one and walk through | Medium (one reflection level) |
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
5. Mirrors are solid: the backing plate blocks players and weapons, except in `Tiled` mirrors, where the tiles collide. The back of a mirror shows plain metal. For a mirror wall that reflects on both sides, place a second mirror on the back facing the other way (see spacing below).
6. Nothing else to place. The stand-ins that draw your body in reflections (`BP_MirrorDouble*`) spawn automatically.

**Spacing rules** (MirrorKit cannot move your geometry, so these are up to you; `Depth` is clamped to 1-10):

| Behind a mirror face | Keep at least | Example at `Depth` 1 / 3 |
| --- | --- | --- |
| Walls, floors, any map geometry | `Depth` + 3 cm (the orange box) | 4 / 6 cm |
| Another mirror back to back (no wall between) | `Depth` + 4 cm between the two faces, using the larger `Depth` | 5 / 7 cm |
| A wall between two back-to-back mirrors | `Depth` + 3 cm on each side of the wall | 36 cm wall: faces 44 / 48 cm apart |

Raising `Depth` later means re-checking these gaps: a deeper stack needs more room behind the face.

### Build your map so it works with the shared PAK

- **Use your own meshes and materials for structure.** Place only kit actors and kit mirror materials. A player without MirrorKit then still gets a playable map instead of holes in it.
- **Your PAK must not contain MirrorKit's files.** Cooking your map also writes cooked kit files to `Saved/Cooked/.../Content/Mods/MirrorKit/`; package only your own mod folder.
- **Keep the kit's enums out of native conversion.** If your project uses Blueprint nativization:
  - Add `/Game/Mods/MirrorKit/Blueprints/E_MirrorResolutionProfile` and `/Game/Mods/MirrorKit/Blueprints/E_MirrorStaggerMode` as `+ExcludedAssets` under `[BlueprintNativizationSettings]` in `Config/DefaultEditor.ini`.
  - Otherwise UE 4.25 can leave them out of the PAK.
  - Recook referring maps after changing this.
- Add MirrorKit as a dependency in your `mod.json`.

## Settings for many mirrors at once

Every mirror works with no extra setup: its defaults are the schedule tuned on MirrorMaze (half resolution, 90 updates per second). The mirror, `BP_MirrorKitSettings` and new profiles start from the **same** values, so placing a settings actor changes nothing until you edit it. The capture budget is the exception: it is off unless the map has a `BP_MirrorKitBudget`, and placing one turns it on (budget 2 captures per frame by default).

Two optional map actors, both in `Blueprints/` (drag each in at most once; they have no mesh or collision and players never see them):

| Actor | Sets | Applies to |
| --- | --- | --- |
| **`BP_MirrorKitSettings`** | the per-mirror performance settings below | mirrors with `SettingsSource` `MapDefault` (and `Profile` mirrors without a profile) |
| **`BP_MirrorKitBudget`** | the budget: `CaptureBudget`, `CaptureBudgetMaxMultiplier`, `MinUpdateRate`, `BudgetFocusDistance`, `BudgetFocusAngle` | **every** mirror, whatever its `SettingsSource` (the budget is shared by the whole map) |

Each mirror picks where its per-mirror settings come from with **`SettingsSource`** (category *MirrorKit › Settings*):

| `SettingsSource` | The mirror uses |
| --- | --- |
| `MapDefault` (default) | the map's `BP_MirrorKitSettings`; its own values if the map has none |
| `Profile` | the **MirrorProfile** data asset in its `Profile` slot; with no profile assigned, the map's settings actor |
| `Own` | its own values |

- **Whole map:** edit `BP_MirrorKitSettings` (or leave it out and keep the defaults).
- **Groups:** right-click in the Content Browser › *Miscellaneous › Data Asset* › `BP_MirrorProfile` to make a profile. Set `SettingsSource` to `Profile` on the group and pick it; editing the profile changes the whole group. Shipped: `Profiles/MP_MirrorMaze` (the MirrorMaze schedule, so a group keeps it if you change the map settings) and `Profiles/MP_HighDetail` (full resolution at 120 updates/s).
- **One-offs:** set `SettingsSource` to `Own` and edit that mirror.
- **Budget:** edit `BP_MirrorKitBudget`; mirrors, profiles and settings actors never carry it. Without a budget actor the budget is **off**. Placing one turns it on with its defaults (budget 2, max ×4, min 30 updates/s, focus 800 cm / 15°), visible and editable on the actor. To keep one mirror out of it (e.g. a showcase mirror), tick its `ExcludeFromBudget`.

Per-mirror performance settings: `Resolution`, `UpdateEveryNFrames`, `TargetUpdateRate`, `FocusUpdateRate`, `StaggerMode`, `DistanceStaggerStart`, `DistanceThrottle`, `DistanceThrottleStart`, `FarUpdateEveryNFrames`, `CaptureDistance`, `MaxViewDistance`, `VisibilityTolerance`, `OffscreenUpdateEveryNFrames`, `ChecksEveryNFrames`, `DisableReflectionAO`. Look, body, depth, partner and tile settings stay on each mirror. Settings are resolved once, when the match starts. Two-sided back faces follow their front face.

**For a single sharp, every-frame mirror** (the pre-0.8.0 defaults), set `SettingsSource` `Own`, `Resolution` 1, `TargetUpdateRate` 0, `UpdateEveryNFrames` 1, `StaggerMode` None, `DisableReflectionAO` off, and `ExcludeFromBudget` on.

## Mirror quality for players and servers

Players and server hosts can lower mirror cost without changing the map. A **MirrorQuality** only ever lowers what the map set (caps), so the map's own choices stay intact; when several apply, each value takes the strictest.

| Who | How |
| --- | --- |
| Player | Launch option `-MirrorQuality=Low` (or `Medium`, `High`, `Off`) |
| Player, custom values | Launch options `-MirrorMaxResolution=0.4`, `-MirrorMaxRate=45` (updates per second), `-MirrorMaxDistance=1500` (cm), `-MirrorMaxCaptures=2` (captures per frame), `-MirrorNoAO`, `-MirrorsOff` |
| Player, custom preset | `-MirrorQuality=/Game/Mods/<Mod>/<Asset>`: any `BP_MirrorQuality` data asset in a PAK the player has |
| Server host (optional) | Map URL option `?MirrorQuality=Low`: sent to every player as a maximum |

| Preset (`Quality/`) | Resolution | Updates/s | Distance | Captures/frame | AO |
| --- | --- | --- | --- | --- | --- |
| `MQ_High` | map | map | map | map | map |
| `MQ_Medium` | ≤ 0.5 | ≤ 60 | ≤ 20 m | ≤ 3 | off |
| `MQ_Low` | ≤ 0.35 | ≤ 45 | ≤ 15 m | ≤ 2 | off |
| `MQ_Off` | mirrors show a still environment image; no live reflections | | | | |

Mirrors only render on players' machines: a dedicated server does no mirror rendering at all, so hosting costs no GPU. The server cap is for hosts who want every player on lighter settings. Custom presets and values are client-side; the server does not need them. A player's custom preset needs its PAK on that player's machine (whether Unchained lets a client load a PAK the server does not list is untested; the inline values need no PAK). An unknown preset name changes nothing. Quality is applied when the match starts (and on clients when the server cap arrives).

## Mirror settings

All settings are on every placed mirror. The performance defaults are the MirrorMaze schedule (kit 0.8.0; see above for the pre-0.8.0 sharp, every-frame values). With `SettingsSource` `MapDefault` or `Profile`, the per-mirror performance settings come from the settings actor or profile when there is one; the budget is not a mirror setting (see the budget table below).

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
| `Tiled` | off | The mirror is invisible and walk-through; tiles show its reflection |

**Partners and depth**

| Setting | Default | What it does |
| --- | --- | --- |
| `Partner` | none | Facing mirror for the tunnel effect; empty = independent mirror |
| `Depth` | 1 | Reflection levels with a partner, 1-10. Each level is another full render; above 3 is not recommended. Needs `Depth` + 3 cm clear behind the face ([spacing rules](#place-a-live-mirror)) |

**Resolution**

| Setting | Default | What it does |
| --- | --- | --- |
| `Resolution` | 0.5 | Overall scale. With the default profile, targets are 1280 × 720 at level 1 and 640 × 360 deeper. 0.5 is a good value for mirror-heavy maps |
| `ResolutionProfile` | FixedDeep | Deeper-level rule: `FixedDeep`, `Linear` or `Multiply` |
| `DeepResolutionScale` | 0.5 | `FixedDeep`: scale for every level after the first |
| `ResolutionDropPerLevel` | 0.15 | `Linear`: amount subtracted per level |
| `PerLevelResolutionMultiplier` | 0.5 | `Multiply`: factor applied per level |
| `MinimumDepthResolutionScale` | 0.125 | Lowest scale for deeper levels in any profile |

**Performance**

| Setting | Default | What it does |
| --- | --- | --- |
| `UpdateEveryNFrames` | 2 | Update every N frames (used when `TargetUpdateRate` is 0); skipped frames reuse the last reflection. 2 roughly halves capture work |
| `StaggerMode` | Distance | Spread updates across frames: `Distributed` (all mirrors) or `Distance` (mirrors at or beyond `DistanceStaggerStart`). Needed with N > 1 to keep worst-case frames smooth |
| `DistanceStaggerStart` | 1000 cm | Face distance where `Distance` staggering begins |
| `DistanceThrottle` | off | Use `FarUpdateEveryNFrames` at or beyond `DistanceThrottleStart` |
| `DistanceThrottleStart` | 1500 cm | Face distance where throttling begins |
| `FarUpdateEveryNFrames` | 4 | Update interval for throttled (distant) mirrors |
| `CaptureDistance` | 2500 cm | Stop updating beyond this face distance (the mirror keeps its last image) |
| `MaxViewDistance` | 0 | Limit how far each capture renders into the reflected scene; 0 = unlimited |
| `VisibilityTolerance` | 0.2 s | Stop updating when neither this mirror nor its partner has been drawn for this long |
| `OffscreenUpdateEveryNFrames` | 1 | While the whole face is outside your view: 1 unchanged, N > 1 every N frames, 0 frozen |
| `DisableReflectionAO` | on | Skip ambient occlusion inside this mirror's reflections (cheaper, rarely visible) |
| `TargetUpdateRate` | 90 (0 = off) | Target reflection updates per second; when set, replaces `UpdateEveryNFrames` with `round(FPS ÷ rate)` frames, at least 1 (e.g. 90: every 3 frames at 300 FPS, every frame at 60). Keep the rate well below your usual FPS: near 2 × rate, a mirror that switches to every frame can drag FPS down and stay there |
| `FocusUpdateRate` | 0 (off) | Updates per second for focused mirrors (near you or near the view centre, using the budget focus distance and angle). Keeps close reflections smooth at high uncapped FPS, where the normal rate holds each reflection for several frames; e.g. 1000 = every frame. Capped by a player MirrorQuality rate limit |
| `ChecksEveryNFrames` | 4 | Run the expensive per-mirror checks (distance, off-screen, focus) only on the mirror's due frames and otherwise every N frames; 1 = every frame (pre-0.5.3) |
| `ExcludeFromBudget` | off | Keep this mirror out of the map-wide budget: never counted and never slowed down by it |

**Budget** (map-wide, on `BP_MirrorKitBudget`; off without one; defaults below are the actor's)

| Setting | Default | What it does |
| --- | --- | --- |
| `CaptureBudget` | 2 (0 = off) | Target captures per frame across all mirrors; when the smoothed load is over it, every mirror's interval is stretched (mirrors with `ExcludeFromBudget` are left out) |
| `CaptureBudgetMaxMultiplier` | 4 | Largest interval stretch the budget may apply |
| `MinUpdateRate` | 30 (0 = off) | The capture budget never stretches a mirror below this many updates per second |
| `BudgetFocusDistance` | 800 cm | While the budget is stretching intervals, mirrors whose face is this close keep their normal interval (0 = off) |
| `BudgetFocusAngle` | 15° | While the budget is stretching intervals, mirrors whose centre is within this angle of your view direction (0 = off) |

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
| Tiled wall (front and back mirrors), `Depth` 1 | ~210 fps | ~2 ms |
| Baked tiles | 200-300 fps | < 1 ms |
| Screen-space floor/ceiling materials | 150-220 fps | ~1-3 ms |

## How a mirror decides to update

Each frame, a mirror goes through four steps:

1. **Schedule.** Every tick increments `FrameCounter`. An update is due when `(FrameCounter + phase) % N == 0`.
   - `N` is `UpdateEveryNFrames`, or `FarUpdateEveryNFrames` when throttled, or the off-screen interval, multiplied by the capture-budget multiplier (1 when `CaptureBudget` is off).
   - `phase` is 0 unless the mirror is staggered (or the budget multiplier is above 1); then it is `floor(PhaseHash × N)`. `PhaseHash` is computed once at start-up from the mirror's position (`Fraction(Sin(0.0129898X + 0.078233Y + 0.037719Z) × 43758.5453)`), which spreads grid-aligned mirrors evenly.
   - **Capture budget** (`CaptureBudget` > 0): mirrors report their captures to one shared owner mirror. Once a frame the owner smooths the total, and at most every 0.2 s steps a shared multiplier up (load over budget) or down (load at one step lower would stay within 90 % of budget), from 1 to `CaptureBudgetMaxMultiplier`. While the multiplier is above 1, *focused* mirrors (face within `BudgetFocusDistance`, or centre inside the `BudgetFocusAngle` view cone) keep their unstretched interval, so the mirrors you look at stay smooth.
2. **Eligibility.** All of these must hold:
   - you have a local camera;
   - the face distance is under `CaptureDistance`;
   - you are in front of the face;
   - the mirror (or its partner) was drawn within `VisibilityTolerance`.

   "Drawn" includes being seen inside another mirror. Distances check the centre distance first and compute the exact nearest point only when needed.
3. **Off screen.** If `OffscreenUpdateEveryNFrames` is not 1, the four face corners are tested against your view (with a 15% margin). Walls hiding a mirror in your view are not detected.
4. **Capture.** Level 1 is rendered (fitted to the visible part with `FitVisible`), then deeper partner levels.

Independent mirrors show other mirrors' images inside their reflection. Those images were rendered for your own viewpoint, so mirror-in-mirror repetitions are plausible but not geometrically exact. Partners give exact nested reflections, at the cost of one render per level.

`DisableReflectionAO` works through a stored post-process override (AO intensity 0) whose blend weight the checkbox raises at start-up.

A mirror's size and settings are read when play starts. Do not resize live mirrors at runtime.

## Partners and tunnels

- Set two facing mirrors as each other's `Partner` and raise `Depth` (usually 3) for an infinite-looking tunnel. Set relationships before play starts.
- Partners do not have to be parallel: each reflection is computed across each mirror's own plane.
- A mirror on the back of a wall is an ordinary mirror, so it takes its own `Partner`.
- **One partner per mirror.** Chains (A → B → C → A) get exact deeper reflections only into each mirror's own partner.

## Tiles and materials

**Breakable live tiles**

1. Place a `BP_CaptureMirror` covering the wall's front, and a second one back to back covering its back (facing the other way, spaced as in the [spacing rules](#place-a-live-mirror)). Turn on `Tiled` on both and keep `Depth` 1.
2. Place `BP_MirrorTileLive` tiles across them, centred between the two faces, with thickness about the gap. For example, scale (1.5, 0.06, 2.0) is a 150 × 200 cm tile for a 6 cm gap.
3. Set each tile's `Wall` to the front mirror and `BackWall` to the back mirror. A tile without `BackWall` shows plain metal on its back.

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
| `Blueprints/` | `BP_CaptureMirror`; map actors `BP_MirrorKitSettings`, `BP_MirrorKitBudget`; presets `BP_Mirror_Funhouse`, `BP_Mirror_Headless`, `BP_Mirror_StretchHead`, `BP_Mirror_Ghost`; tiles `BP_MirrorTileLive`, `BP_MirrorTile`, `BP_MirrorTile_NoSnow`; stand-ins `BP_MirrorDoubleFull`, `BP_MirrorDouble`, `BP_MirrorDoubleStretch` (spawned automatically); profile data asset class `BP_MirrorProfile`; quality data asset class `BP_MirrorQuality`; `BP_MirrorKitNet` (spawned by the server for `?MirrorQuality=`); enums `E_MirrorResolutionProfile`, `E_MirrorStaggerMode`, `E_MirrorSettingsSource` |
| `Quality/` | MirrorQuality presets `MQ_Off`, `MQ_Low`, `MQ_Medium`, `MQ_High` |
| `Profiles/` | MirrorProfiles `MP_MirrorMaze` (MirrorMaze schedule), `MP_HighDetail` |
| `Materials/` | `M_MirrorEnvironment`, `M_MirrorPolished` (screen-space only), `M_MirrorDarkCenter`, `M_MirrorCracked`; internal: `M_CaptureMirror`, `M_Matte` |
| `Meshes/` | `SM_MirrorTile`, `SM_MirrorTile_Cracked`; internal: `SM_MKCube`, `SM_MirrorTileCore` |
| `Textures/` | `TC_MK_DefaultEnvironment` (default cubemap) |

Release files: `MirrorKit.pak` (+ `.sha256`), `MirrorKit_Source.zip` (`Content/`, README, `mod.json`, license and third-party notices), `README.md`.

## For maintainers

**Reading the Blueprint.** Open `BP_CaptureMirror`. EventGraph routes BeginPlay to `InitializeMirror` and Tick to `UpdateMirror`; double-click the calls to follow the work. Coloured `MirrorKit |` comment blocks explain each function, and functions are grouped by category:

| Category | Functions |
| --- | --- |
| Lifecycle | `InitializeMirror`, `UpdateMirror` |
| Setup | `ResolveSettings` (first at BeginPlay), `ConfigureBackingAndDepth`, `ConfigureTileMode`, `InitializeCaptureResources`, `InitializeFirstCapture`, `InitializeDeepCapture`, `EnsurePlayerStandIn` |
| Scheduling | `ResolvePartner`, `AdvanceCaptureSchedule`, `CheckCaptureEligibility`, `IsWithinFaceDistance`, `DistanceToMirrorFace`, `UpdateOffscreenFlag` |
| Capture | `CaptureReflectionStack`, `CapturePortalFirst`, `CapturePortalDeep`, `CaptureLegacyView` |
| Projection | `ReflectViewAcrossPlane`, `ProjectMirrorRay`, `FitMirrorInterval`, `ComputeVisibleMirrorBounds` (pure maths) |
| Visibility | `RefreshCaptureHiddenActors`, `HideActorFromActiveCaptures`, `HidePartnerPanelsForCapture` |

**Diagnostics.** Each mirror counts its captures in `CaptureCount`. `TelemetryMark` is reserved for telemetry actors that read the counters. MirrorKit never reads either value.

**AgGraphTools.** [`Tools/AgGraphTools`](Tools/AgGraphTools) is the editor-only UE 4.25 plugin that MirrorLab's generator uses to build MirrorKit's Blueprint graphs from Python, a capability UE 4.25 Python lacks. Players, hosts and mappers do not need it. `Tools/annotate_blueprints.py` reapplies the comment blocks without changing logic.

## Changelog

| Version | Changes |
| --- | --- |
| 0.12.0 (internal) | • `FocusUpdateRate`: a faster update rate for focused (near or central) mirrors, against sideways jitter at high uncapped FPS. Off by default until tested. |
| 0.11.0 (internal) | • **MirrorQuality** for players and servers: presets (`MQ_Off`/`Low`/`Medium`/`High`, `BP_MirrorQuality` data assets), `-MirrorQuality=` launch option (preset name or asset path), inline `-Mirror*` caps, optional server `?MirrorQuality=` shared through `BP_MirrorKitNet`. Caps only lower the map settings. |
| 0.10.1 (internal) | • No `BP_MirrorKitBudget` in the map = budget off (mirrors no longer carry hidden default budget numbers). |
| 0.10.0 (internal) | • The budget is map-wide only: its five settings are no longer on mirrors. `BP_MirrorKitBudget` sets them on every mirror; without one the kit defaults apply (budget 2, max ×4, min 30 updates/s, focus 800 cm / 15°).<br>• New per-mirror `ExcludeFromBudget`. |
| 0.9.0 (internal) | • **Removed the generated back face:** `TwoSided`, `TwinGap`, `TwinGapWasAdjusted`, `BackPartner`, `BackPartnerBackFace`, `PartnerBackFace` and the `BP_Mirror_TwoSided` preset. Place a second mirror on the back instead (spacing rules in *Place a live mirror*).<br>• `BP_MirrorTileLive` gets `BackWall` for its back face (was the front mirror's generated twin). |
| 0.8.0 (internal) | • Budget moved to its own map actor **`BP_MirrorKitBudget`**, applied to every mirror (also `Profile` and `Own`); `BP_MirrorKitSettings` and profiles carry only per-mirror settings.<br>• **Defaults changed:** mirrors, both actors and profiles default to the MirrorMaze schedule (`Resolution` 0.5, `TargetUpdateRate` 90, `StaggerMode` Distance, `DisableReflectionAO` on, `CaptureBudget` 2, `MinUpdateRate` 30). Placed mirrors that kept the old defaults switch to these. |
| 0.7.1 (internal) | • Profile `MP_MirrorMaze`: the MirrorMaze per-mirror schedule for a group of mirrors. |
| 0.7.0 (internal) | • Settings hierarchy: map-wide `BP_MirrorKitSettings` actor (defaults = recommended schedule), reusable `BP_MirrorProfile` data assets, per-mirror `SettingsSource` (`MapDefault` / `Profile` / `Own`). Existing maps without a settings actor behave as before.<br>• Details-panel categories and tooltips (*MirrorKit › Settings / Performance / Budget / Look*, internal state under *MirrorKit › Internal*). |
| 0.5.5 (internal) | • `TargetUpdateRate` and `MinUpdateRate` round instead of floor (a 120 Hz target dropped to every frame below 240 FPS and held FPS there; 59.9 FPS ÷ 30 floored to 1). |
| 0.5.4 (internal) | • Time-based scheduling: `TargetUpdateRate` and `MinUpdateRate` (both off by default), so mirrors stay smooth at low or capped frame rates and the budget backs off where it cannot help. |
| 0.5.3 (internal) | • Lean tick: `ChecksEveryNFrames` (default 4) skips the expensive checks on frames where a mirror is not due. Measured reason: on MirrorMaze the per-tick Blueprint checks were the bottleneck (turning the off-screen test off gave +32 FPS walking).<br>• The focus test runs only when focus is configured. |
| 0.5.2 (internal) | • Focus-aware budget: `BudgetFocusDistance` / `BudgetFocusAngle` keep near and central mirrors at their normal pace.<br>• Budget steps back down when the lower step fits the budget (was 90 %, which could stick at ×2). |
| 0.5.1 (internal) | • Adaptive `CaptureBudget` (default off) with `CaptureBudgetMaxMultiplier`.<br>• Better stagger spread: the phase hash is computed once at start-up and no longer clusters on grid-aligned maps (old hash put 193 of 258 MirrorMaze mirrors on one phase at N = 2). |
| 0.5.0 | Performance and usability release (developed as internal builds after 0.4.0):<br>• Independent mirrors: no pairing needed; one capture per face.<br>• Cheaper per-frame work: checks before hidden-list maintenance, cached stand-ins, shared Blueprint functions, bounded loops (behaviour verified identical) and editor comment blocks.<br>• Distances measured to the nearest point of the mirror face, centre-first and only when a feature uses them.<br>• Exact update-slot formula `(FrameCounter + phase) % N`.<br>• New settings `OffscreenUpdateEveryNFrames` and `DisableReflectionAO`; two-sided back faces inherit them.<br>• `CaptureCount` / `TelemetryMark` diagnostics.<br>• README rewritten; map-design guide `docs/MAP-OPTIMIZATION.md`. |
| 0.4.0 | Front/back partner targeting for two-sided mirrors; spacing safeguards (`Depth` 1-10, `TwinGap` minimum, clearance guide); default depth 1 |
| 0.3.0 | First public release |

## License

Original MirrorKit work is licensed under the MIT License. See `LICENSE` and `THIRD_PARTY_NOTICES.md`.
