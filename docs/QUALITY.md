# Mirror quality options

Players and server hosts can change how MirrorKit mirrors look and how much they cost, without touching the map. This page lists every option. For a quick start, see [Mirror quality in the README](../README.md#mirror-quality).

## Where the options go

| Where | Who | Format |
| --- | --- | --- |
| Launch options in the Unchained Launcher | a player, for their own game | `-Key=Value`, e.g. `-MirrorQuality=Low` |
| The map's **Options** box in the main menu, or the server's map options | a server host for everyone on the server; also you when playing offline (you are the server then) | `?Key=Value`, e.g. `?MirrorQuality=High` |

Several options go one after another: `-MirrorQuality=Low -MirrorFocusRate=1000` or `?MirrorQuality=High?MirrorMaxRate=90`. In the Options box, `-Key=Value` does nothing; use `?`.

## How the layers combine

From first to last; a later layer wins:

1. **Map:** the map maker's settings.
2. **Server values (optional):** `?MirrorQuality=` and single values in the map options. Everyone on the server starts from these instead of the map's.
3. **Your choice (optional):** `-MirrorQuality=` and single values in your launch options. Up or down, freely.
4. **Server cap (optional):** `?MirrorMaxQuality=` and limits in the map options. Only values above a limit are lowered, for everyone on the server, including players who chose nothing. Without a cap you can raise as you like.

If nobody chooses anything, mirrors run the map's settings.

## Presets

| Preset | Resolution | Updates per second | Nearest mirrors | Distance | Captures per frame | Reflection AO | Smoothing | Anchor |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Ultra` | 1.0 | 144 | every frame | 40 m | no budget | on | on | off |
| `High` | 0.5 | 90 | 144 | 25 m | 2 | off | on | off |
| `Medium` | 0.5 | 60 | 60 | 20 m | 3 | off | on | off |
| `Low` | 0.35 | 45 | 45 | 15 m | 2 | off | on | off |
| `Off` | no live reflections; mirrors show plain dark glass | | | | | | | |

Use a preset by name (`-MirrorQuality=Low`) or by the path of any `BP_MirrorQuality` data asset in a PAK you have installed (`-MirrorQuality=/Game/Mods/<Mod>/<Asset>`). A custom preset needs its PAK on every machine that uses it. An unknown name changes nothing.

## Single values

The same keys work as launch options (`-Key=`) and as server values (`?Key=`). They apply after the preset in the same layer.

| Key | Value | Changes |
| --- | --- | --- |
| `MirrorResolution` | e.g. `0.5`, `1` | Reflection resolution (1 = 1280 × 720) |
| `MirrorRate` | updates per second, e.g. `90`; `0` = every N frames | How often mirrors update |
| `MirrorFocusRate` | updates per second; `1000` = every frame | How often the nearest and most central mirrors update |
| `MirrorDistance` | cm, e.g. `2500` | Mirrors farther than this stop updating |
| `MirrorCaptures` | captures per frame; `0` = budget off | The capture budget |
| `MirrorAO` | `0` off, `1` on | Ambient occlusion inside reflections |
| `MirrorSmooth` | `0` off, `1` on | Steady update interval when FPS fluctuates |
| `MirrorAnchor` | `0` off, `1` on | Count each interval from the last update |
| `MirrorsOff` | (no value; launch option only) | Turn live reflections off |

## Server cap

| Key | Limits |
| --- | --- |
| `?MirrorMaxQuality=<preset or asset path>` | Every value of that preset becomes a limit |
| `?MirrorMaxResolution=0.5` | Resolution |
| `?MirrorMaxRate=90` | Both update rates |
| `?MirrorMaxDistance=2000` | Capture distance (cm) |
| `?MirrorMaxCaptures=3` | Captures per frame |
| `?MirrorNoAO` | Turns reflection AO off |
| `?MirrorsOff` | Turns live reflections off |

When several limits apply, the strictest wins.

## Notes

- Mirrors render only on players' machines. A dedicated server does no mirror rendering, so hosting costs no GPU. The server shares its map options with clients automatically.
- Quality is applied when the match starts, and again on your machine if the server's options arrive later.
