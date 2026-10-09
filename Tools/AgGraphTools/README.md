# AgGraphTools

AgGraphTools is a custom, editor-only Unreal Engine 4.25 plugin used to generate MirrorKit's
Blueprints from Python. UE 4.25 exposes asset and material editing to Python, but it does not expose
the Blueprint graph-authoring operations needed by `MirrorLab/scripts/create_map.py`.

It adds `unreal.AgGraphLibrary`, whose helpers can:

- create, clear and compile Blueprint event graphs;
- add events, function calls, variables, branches, casts, macros and Spawn Actor nodes;
- connect graph pins and set their defaults;
- add Blueprint components and edit component templates;
- export Blueprint graphs as text for diagnostics;
- build reflection captures and capture a cubemap into an asset; and
- connect the material Pixel Depth Offset input, which UE 4.25 Python does not expose.

The plugin creates editor assets; it is not a runtime dependency and is not included in
`MirrorKit.pak`. Players, servers and mappers placing the supplied MirrorKit assets do not need it.

## Install for regeneration

Make the plugin visible at `ArgonSDK/Plugins/AgGraphTools`. In this workspace that path is a junction
to `mods/MirrorKit/Tools/AgGraphTools`:

```powershell
New-Item -ItemType Junction `
  -Path .\ArgonSDK\Plugins\AgGraphTools `
  -Target .\mods\MirrorKit\Tools\AgGraphTools
```

Build the ArgonSDK editor after installing or changing the C++ plugin. Then regenerate MirrorKit and
MirrorLab with:

```powershell
.\mods\MirrorLab\build.ps1 -Regenerate
```

`create_map.py` calls `unreal.AgGraphLibrary` directly. The saved `.uasset` and `.umap` files it
produces can subsequently be validated, cooked and packaged without executing those graph-authoring
calls.

Only `AgGraphTools.uplugin` and `Source/` belong in source control. `Binaries/` and `Intermediate/`
are machine-generated build output.
