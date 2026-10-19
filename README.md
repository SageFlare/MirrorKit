# MirrorKit

![MirrorKit](images/MirrorPak.png)

Live mirrors for Chivalry 2 Unchained maps: reflections that show you and other players, cheap shiny surfaces and breakable mirror tiles. MirrorKit is a **library**: it adds no map by itself, and maps that use it need it installed.

Version 0.6.6 (development build; the next release is 0.7.0). Built with [ArgonSDK](https://github.com/Chiv2-Community/ArgonSDK) (Unreal Engine 4.25). The [MirrorLab](https://github.com/SageFlare/MirrorLab) map shows every mirror type; [MirrorMaze](https://github.com/SageFlare/MirrorMaze) is a full mirror maze built with it.

## Installation

Copy `MirrorKit.pak` **and** the map's own PAK into `Chivalry 2/TBL/Content/Paks/`, then launch through the [Unchained Launcher](https://github.com/Chiv2-Community/UnchainedLauncher). Server hosts need both files on the server too; the server itself does no mirror work.

Without `MirrorKit.pak` a map that uses it still loads, but its mirrors are missing.

## Mirror quality

Every map ships with mirror settings chosen by its maker. You can raise or lower them for your own game, and server hosts can set them for everyone.

**For your own game:** add a launch option under *Settings › Advanced Settings › CLI Arguments* in the Unchained Launcher, for example:

```
-MirrorQuality=Low
```

![Launch option](images/launch-options.png)

**For a server, or when playing offline:** put it in the map's **Options** box in the Unchained main menu, with a `?` instead of a `-`:

```
?MirrorQuality=High
```

![Options box](images/options-box.png)

Presets that come with MirrorKit:

| Preset | For |
| --- | --- |
| `Off` | No live reflections; mirrors show dark glass |
| `Low` | Weaker PCs: softer, slower reflections |
| `Medium` | A middle ground |
| `High` | The settings tuned for mirror-heavy maps |
| `Ultra` | Strong PCs: full resolution, the nearest mirrors update every frame |

If you choose nothing, mirrors use the map's settings, or the server's if the host set any. You can also change single values instead of a whole preset (for example `-MirrorFocusRate=1000`), and hosts can set an optional cap. **[Every quality option](docs/QUALITY.md)**.

## Making a map with mirrors

Drag `BP_CaptureMirror` (or one of its variants) into your level, scale it, and it works with no other setup. **[The map-making guide](docs/MAP-MAKING.md)** covers installing the kit in ArgonSDK, placing mirrors, spacing rules, settings for many mirrors at once, the capture budget, breakable mirror tiles and every mirror setting. For mirror-heavy maps, read **[designing maps for mirror performance](docs/MAP-OPTIMIZATION.md)**.

| Mirror | What you see |
| --- | --- |
| `BP_CaptureMirror` | A true reflection: your full body, other players and the room. With a facing partner, an infinite-looking tunnel |
| `BP_Mirror_Funhouse` | Magnified, gappy carnival look |
| `BP_Mirror_Headless` | Your reflection has no head |
| `BP_Mirror_StretchHead` | Your head stretches as you look up and down |
| `BP_Mirror_Ghost` | Your body is invisible; your weapons float |
| `BP_MirrorTileLive` | Breakable tiles showing a live reflection on both sides |
| `BP_MirrorTile` / `BP_MirrorTile_NoSnow` | Breakable tiles with a still reflection (cheap) |

Every effect is per viewer: a ghost or headless mirror changes only *your own* reflection as *you* see it.

## Known limitations

- Reflections of mirrors inside other independent mirrors are plausible but not geometrically exact.
- Sheathed weapons do not appear in first-person reflections; weapons in hand do.
- Live tiles show one reflection level; still tiles never show players.

[Version history](docs/CHANGELOG.md).

## License

Original MirrorKit work is licensed under the MIT License. See `LICENSE` and `THIRD_PARTY_NOTICES.md`.
