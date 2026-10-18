# Making a map with MirrorKit

Everything a map maker needs: installing the kit in ArgonSDK, placing mirrors, settings for many mirrors at once, the capture budget, breakable tiles and every mirror setting. Players and server hosts only need [the quality options](QUALITY.md). For mirror-heavy maps, also read [designing maps for mirror performance](MAP-OPTIMIZATION.md).

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
  - Add `/Game/Mods/MirrorKit/Blueprints/E_MirrorResolutionProfile`, `/Game/Mods/MirrorKit/Blueprints/E_MirrorStaggerMode` and `/Game/Mods/MirrorKit/Blueprints/E_MirrorSettingsSource` as `+ExcludedAssets` under `[BlueprintNativizationSettings]` in `Config/DefaultEditor.ini`.
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
- **Budget:** edit `BP_MirrorKitBudget`; mirrors, profiles and settings actors never carry it. At runtime a hidden `BP_MirrorKitManager` (spawned automatically, nothing to place) applies it. Without a budget actor the budget is **off**. Placing one turns it on with its defaults (budget 2, max ×4, min 30 updates/s, focus 800 cm / 15°), visible and editable on the actor. To keep one mirror out of it (e.g. a showcase mirror), tick its `ExcludeFromBudget`.

Per-mirror performance settings: `Resolution`, `UpdateEveryNFrames`, `TargetUpdateRate`, `FocusUpdateRate`, `StaggerMode`, `DistanceStaggerStart`, `CaptureDistance`, `MaxViewDistance`, `VisibilityTolerance`, `ChecksEveryNFrames`, `DisableReflectionAO`, `SmoothUpdateInterval`, `AnchorUpdateCadence`. Look, body, depth, partner and tile settings stay on each mirror. Settings are resolved when the match starts; player and server quality choices are applied on top (again if the server options arrive later).

**For a single sharp, every-frame mirror** (the defaults before 0.6.0), set `SettingsSource` `Own`, `Resolution` 1, `TargetUpdateRate` 0, `UpdateEveryNFrames` 1, `StaggerMode` None, `DisableReflectionAO` off, and `ExcludeFromBudget` on.

## How the settings pieces fit

| Piece | Who makes it | Job |
| --- | --- | --- |
| `BP_MirrorKitSettings` | map maker (placed, one per map) | default look and schedule for every `MapDefault` mirror |
| `BP_MirrorProfile` | map maker (data asset) | the same settings for a group of mirrors (`SettingsSource` = `Profile`) |
| `BP_MirrorKitBudget` | map maker (placed, optional) | budget rules; without it the budget is off |
| `BP_MirrorQuality` (`MQ_*`) | kit / anyone (data assets) | MirrorQuality presets for players and servers |
| `BP_MirrorKitManager` | spawned automatically by the first mirror, every machine | reads the budget actor and the quality options once, smooths the FPS and runs the budget each frame |
| `BP_MirrorKitNet` | spawned automatically on a server whose map options include mirror options | carries the server options to clients |

Each mirror: own values, then `BP_MirrorKitSettings` or its profile (`ResolveSettings`), then the manager's quality layers and budget rules; afterwards it only re-reads when the manager reports a change.

## Mirror settings

