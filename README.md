# MirrorKit

Mirrors for Chivalry 2 Unchained maps: live reflections that show players, cheap shiny surfaces and breakable mirror tiles. MirrorKit is a **library**: it adds no map by itself. Maps use its assets and require it to be installed.

Version 0.3.0. Built with [ArgonSDK](https://github.com/Chiv2-Community/ArgonSDK) (Unreal Engine 4.25). The optional [MirrorLab](https://github.com/SageFlare/MirrorLab) map demonstrates every mirror type.

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

Live mirrors render the scene again from the mirror's point of view: one extra render per reflection level (`Depth`, 1 to 3) per visible mirror face. A mirror only renders while you are within 25 m, in front of it, and it was on screen in the last 0.2 s; mirrors behind walls or behind you cost nothing.

Measured in MirrorLab on one PC, solo, frame rate uncapped (empty hub: about 240 fps which was capped at 240). Use these as relative guidance; your numbers will differ, and more players inside a mirror's view add cost.

| Setup | Frame rate | Approx. extra per frame |
| --- | --- | --- |
| Facing pair, `Depth` 3, both in view | 120-200 fps | 2-5 ms |
| Floor + ceiling tunnel, `Depth` 3 | ~170 fps | ~3 ms |
| Two-sided tiled wall, `Depth` 1 | ~210 fps | ~2 ms |
| Baked tiles | 200-300 fps | < 1 ms |
| Screen-space floor/ceiling materials | 150-220 fps | ~1-3 ms (engine reflections, view dependent) |

To save cost: keep the default `Depth` 1 where a tunnel is not needed, avoid many live mirrors visible at once, and use baked tiles or materials for decoration. Partnered mirrors can use `Depth` 3 for a tunnel. `Depth` can go up to 10, but each level is another full render of the scene while the mirror is visible and the deep images become tiny; values above 3 are allowed, not tested, and not recommended. `Resolution` (default 1) trades sharpness for cost.

## Using MirrorKit in your map

### Install the kit in ArgonSDK

1. Unzip `MirrorKit_Source.zip`.
2. Copy its `Content` folder's contents to **`ArgonSDK/Content/Mods/MirrorKit/`** (so you get `ArgonSDK/Content/Mods/MirrorKit/Blueprints/BP_CaptureMirror.uasset`).
   - This exact location matters: Unreal names assets by folder (`/Game/Mods/MirrorKit/...`), the kit's assets reference each other by those names, and `MirrorKit.pak` loads them at the same names in game. Do not move or rename kit assets in the editor; your map would then point at assets the shared `MirrorKit.pak` does not contain. (The files can physically live elsewhere if you link the folder into place.)
3. Copy `MirrorKit.pak` into your game's `TBL/Content/Paks/` to test.

### Build your map so it works with the shared PAK

- **Your map's own walls, floors and props should not typically use kit assets.** Use your own meshes/materials for structure; only place kit actors and kit mirror materials. Then a player missing MirrorKit still gets a playable map instead of areas missing entirely and creating pitfalls to the void.
- **Your PAK must not contain MirrorKit's files.** Cooking your map also writes cooked kit files to `Saved/Cooked/.../Content/Mods/MirrorKit/`; package only your own mod folder.
- In your `mod.json`, add MirrorKit as a dependency.

### Placing a live mirror

1. Drag `BP_CaptureMirror` (or a preset) into the level. The mirror's **local +X direction points out of its reflective face**; the face is at the actor's origin. Switch the viewport transform gizmo from **World** to **Local** to see this direction: in World mode the red X arrow stays aligned to the world even when the mirror rotates. Turn a wall mirror with **Rotation Z (Yaw)**, not Rotation X.
2. Size it with the actor **scale**: Y x 100 = width in cm, Z x 100 = height in cm, keep X = 1. Example: scale (1, 12, 6) is a 12 m x 6 m wall mirror. For a floor mirror, rotate it to face up.
3. **Facing mirrors:** set each one's `Partner` to the other to get the tunnel. A single mirror needs no partner.
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
| `Resolution` | 1 | Reflection image size multiplier (1280 x 720 first level, 640 x 360 deeper) |

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
