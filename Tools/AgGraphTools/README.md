# AgGraphTools

AgGraphTools is a custom, editor-only Unreal Engine 4.25 plugin used to generate MirrorKit's
Blueprints from Python. UE 4.25 exposes asset and material editing to Python, but it does not expose
the Blueprint graph-authoring operations MirrorKit's generator scripts need.

It adds `unreal.AgGraphLibrary`, whose helpers can:

- create, select, clear and compile Blueprint event and Construction Script graphs;
- add events, function calls, variables, branches, casts, macros and Spawn Actor nodes;
- connect graph pins and set their defaults;
- add Blueprint components and edit component templates;
- create named user-enum variables for editor dropdown settings;
- configure selection-only editor guide components that do not affect navigation;
- export Blueprint graphs as text for diagnostics;
- build reflection captures and capture a cubemap into an asset; and
- connect the material Pixel Depth Offset input, which UE 4.25 Python does not expose.

The plugin creates editor assets; it is not a runtime dependency and is not included in
`MirrorKit.pak`. Players, servers and mappers placing the supplied MirrorKit assets do not need it.

Function authoring: `CreateFunction` replaces/selects a named function; `AddFunctionInput` adds
typed input pins. Compile the signature before `AddSelfFunctionCall`. `SelectFunction("None")`
returns to EventGraph; construction selection also clears function selection. Node names are
local to the selected graph: never connect handles originating in different graphs.
`LayoutGraph` assigns dependency columns and separated rows. Reusing a function reduces duplicated
implementation; merely moving an entire graph into a function does not reduce its runtime work.

`AddFunctionOutput` and `AddFunctionReturn` author explicit return paths; `DescribeFunction`
sets category, tooltip, entry comment and purity. `AddMakeArray`/`AddArrayGet` support bounded
component loops using the engine's standard loop macros. `InvokeNoArgs` is for editor simulation
validation of parameterless functions, and is never referenced by the shipped runtime Blueprint.
`PruneUnusedPureNodes` removes only pure nodes with no output consumers, repeating until orphaned
dependencies are gone. It leaves executed/impure nodes and connected calculations intact.

## Install for regeneration

Copy (or link) this folder to `ArgonSDK/Plugins/AgGraphTools` and build the ArgonSDK editor after installing or changing the C++ plugin. Editor Python scripts can then call `unreal.AgGraphLibrary`. MirrorKit's own generator scripts are part of its development workspace, not of this repository; the saved `.uasset` files they produce are validated, cooked and packaged without executing any graph-authoring calls.

Only `AgGraphTools.uplugin` and `Source/` belong in source control. `Binaries/` and `Intermediate/`
are machine-generated build output.

### Editor comment blocks

`ListAnnotationNodes`, `ClearGeneratedComments` and `AddCommentBlock` support `../annotate_blueprints.py`. Generated blocks use the `MirrorKit | ` prefix; clearing preserves unrelated comments. Blocks group and position existing nodes without modifying links or defaults. The helper is editor-only. Verification compares serialized non-comment nodes excluding positions and confirms compilation; comments are not counted as logic nodes.

Comment text must be assigned after `FGraphNodeCreator::Finalize()` because `PostPlacedNewNode()` resets it. The annotation helper asserts exact labels; the build cold-loads saved assets and checks text/count again. Legacy cleanup requires explicit GUIDs for known generated boxes still titled `Comment`; arbitrary user comments are preserved.