All settings are on every placed mirror. The performance defaults are the MirrorMaze schedule (see above for the sharp, every-frame values used before 0.6.0). With `SettingsSource` `MapDefault` or `Profile`, the per-mirror performance settings come from the settings actor or profile when there is one; the budget is not a mirror setting (see the budget table below).

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
| `CaptureDistance` | 2500 cm | Stop updating beyond this face distance (the mirror keeps its last image) |
| `MaxViewDistance` | 0 | Limit how far each capture renders into the reflected scene; 0 = unlimited |
| `VisibilityTolerance` | 0.2 s | Stop updating when neither this mirror nor its partner has been drawn for this long |
| `DisableReflectionAO` | on | Skip ambient occlusion inside this mirror's reflections (cheaper, rarely visible) |
| `TargetUpdateRate` | 90 (0 = off) | Target reflection updates per second; when set, replaces `UpdateEveryNFrames` with `round(FPS ÷ rate)` frames, at least 1 (e.g. 90: every 3 frames at 300 FPS, every frame at 60). Below about 1.5 × the rate (135 FPS for 90/s) every update is every frame |
| `FocusUpdateRate` | 144 (0 = off) | Updates per second for focused mirrors: face within 800 cm or centre within 15° of your view (the budget actor's `BudgetFocusDistance`/`BudgetFocusAngle` when one is placed). Keeps close reflections smooth at high uncapped FPS, where 90/s holds each reflection for several frames. Costs nothing below about 135 FPS (90/s is already every frame there); 1000 = every frame. Players can change it with MirrorQuality (`-MirrorFocusRate=`); a server cap can limit it |
| `ChecksEveryNFrames` | 4 | Run the expensive per-mirror checks (distance, focus) only on the mirror's due frames and otherwise every N frames; 1 = every frame |
| `SmoothUpdateInterval` | on | Base the update interval on the FPS averaged over ~0.5 s and keep it until the FPS clearly moves, so it does not flip with frame-time noise. Removes most jitter at fluctuating or uncapped FPS (measured: about 0-3 % FPS uncapped, none at a 240 cap) |
| `AnchorUpdateCadence` | off | Optional: count each interval from the mirror's last update, so a changed interval never produces one short or long gap (off = fixed slots `(FrameCounter + phase) % N`). No visible gain over smoothing alone in our tests and about 2-5 % FPS uncapped, so it is off by default; worth trying on PCs where smoothing alone still judders |
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

## How a mirror decides to update

Each frame, a mirror goes through three steps:

1. **Schedule.** Every tick increments `FrameCounter`. An update is due when `(FrameCounter + phase) % N == 0`; with `AnchorUpdateCadence` (optional), after the first update it is due when `N` frames have passed since the last one.
   - `N` is `round(FPS ÷ TargetUpdateRate)` (at least 1; with `SmoothUpdateInterval` the FPS is averaged over ~0.5 s and `N` changes only when the ratio is 0.65 or more away from it), or `UpdateEveryNFrames` when the rate is 0. Focused mirrors use `FocusUpdateRate` instead when that gives a shorter interval. Unfocused mirrors are then multiplied by the capture-budget multiplier (1 when `CaptureBudget` is off), never below `MinUpdateRate`.
   - `phase` is 0 unless the mirror is staggered (or the budget multiplier is above 1); then it is `floor(PhaseHash × N)`. `PhaseHash` is computed once at start-up from the mirror's position (`Fraction(Sin(0.0129898X + 0.078233Y + 0.037719Z) × 43758.5453)`), which spreads grid-aligned mirrors evenly.
   - **Capture budget** (`CaptureBudget` > 0): mirrors report their captures to the hidden `BP_MirrorKitManager`. Once a frame it smooths the total, and at most every 0.2 s steps a shared multiplier up (load over budget) or down (the load at one step lower would fit the budget), from 1 to `CaptureBudgetMaxMultiplier`. While the multiplier is above 1, *focused* mirrors (face within `BudgetFocusDistance`, or centre inside the `BudgetFocusAngle` view cone) keep their unstretched interval, so the mirrors you look at stay smooth.
2. **Eligibility.** All of these must hold:
   - you have a local camera;
   - the face distance is under `CaptureDistance`;
   - you are in front of the face;
   - the mirror (or its partner) was drawn within `VisibilityTolerance`.

   "Drawn" includes being seen inside another mirror. Distances check the centre distance first and compute the exact nearest point only when needed.
3. **Capture.** Level 1 is rendered (fitted to the visible part with `FitVisible`), then deeper partner levels.

Independent mirrors show other mirrors' images inside their reflection. Those images were rendered for your own viewpoint, so mirror-in-mirror repetitions are plausible but not geometrically exact. Partners give exact nested reflections, at the cost of one render per level.

`DisableReflectionAO` works through a stored post-process override (AO intensity 0) whose blend weight the checkbox raises at start-up.

A mirror's size, settings, position and facing are read when play starts (the camera is read once per tick). Do not resize live mirrors at runtime; after moving or turning one at runtime, call its `RefreshMirrorTransform`.

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

## Contents

| Path (`/Game/Mods/MirrorKit/`) | Assets |
| --- | --- |
| `Blueprints/` | `BP_CaptureMirror`; map actors `BP_MirrorKitSettings`, `BP_MirrorKitBudget`; runtime `BP_MirrorKitManager` (spawned automatically by the first mirror); presets `BP_Mirror_Funhouse`, `BP_Mirror_Headless`, `BP_Mirror_StretchHead`, `BP_Mirror_Ghost`; tiles `BP_MirrorTileLive`, `BP_MirrorTile`, `BP_MirrorTile_NoSnow`; stand-ins `BP_MirrorDoubleFull`, `BP_MirrorDouble`, `BP_MirrorDoubleStretch` (spawned automatically); profile data asset class `BP_MirrorProfile`; quality data asset class `BP_MirrorQuality`; `BP_MirrorKitNet` (spawned on a server whose map options include mirror options); enums `E_MirrorResolutionProfile`, `E_MirrorStaggerMode`, `E_MirrorSettingsSource` |
| `Quality/` | MirrorQuality presets `MQ_Off`, `MQ_Low`, `MQ_Medium`, `MQ_High`, `MQ_Ultra` |
| `Profiles/` | MirrorProfiles `MP_MirrorMaze` (MirrorMaze schedule), `MP_HighDetail` |
| `Materials/` | `M_MirrorEnvironment`, `M_MirrorPolished` (screen-space only), `M_MirrorDarkCenter`, `M_MirrorCracked`; internal: `M_CaptureMirror`, `M_Matte`, `M_MirrorOff` (mirrors switched off) |
| `Meshes/` | `SM_MirrorTile`, `SM_MirrorTile_Cracked`; internal: `SM_MKCube`, `SM_MirrorTileCore` |
| `Textures/` | `TC_MK_DefaultEnvironment` (default cubemap) |

Release files: `MirrorKit.pak` (+ `.sha256`), `MirrorKit_Source.zip` (`Content/`, README, `mod.json`, license and third-party notices), `README.md`.

## Reading the Blueprints

Open `BP_CaptureMirror`. EventGraph resolves the settings, finds the manager and applies its quality, then routes to `InitializeMirror`; Tick goes to `UpdateMirror`; double-click the calls to follow the work. Each function's tooltip says what it does; inside larger functions, comment boxes mark each step (what that group of nodes does). `BP_MirrorKitManager`, the tile and the stand-in Blueprints follow the same style. Functions are grouped by category:

| Category | Functions |
| --- | --- |
| Lifecycle | `InitializeMirror`, `UpdateMirror`, `SyncWithManager` |
| Setup | `ResolveSettings` (first at BeginPlay), `SnapshotQualityBase`, `EnsureManager`, `ApplyManagerQuality`, `ReadManagerBudget`, `ApplyQualityValues`, `ApplyQualityCaps`, `ApplyMirrorsOff`, `RefreshMirrorTransform`, `ConfigureBackingAndDepth`, `ConfigureTileMode`, `InitializeCaptureResources`, `InitializeFirstCapture`, `InitializeDeepCapture`, `EnsurePlayerStandIn` |
| Scheduling | `ResolvePartner`, `UpdateCaptureBudget`, `AdvanceCaptureSchedule`, `CheckCaptureEligibility`, `IsWithinFaceDistance`, `DistanceToMirrorFace` |
| Capture | `CaptureReflectionStack`, `CapturePortalFirst`, `CapturePortalDeep`, `CaptureLegacyView` |
| Projection | `ReflectViewAcrossPlane`, `ProjectMirrorRay`, `FitMirrorInterval`, `ComputeVisibleMirrorBounds` (pure maths) |
| Visibility | `RefreshCaptureHiddenActors`, `HideActorFromActiveCaptures`, `HidePartnerPanelsForCapture` |

`BP_MirrorKitManager`: `ApplyServerOptions` (parses the quality layers), `ResolveBudget` (reads `BP_MirrorKitBudget`), and a Tick that smooths the FPS and steps the budget multiplier.

**Diagnostics.** Each mirror counts its captures in `CaptureCount`; MirrorKit never reads it.
