"""Idempotent editor-only graph documentation. Does not change pins or defaults."""
import re
import unreal as u

DESCRIPTIONS = {
 'UserConstructionScript': 'Editor setup\nClamp Depth to 1-10, keep TwinGap clear of capture planes, size the placement guide.',
 'EventGraph': 'Entry points\nBeginPlay initializes the mirror. Tick delegates to UpdateMirror.',
 'InitializeMirror': 'Startup orchestration\nConfigure geometry, optional back face and tiles; initialize capture resources and player stand-in.',
 'ConfigureBackingAndDepth': 'Backing and effective depth\nConfigure backing visibility and depth for this mirror instance.',
 'CreateBackFace': 'Optional second face\nCreate and configure the twin mirror on the back of a two-sided wall.',
 'ConfigureTileMode': 'Breakable tile display\nConfigure the capture wall for tiles sharing its reflection.',
 'InitializeFirstCapture': 'First reflection resources\nCreate the first render target and material binding using the first-level resolution rules.',
 'InitializeDeepCapture': 'Deeper reflection resources\nApply the selected resolution profile and minimum scale to this depth slot.',
 'InitializeCaptureResources': 'Bounded component initialization\nVisit the ten existing capture slots; initialize first and deeper levels with their respective rules.',
 'EnsurePlayerStandIn': 'Local player reflection stand-in\nFind or create the selected head/body stand-in used by scene captures.',
 'UpdateMirror': 'Per-frame orchestration\nResolve partner, advance scheduling, check eligibility, then refresh hidden actors and capture.',
 'ResolvePartner': 'Optional explicit partner\nResolve a valid distinct partner/back face. Unassigned or self references use one reflection level.',
 'AdvanceCaptureSchedule': 'Cadence, stagger and distance throttle\nIncrement FrameCounter; due when (FrameCounter + phase) % interval == 0.',
 'CheckCaptureEligibility': 'Capture gating\nCheck viewer validity, face distance, front-facing and recent-render/tile conditions before capture work.',
 'IsWithinFaceDistance': 'Centre-first distance test\nDecide from the centre distance when possible; compute the exact face distance only near the limit.',
 'DistanceToMirrorFace': 'Face distance\nDistance from a point to the nearest point of the mirror face rectangle; used by capture distance, stagger and throttle.',
 'ReflectViewAcrossPlane': 'Reflection geometry\nReflect the supplied view across the mirror plane.',
 'ProjectMirrorRay': 'Projection helper\nProject a mirror ray into the capture view.',
 'FitMirrorInterval': 'Projection interval\nFit the supplied bounds while preserving the original clamp rules.',
 'ComputeVisibleMirrorBounds': 'Visible panel bounds\nCompute the mirror bounds used by fitted portal projection.',
 'CapturePortalFirst': 'First portal capture\nSet first-level view, projection and panel state, then capture the scene.',
 'CapturePortalDeep': 'Deeper portal capture\nApply the supplied reflected view and projection to a deeper capture slot.',
 'CaptureLegacyView': 'Legacy capture path\nApply the legacy reflected camera transform and capture settings.',
 'HidePartnerPanelsForCapture': 'Partner panel exclusions\nHide the applicable partner panels; stop at the active panel count.',
 'HideActorFromActiveCaptures': 'Capture exclusions\nApply actor hiding to active capture components; stop at the first inactive slot.',
 'RefreshCaptureHiddenActors': 'Eligible-capture maintenance\nRefresh stand-in references at the existing interval and update player component exclusions.',
}

def annotate(bp, legacy_comment_guids=()):
    api = u.AgGraphLibrary
    api.clear_generated_comments(bp, list(legacy_comment_guids))
    groups = {}
    for line in api.list_annotation_nodes(bp):
        graph, node, pure, member = line.split('\t')
        groups.setdefault(graph, []).append((node, pure == '1', member))
    # Empty inherited graphs also deserve an explanation.
    for graph in re.findall(r'^=== GRAPH (.+) \(', api.export_graphs_text(bp), re.M):
        groups.setdefault(graph, [])
    count = 0
    expected = []
    for graph, nodes in groups.items():
        if bp.get_name() == 'BP_CaptureMirror':
            if graph == 'CaptureReflectionStack':
                sections = [
                    ('Reflected views and geometry\nPure calculations feeding the capture sequence below; these nodes do not initiate captures.', [n for n,p,m in nodes if p]),
                    ('Ordered scene captures\nCapture the first level, then deeper levels from 10 down to 2.\nIndependent mirrors exit before partner/deep capture work.', [n for n,p,m in nodes if not p])]
            else:
                sections = [(DESCRIPTIONS[graph], [n for n,p,m in nodes])]
        elif bp.get_name().startswith('BP_MirrorDouble'):
            sections = [('Player stand-in: '+graph+'\nCopy local player meshes/pose for reflections. Tick skips dedicated servers and checks player validity.\nHead/body variant behavior is retained in this Blueprint.', [n for n,p,m in nodes])]
        elif bp.get_name().startswith('BP_MirrorTile'):
            sections = [('Breakable tile: '+graph+'\nDetect broken/dead state, hide the tile and disable collision.\nLive tiles also display the wall and twin reflection materials.', [n for n,p,m in nodes])]
        else:
            sections = [('Inherited mirror preset: '+graph+'\nCapture behavior lives in BP_CaptureMirror. Inspect Class Defaults for this preset settings.', [n for n,p,m in nodes])]
        for title, names in sections:
            assert api.add_comment_block(bp, graph, title, names, count), (bp.get_name(), graph)
            expected.append('MirrorKit | ' + title)
            count += 1
    assert sorted(comment_texts(api.export_graphs_text(bp))) == sorted(expected), bp.get_name()
    return count

def comment_texts(text):
    comments = []
    for match in re.finditer(r'^Begin Object Class=/Script/UnrealEd.EdGraphNode_Comment .*?^End Object', text, re.M | re.S):
        field = re.search(r'^\s*NodeComment="(.*)"\s*$', match.group(0), re.M)
        assert field, 'Comment text missing'
        comments.append(field.group(1).replace('\\r', '').replace('\\n', '\n').replace('\\"', '"'))
    return comments

def semantic_nodes(text):
    """Compare all serialized existing node data except editor placement."""
    result = {}
    for section in text.split('=== GRAPH ')[1:]:
        graph = section.split(' (', 1)[0]
        for match in re.finditer(r'^Begin Object Class=.*?^End Object', section, re.M | re.S):
            block = match.group(0)
            if 'Class=/Script/UnrealEd.EdGraphNode_Comment ' in block.splitlines()[0]:
                continue
            name = re.search(r' Name="([^"]+)"', block).group(1)
            result[graph+'/'+name] = '\n'.join(line for line in block.strip().splitlines()
                if not re.match(r'\s*NodePos[XY]=', line))
    return result
