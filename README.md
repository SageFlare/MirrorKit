# MirrorKit

Mirrors for Chivalry 2 Unchained maps: live reflections that show players, cheap shiny surfaces and breakable mirror tiles. MirrorKit is a **library**: it adds no map by itself. Maps use its assets and require it to be installed.

Version 0.4.0 (optional slower updates for mirrors outside the player's view). Built with [ArgonSDK](https://github.com/Chiv2-Community/ArgonSDK) (Unreal Engine 4.25). The optional [MirrorLab](https://github.com/SageFlare/MirrorLab) map demonstrates every mirror type.

## What you need

| You are | Install | Where |
| --- | --- | --- |
| **Player** joining a server | `MirrorKit.pak` **and** the map's own PAK | Through the [Unchained Launcher](https://github.com/Chiv2-Community/UnchainedLauncher) Mod Manager, or copy both into `Chivalry 2/TBL/Content/Paks/`. Launch in Unchained multiplayer mode. |
| **Server host** | `MirrorKit.pak` **and** the map's PAK | The server's `TBL/Content/Paks/` (or install both mods in the launcher before "Launch Headless"). The server does no mirror work; it only needs the files so the map's mirror actors load. |
| **Mapper** (building a map) | `MirrorKit_Source.zip` and `MirrorKit.pak` | Source: see [Using MirrorKit in your map](#using-mirrorkit-in-your-map). PAK: your game's Paks folder, to test your map. |

Without `MirrorKit.pak`, a map that uses it still loads if it was built correctly (see below), but every mirror, tile and stand-in is missing.

Unchained `mod.json` for MirrorKit: `"mod_type": "Shared"`, `"tags": ["Assets"]`. A map that uses MirrorKit should list it under `"dependencies"`.

## The mirrors

All live mirrors are `BP_CaptureMirror` or one of its presets. They differ only in their settings; every setting can be changed on any placed mirror.

| Asset | What you see | Cost |
| --- | --- | --- |
| `BP_CaptureMirror` | True reflection: your full body (moving legs, head, weapon in hand), other players, the room. With a facing `Partner`, an infinite-looking tunnel (3 levels deep) | High |
| `BP_Mirror_Funhouse` | Magnified, gappy carnival look | High |
| `BP_Mirror_Headless` | Your reflection has no head (others look normal) | High |
| `BP_Mirror_StretchHead` | Your head stretches as you look up and down | High |
| `BP_Mirror_Ghost` | Your body is invisible; your weapons float | High |
| `BP_Mirror_TwoSided` | Reflects on both faces (e.g. a mirror wall standing in the middle of a room) | High (only the face you look at renders) |
| `BP_MirrorTileLive` + a `Tiled` mirror | A wall of breakable tiles showing a live reflection on both faces; break one and walk through the hole | Medium (one reflection level) |
| `BP_MirrorTile` / `BP_MirrorTile_NoSnow` | Breakable tiles with a baked (still) reflection; the snow variant plays a wind sound when broken | Very low |
| `M_MirrorEnvironment` | Material: shiny metal with a baked image of the room; also live screen-space reflections at shallow angles | Very low |
| `M_MirrorDarkCenter` | Material: reflects at shallow angles, black when looking straight at it | Very low |

Every effect is per viewer: a ghost or headless mirror changes only *your own* reflection as *you* see it. Other players see you normally in their mirrors.

## Performance

Live mirrors render the scene again from the mirror's point of view: one extra render per active reflection level per eligible face. With no valid, distinct partner, only level 1 is allocated and captured, even if requested `Depth` is higher. A valid explicit partner permits the requested depth (1 to 10). Eligibility also uses capture distance, facing direction, cadence and recent-render timestamps; those timestamps can include other captures, so they are not a guarantee of visibility in the player's main view.

### Independent mirrors (0.3.1)

Leave `Partner` and `BackPartner` empty for independent front/back reflections. No manual pairing is required. Empty references stay empty; a reference to the mirror itself is treated as unpartnered for rendering. Each independent face allocates one render target and performs at most one capture per eligible update. Requested `Depth`, physical backing and clearance are preserved; runtime `EffectiveDepth` records the allocated depth.

Other mirrors' displayed images remain visible inside an independent capture. This preserves the existing approximate mirror-within-mirror feedback, including delayed or perspective-inaccurate repetitions. It does not enforce a global visible-bounce limit. Only this actor's own reflective panels are hidden from its independent capture.

Explicit partners remain available for the existing calculated nested-view method, including back-face resolution and presets. Set relationships before BeginPlay; adding a partner to a mirror initialized independently requires a reload to allocate deeper targets. Loss of a partner stops deeper capture execution, but existing allocated targets are retained until the actor ends play. Live relationship rewiring and hidden-list cleanup are not part of this release.

### Face distance (0.3.6)

`CaptureDistance`, `DistanceStaggerStart` and `DistanceThrottleStart` measure from the camera to the **nearest point of the mirror face**, not to the mirror's centre. In front of the mirror this is the straight-line distance to the glass; to the side of it, the distance to the closest edge. Wide mirrors therefore stay live while you stand near one end or walk along the face, where 0.3.5 measured to the centre and could freeze a long wall at about half its width closer than the setting. A viewer exactly in the mirror's plane, or behind it, still never captures. Since 0.3.7 each test first uses the centre distance: the face distance is never larger than it, and never smaller than the centre distance minus half the mirror's diagonal. A mirror whose centre is already inside the limit, or farther than the limit plus half its diagonal, is decided from the centre distance alone; only mirrors in between compute the exact nearest point (clamp the camera's offset to the face rectangle along the mirror's right and up axes). `CaptureDistance` captures strictly inside the limit; distance stagger and throttle apply at or beyond their start distance.

Since 0.3.8 the centre distance is measured once per frame and the mirror's half diagonal once at BeginPlay. Stagger and throttle distance tests run only when `StaggerMode` is `Distance` or `DistanceThrottle` is on, so a mirror using neither pays one distance per frame (as before 0.3.6). A mirror's size is read when play starts; do not resize live mirrors at runtime (render targets, back faces and the distance cache are all set up at BeginPlay).

### Update slots (0.3.7)

Each tick increments `FrameCounter`; a capture is due when `(FrameCounter + phase) % N == 0`, where `N` is `UpdateEveryNFrames` (or `FarUpdateEveryNFrames` when throttled) and `phase` is 0 unless staggered. Staggered mirrors use `phase = |round(X + 3Y + 7Z)| % N` from the actor location. Versions 0.3.4-0.3.6 evaluated the counter one frame ahead (kept then for exact equivalence with earlier builds); 0.3.7 removes that offset. Update frequency, staggering and cost are unchanged; every mirror simply updates one frame later in its cycle.

### Off-screen updates (0.4.0)

A mirror can stay live while you cannot see it directly, because it appears inside another mirror's reflection (for example two mirrors meeting in a corner). `OffscreenUpdateEveryNFrames` controls those mirrors:

- `1` (default): no change; off-screen mirrors keep their normal cadence.
- `N > 1`: while the whole mirror face is outside your view, it updates every N frames (or its normal interval, if that is already longer).
- `0` or lower: while the whole face is outside your view, it stops updating and keeps its last image; it resumes as soon as any part enters your view.

"Outside your view" is a frustum test on the four face corners against the rendered view's field of view with a 15% margin, so mirrors at the screen edge still count as visible. A mirror inside your view but hidden behind a wall still counts as visible (Unreal does not report main-view occlusion to Blueprints). The test runs only when the setting is not 1.

### Diagnostics (0.3.9)

Each live mirror counts its submitted scene captures in `CaptureCount` (one integer add per capture; no settings, no visual change). `TelemetryMark` is reserved for telemetry actors that read the counters, for example to count how many different mirrors captured in a time window. MirrorKit itself never reads either value.

### Tick maintenance (0.3.2)

Cadence, camera, distance, facing and recent-render checks now precede hidden-list maintenance. Only allocated `EffectiveDepth` levels receive player/stand-in hiding updates. Stand-in actor references are cached and refreshed at most once per game second while eligible; a late-spawned alternative stand-in may take up to one second to be discovered. Player components are still refreshed on every eligible capture update to preserve respawn/equipment handling. Self components are hidden once at BeginPlay instead of re-adding ten self-panel entries on every independent capture. Explicit-partner panel selection still updates before each capture. Capture resolution, depth, cadence and visibility defaults are unchanged; all ten component slots remain for compatibility.

Measured in MirrorLab on one PC, solo, frame rate uncapped (empty hub: about 240 fps which was capped at 240). Use these as relative guidance; your numbers will differ, and more players inside a mirror's view add cost.

| Setup | Frame rate | Approx. extra per frame |
| --- | --- | --- |
| Facing pair, `Depth` 3, both in view | 120-200 fps | 2-5 ms |
| Floor + ceiling tunnel, `Depth` 3 | ~170 fps | ~3 ms |
| Two-sided tiled wall, `Depth` 1 | ~210 fps | ~2 ms |
| Baked tiles | 200-300 fps | < 1 ms |
| Screen-space floor/ceiling materials | 150-220 fps | ~1-3 ms (engine reflections, view dependent) |

**Designing a map with many mirrors?** Read [Designing maps for mirror performance](docs/MAP-OPTIMIZATION.md). It has measured guidance on baked lighting (the biggest single gain: about +23-31% FPS in the test maze, because every mirror capture otherwise recalculates movable lights), using fewer and longer mirrors, staggered updates and the settings below.

### Performance controls

Live mirrors render the scene again for every active reflection level. The following settings reduce that work, with different visual tradeoffs:

- Keep `Depth` at its default `1` unless a partnered tunnel needs deeper bounces. `Depth` can go up to 10, but every level is another scene render; values above 3 are allowed, not tested, and not recommended.
- Lower `Resolution` to reduce all render-target pixels. Choose `ResolutionProfile` `FixedDeep` for one fixed deeper-level scale, `Linear` to subtract a fixed amount per level, or `Multiply` to repeatedly multiply each level. Smaller deep targets improve GPU cost but can make distant bounces softer or more pixelated.
- Lower `CaptureDistance` to stop the complete mirror capture when the player is farther away. This usually gives the largest saving when distant mirrors do not need to remain live.
- Set a positive `MaxViewDistance` to omit distant geometry inside the reflected scene. This can especially help multi-level partner reflections, but a value shorter than the room or partner spacing can remove walls, mirrors or deeper bounces. `0` keeps Unreal's default/unlimited range.
- Raise `UpdateEveryNFrames` to reuse the previous reflection between captures. `2` updates every second frame, roughly halving capture frequency; higher values save more but make player, weapon and camera motion in the reflection less fluid. Values `1` or lower update every frame.
- Choose `StaggerMode` `Distributed` to spread every eligible static mirror across stable update phases, or `Distance` to stagger only mirrors beyond `DistanceStaggerStart`. The default `None` keeps mirrors on the same phase. Staggering smooths frame-time spikes but does not reduce total captures.
- Enable `DistanceThrottle` to switch mirrors beyond `DistanceThrottleStart` from `UpdateEveryNFrames` to `FarUpdateEveryNFrames`. This actually reduces distant capture work, at the cost of less-fluid distant reflections.
- Lower `VisibilityTolerance` to stop captures sooner after both the mirror and its partner leave the rendered view. Very small values may cause updates to stop during brief visibility gaps or occlusion.
- Avoid placing many live mirrors in the same sight line. Use baked breakable tiles or environment/SSR materials for decorative mirrors that do not need a live player reflection.

## Using MirrorKit in your map

### Install the kit in ArgonSDK

1. Unzip `MirrorKit_Source.zip`.
2. Copy its `Content` folder's contents to **`ArgonSDK/Content/Mods/MirrorKit/`** (so you get `ArgonSDK/Content/Mods/MirrorKit/Blueprints/BP_CaptureMirror.uasset`).
   - This exact location matters: Unreal names assets by folder (`/Game/Mods/MirrorKit/...`), the kit's assets reference each other by those names, and `MirrorKit.pak` loads them at the same names in game. Do not move or rename kit assets in the editor; your map would then point at assets the shared `MirrorKit.pak` does not contain. (The files can physically live elsewhere if you link the folder into place.)
3. Copy `MirrorKit.pak` into your game's `TBL/Content/Paks/` to test.

### Build your map so it works with the shared PAK

- **Your map's own walls, floors and props should not typically use kit assets.** Use your own meshes/materials for structure; only place kit actors and kit mirror materials. Then a player missing MirrorKit still gets a playable map instead of areas missing entirely and creating pitfalls to the void.
- **Your PAK must not contain MirrorKit's files.** Cooking your map also writes cooked kit files to `Saved/Cooked/.../Content/Mods/MirrorKit/`; package only your own mod folder.
- **Keep runtime control enums out of native conversion.** In projects using Blueprint nativization, add `/Game/Mods/MirrorKit/Blueprints/E_MirrorResolutionProfile` and `/Game/Mods/MirrorKit/Blueprints/E_MirrorStaggerMode` as `+ExcludedAssets` under `[BlueprintNativizationSettings]` in `Config/DefaultEditor.ini`. Otherwise UE4.25 may omit them from the PAK expecting generated native code. Keep both enum `.uasset/.uexp` pairs in MirrorKit's cooked package; recook referring maps after correcting this setting. The current workspace has these two exclusions without disabling base-game nativization.
- In your `mod.json`, add MirrorKit as a dependency.

### Placing a live mirror

1. Drag `BP_CaptureMirror` (or a preset) into the level. The mirror's **local +X direction points out of its reflective face**; the face is at the actor's origin. Switch the viewport transform gizmo from **World** to **Local** to see this direction: in World mode the red X arrow stays aligned to the world even when the mirror rotates. Turn a wall mirror with **Rotation Z (Yaw)**, not Rotation X.
2. Size it with the actor **scale**: Y x 100 = width in cm, Z x 100 = height in cm, keep X = 1. Example: scale (1, 12, 6) is a 12 m x 6 m wall mirror. For a floor mirror, rotate it to face up.
3. **Independent mirrors:** leave partners empty. They still capture any other visible mirror images. For the optional calculated paired tunnel, assign each mirror's `Partner` to the other and raise `Depth` as needed.
4. **Leave empty space behind the face: `Depth` + 3 cm (4 cm at the default `Depth` 1; 6 cm at `Depth` 3).** The reflection layers live there; a wall or floor flush with the mirror flickers in the reflection. Mount mirrors in front of walls, not inside them.
5. **Mirrors are solid:** the backing plate blocks players and weapons (except `Tiled` mirrors, where the tiles collide). The back of a one-sided mirror shows plain metal; use `TwoSided` for a free-standing mirror wall, or a real wall behind it.  Mirrors can be used as walls as well.
6. Nothing else to place. The stand-ins that draw your own body in reflections (`BP_MirrorDouble*`) are spawned by the mirrors when the game starts; do not place them.

### Spacing safeguards

Two different clearances matter:

- **Internal two-sided spacing is automatic.** `TwinGap` is clamped to at least `Depth + 4` by the Construction Script and again at runtime. Increasing `Depth` therefore moves the generated back face far enough away automatically. If an entered gap is too small, the Details value is raised and UE prints a warning. `TwinGapWasAdjusted` records that correction.
- **External map clearance is not automatic.** MirrorKit cannot move your walls, floors or props. Leave `Depth + 3 cm` clear behind the mirror face along its **local -X axis**. Selecting a mirror in the editor shows an orange wireframe `ClearanceGuide` covering this required volume; keep blocking geometry outside it. The guide has no collision and is hidden in game.

`Depth` is clamped to the supported range 1-10. These safeguards prevent internal face overlap and expose external placement mistakes, but they cannot automatically relocate map geometry.

### Mirror settings

| Setting | Default | What it does |
| --- | --- | --- |
| `Partner` | (itself) | Facing mirror for the tunnel effect |
| `PartnerBackFace` | off | Target `Partner`'s generated back face instead of its front. The target must have `TwoSided` enabled |
| `Depth` | 1 | Reflection levels, 1-10. Use 1 for a single mirror and usually 3 for a partnered tunnel; values above 3 are not recommended. Each level costs one extra render while visible |
| `HeadSource` | 3 | Your body in the reflection: 3 full body; 1 head and torso only (stiff head); 2 stretching head; 0 headless |
| `HideBody` | off | Ghost: hide your character from this mirror |
| `TwoSided` | off | Also reflect on the back face |
| `TwinGap` | 6 cm | Distance between the two faces of a two-sided mirror (runtime minimum `Depth` + 4, or 7 cm at `Depth` 3). Use 2 x (`Depth` + 2) or more with the funhouse look (10 at `Depth` 3) |
| `BackPartner` | none | Partner for the back face |
| `BackPartnerBackFace` | off | Make this mirror's back face target `BackPartner`'s back face instead of its front |
| `PortalMode` | on | Off = the old screen-mapped look (funhouse) |
| `MatchViewFov` | on | Off = funhouse magnification |
| `FitVisible` | on | Renders only the visible part of the mirror (sharper up close) |
| `ScreenLevel1` | off | Sharper big **floor** mirrors when looking straight down. Use with `ClipBehind` off, and only if nothing is below the mirror |
| `ClipBehind` | on | Hides geometry behind the mirror (used by the screen-mapped look) |
| `Tiled` | off | The mirror is invisible and walk-through; tiles show its reflection (see below) |
| `Resolution` | 1 | Overall multiplier applied after the selected resolution profile. With the default `FixedDeep` settings, targets are 1280 x 720 at level 1 and 640 x 360 at deeper levels |
| `ResolutionProfile` | FixedDeep | Deep-level resolution rule: `FixedDeep`, `Linear`, or `Multiply` |
| `DeepResolutionScale` | 0.5 | `FixedDeep`: multiplier used by every level after level 1. The default reproduces the original 640 x 360 deep targets |
| `ResolutionDropPerLevel` | 0.15 | `Linear`: amount subtracted from the resolution scale for each successive level |
| `PerLevelResolutionMultiplier` | 0.5 | `Multiply`: multiplier applied again for every successive level. `0.5` halves each level |
| `MinimumDepthResolutionScale` | 0.125 | Lowest scale permitted for levels after level 1 in any profile |
| `CaptureDistance` | 2500 cm | Maximum camera distance to the nearest point of the mirror face at which this mirror updates. Lower it to stop the entire capture sooner and save performance |
| `MaxViewDistance` | 0 | Maximum distance each capture renders into the reflected scene. `0` uses Unreal's normal unlimited/default range; a lower positive limit can save performance but removes distant objects from the reflection |
| `UpdateEveryNFrames` | 1 | Capture cadence. `1` or lower updates every frame; `2` updates every second frame, and so on. Skipped frames reuse the previous reflection |
| `StaggerMode` | None | Update phase scheduling: `None`; `Distributed` across all eligible static mirrors; or `Distance`, which staggers only mirrors at or beyond `DistanceStaggerStart`. See [Update slots](#update-slots-037) |
| `DistanceStaggerStart` | 1000 cm | Face distance (camera to nearest point of the mirror) after which `StaggerMode = Distance` begins distributing update phases |
| `DistanceThrottle` | off | Use `FarUpdateEveryNFrames` instead of the base interval beyond `DistanceThrottleStart` |
| `DistanceThrottleStart` | 1500 cm | Face distance at which optional throttling begins |
| `FarUpdateEveryNFrames` | 4 | Distant capture interval when throttling is enabled. Values `1` or lower still mean every frame |
| `OffscreenUpdateEveryNFrames` | 1 | Update interval while the whole face is outside your view: `1` unchanged, `N > 1` every N frames, `0` frozen. See [Off-screen updates](#off-screen-updates-040) |
| `VisibilityTolerance` | 0.2 s | How recently this mirror's or its partner's panel must have been rendered to keep captures active. Lower values stop off-screen work sooner |

### Breakable live mirror tiles

1. Place a `BP_CaptureMirror` covering the whole wall, as above. Turn on `Tiled` (and `TwoSided` if both sides should reflect). Set `Depth` to 1 (tiles show only the first level).
2. Place `BP_MirrorTileLive` tiles across it, centred on the wall plane, thickness about the `TwinGap` (scale Y 0.06 for 6 cm). Example tile: scale (1.5, 0.06, 2.0) = 150 x 200 cm.
3. On each tile, set `Wall` to that mirror. (The tile's own mesh, `SM_MirrorTileCore` at 0.8 thickness, is already the default.)
4. A broken tile disappears on both faces and leaves a walk-through hole. One health bar per tile.

### Baked mirror tiles

Drag in `BP_MirrorTile` (snow wind on break) or `BP_MirrorTile_NoSnow` and scale to the tile size (e.g. (1.5, 0.05, 2.0)). They already use `SM_MirrorTile` (healthy) and `SM_MirrorTile_Cracked` (damaged) by default; to use your own tile shape, change the mesh on the tile's `StaticMesh` component. They do not show players. To make their shine match your room, set their `StaticMesh` component's material to your own environment material instance (next section).

### Environment material (shiny surfaces)

`M_MirrorEnvironment` shows a baked 360-degree image (cubemap) of the room where screen-space reflections have nothing to show. Its default image is a neutral grey room. To match your room:

1. Place a **Scene Capture Cube** actor where the mirror surface is, at head height.
2. Create a **Cube Render Target** asset, assign it to the capture, and let it capture (move the actor slightly or use "Capture" in its details).
3. Right-click the render target, **Create Static Texture**. That makes your cubemap.
4. Right-click `M_MirrorEnvironment`, **Create Material Instance**, save it in *your* map's folder, and set `Environment` to your cubemap. Adjust `EnvStrength`, `Tint` and `Roughness` if wanted.
5. Apply that instance to your floors, ceilings or tiles. Delete the capture actor.

### Partners: angles and more than two mirrors

- **Angled partners:** partnered mirrors do not have to be parallel; each reflection is computed across each mirror's own plane.
- **Targeting a back face:** enable `PartnerBackFace` beside `Partner`. For a connection originating from a two-sided mirror's back, use `BackPartner` and `BackPartnerBackFace`. Example: Funhouse front to TwoSided back uses `Funhouse.Partner = TwoSided`, `Funhouse.PartnerBackFace = on`, `TwoSided.BackPartner = Funhouse`, and `TwoSided.BackPartnerBackFace = off`.
- **One partner per mirror.** Several mirrors facing each other can be chained (A -> B -> C -> A): each mirror gets correct deeper reflections only into its partner; a non-partner mirror seen inside a reflection shows its own first-level image (plausible, but not geometrically exact).

## Online play

Each player's game renders mirrors from their own camera, so everyone sees a correct reflection. Your stand-in exists only on your machine and only for your character; other players see your real body in their mirrors. Breakable tiles use the game's own breakable windows: the server decides when a tile breaks and every player sees it break. Dedicated servers do no mirror rendering and spawn no stand-ins.

## Known limitations

- Sheathed weapons do not appear in first-person reflections (weapons in hand do; third person shows everything).
- The head-only variants (`HeadSource` 1, stretch head) have a stiff head; the default full body does not.
- Live tiles show only the first reflection level.
- Tile breaking can only be tested in game (the SDK's window Blueprints are empty placeholders).
- Baked tiles never show players.
- One partner per mirror (see Partners above).

## Contents

| Path (`/Game/Mods/MirrorKit/`) | Assets |
| --- | --- |
| `Blueprints/` | `BP_CaptureMirror`; presets `BP_Mirror_Funhouse`, `BP_Mirror_Headless`, `BP_Mirror_StretchHead`, `BP_Mirror_Ghost`, `BP_Mirror_TwoSided`; tiles `BP_MirrorTileLive`, `BP_MirrorTile`, `BP_MirrorTile_NoSnow`; stand-ins `BP_MirrorDoubleFull`, `BP_MirrorDouble`, `BP_MirrorDoubleStretch` (spawned automatically) |
| `Materials/` | `M_MirrorEnvironment`, `M_MirrorPolished` (screen-space only), `M_MirrorDarkCenter`, `M_MirrorCracked`; internal: `M_CaptureMirror`, `M_Matte` |
| `Meshes/` | `SM_MirrorTile`, `SM_MirrorTile_Cracked`; internal: `SM_MKCube`, `SM_MirrorTileCore` |
| `Textures/` | `TC_MK_DefaultEnvironment` (default cubemap) |

## Developer tool: AgGraphTools

`BP_CaptureMirror` now separates initialization, scheduled updates, hidden-actor maintenance,
first-level portal capture, deep portal capture and legacy/funhouse capture into Blueprint
functions. All deep levels call the same portal implementation; all legacy levels call the same
legacy implementation. These are compiled functions, not macros containing repeated code.
The EventGraph contains only BeginPlay/Tick and their function calls. Dependency-based node
placement replaces overlapping generated positions within the new functions.

This refactor retains the existing settings, ten component slots, allocation rules and capture
order (first level, then eligible deeper levels in descending order). Resource initialization,
active-capture actor hiding and partner-panel hiding use bounded runtime loops. Depth dispatch
remains explicit to preserve capture order and avoid evaluating partner views on independent
mirrors. It does not add a global scheduler or change how non-partner images update. Node/file-size
reductions alone do not establish an FPS gain.

### Reading the Blueprint

Open `BP_CaptureMirror`. EventGraph routes BeginPlay to `InitializeMirror` and Tick to
`UpdateMirror`. Initialization is an eight-node overview; updating is a seven-node overview. Double-click its calls to follow the work;
the My Blueprint function list groups helpers under `Mirror` categories. Function entry comments
and tooltips explain constraints and fallback behavior.

| Category | Functions and responsibility |
| --- | --- |
| Lifecycle | `InitializeMirror`, `UpdateMirror`: ordering and early-exit branches |
| Setup | `ConfigureBackingAndDepth`, `CreateBackFace`, `ConfigureTileMode`, `InitializeCaptureResources`, `InitializeFirstCapture`, `InitializeDeepCapture`, `EnsurePlayerStandIn` |
| Scheduling | `ResolvePartner`, `AdvanceCaptureSchedule`, `CheckCaptureEligibility` |
| Capture | `CaptureReflectionStack`, `CapturePortalFirst`, `CapturePortalDeep`, `CaptureLegacyView` |
| Projection | `ReflectViewAcrossPlane`, `ProjectMirrorRay`, `FitMirrorInterval`, `ComputeVisibleMirrorBounds`: pure mathematical helpers |
| Visibility | `RefreshCaptureHiddenActors`, `HideActorFromActiveCaptures`, `HidePartnerPanelsForCapture` |

`InitializeCaptureResources` visits the ten slots in ascending order. Level 1 always receives its
existing first-level setup; eligible deeper slots share deep setup, and inactive deep panels hide.
The two hiding loops stop at the first inactive/out-of-range index. These loops reorganize the
same operations; they do not introduce arbitrary depth or reduce the number of requested captures.

`CaptureReflectionStack` remains the largest function because it explicitly dispatches level 1
before deep levels 10 through 2. An independent mirror exits before evaluating partner-dependent
deep views. `AdvanceCaptureSchedule` preserves the existing frame-counter expression timing,
including its post-increment evaluation. Changing either behavior belongs in a separate test.

Construction Script still enforces depth/gap limits and sizes the editor clearance guide. It is
separate from the BeginPlay resource creation and per-frame capture path.

[`Tools/AgGraphTools`](Tools/AgGraphTools) is the custom editor-only UE 4.25 plugin used by
MirrorLab's generator to create MirrorKit's Blueprint graphs and cubemaps from Python. It exposes
`unreal.AgGraphLibrary` for graph nodes, pins, variables, components, compilation, graph export and
reflection-capture helpers that UE 4.25 Python does not provide.

You do **not** need it to install MirrorKit, place or edit the supplied assets, cook a map that uses
them, play, or host a server. Maintainers only need it when running `create_map.py` to regenerate the
generated MirrorKit and MirrorLab assets. See the tool's [README](Tools/AgGraphTools/README.md) for
installation and usage.

Files in this release: `MirrorKit.pak` (+ `.sha256`), `MirrorKit_Source.zip` (`Content/`, README, manifest, license and third-party notices), `README.md`.

## License

Original MirrorKit work is licensed under the MIT License. See `LICENSE` and `THIRD_PARTY_NOTICES.md`.

## Reading the Blueprint graphs

Start with EventGraph, then InitializeMirror or UpdateMirror. Colored `MirrorKit |` comment blocks explain each function, construction setup, scheduling, partner resolution, capture projection and exclusions. CaptureReflectionStack separates pure view calculations from ordered capture execution. Presets inherit the parent implementation.

The first audited BP_CaptureMirror had 2,294 logic nodes (2,266 EventGraph + 28 Construction Script); after tick optimization it had 2,226, after function extraction 1,571, and after structural cleanup 849. Comment boxes are counted separately: 26 in the parent. Comments and node placement do not affect runtime behavior. `Tools/annotate_blueprints.py` reapplies generated comments without changing pins or defaults.

Validation and the decision to retain the hiding loops are recorded in [Blueprint cleanup validation](docs/blueprint-cleanup-validation.md). Performance variation did not establish a cleanup regression or a reliable benefit from unrolling.
