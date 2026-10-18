"""Idempotent editor-only graph documentation. Does not change pins or defaults.
annotate_sections: one box per logical step, recorded by the generator (BP_CaptureMirror, BP_MirrorKitManager, tiles, stand-ins).
annotate: one explanatory box per graph, used only for the preset children (their graphs are empty)."""
import re
import unreal as u


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
        assert bp.get_name() != 'BP_CaptureMirror', 'BP_CaptureMirror uses annotate_sections'
        if bp.get_name().startswith('BP_MirrorDouble'):
            sections = [('Player stand-in: '+graph+'\nCopy local player meshes/pose for reflections. Tick skips dedicated servers and checks player validity.\nHead/body variant behavior is retained in this Blueprint.', [n for n,p,m in nodes])]
        elif bp.get_name().startswith('BP_MirrorTile'):
            sections = [('Breakable tile: '+graph+'\nDetect broken/dead state, hide the tile and disable collision.\nLive tiles also display the Wall (front) and BackWall (back) reflection materials.', [n for n,p,m in nodes])]
        else:
            sections = [('Inherited mirror preset: '+graph+'\nCapture behavior lives in BP_CaptureMirror. Inspect Class Defaults for this preset settings.', [n for n,p,m in nodes])]
        for title, names in sections:
            assert api.add_comment_block(bp, graph, title, names, count), (bp.get_name(), graph)
            expected.append('MirrorKit | ' + title)
            count += 1
    assert sorted(comment_texts(api.export_graphs_text(bp))) == sorted(expected), bp.get_name()
    return count

def annotate_sections(bp, sections, legacy_comment_guids=()):
    """0.13.0: one comment box per logical section (sections = {graph: [(text, node names)]} recorded by the generator),
    stacked top to bottom in execution order. Graphs without sections get no box; their function tooltip describes them.
    Nodes that no longer exist (pruned) are skipped."""
    api = u.AgGraphLibrary
    api.clear_generated_comments(bp, list(legacy_comment_guids))
    present = {}
    for line in api.list_annotation_nodes(bp):
        graph, node = line.split('	')[:2]
        present.setdefault(graph, set()).add(node)
    expected, count = [], 0
    for graph, items in sections.items():
        for text, nodes in items:
            nodes = [n for n in nodes if n in present.get(graph, ())]
            if not nodes:
                continue
            assert api.add_comment_block(bp, graph, text, nodes, count), (bp.get_name(), graph, text)
            expected.append('MirrorKit | ' + text)
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
