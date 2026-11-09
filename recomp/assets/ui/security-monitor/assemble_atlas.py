#!/usr/bin/env python3
"""D53: assemble tv-atlas.photon and tv-atlas.png in Photon Studio from the
layer sources (make_tv_layers.py writes sources/). Needs Photon; set
PHOTON_BIN if it is not at ~/.photon/bin/photon. Each part is one named,
editable layer at its atlas position."""
import json, os, subprocess, sys, tempfile
from pathlib import Path
HERE = Path(__file__).resolve().parent
P = os.environ.get('PHOTON_BIN', str(Path.home() / '.photon/bin/photon'))
LAYERS = [('tv-bezel', 'front.png', 0, 0, 128, 128), ('screen-cursor-off', 'screen-a.png', 128, 0, 96, 96),
          ('screen-cursor-on', 'screen-b.png', 128, 96, 96, 96), ('side-top-bottom', 'side-h.png', 0, 192, 128, 32),
          ('side-left-right', 'side-v.png', 224, 0, 32, 128), ('recess-wall', 'inner.png', 128, 192, 16, 16),
          ('screen-haze', 'haze.png', 144, 192, 32, 16)]
def run(*a):
    r = json.loads(subprocess.run([P, *a], capture_output=True, text=True, check=True).stdout)
    if not r.get('ok'):
        raise SystemExit(f'photon {a[0]}: {r.get("error")}')
    return r
for s in run('session.list')['result']:            # an open copy owns the path
    for d in s['documents']:
        if d.get('filePath') == str(HERE / 'tv-atlas.photon'):
            rev = run('execute', 'inspect', '--session', s['id'], '--document', d['id'], '--params', '{}')['revision']
            run('execute', 'session.close', '--session', s['id'], '--document', d['id'], '--revision', str(rev), '--params', '{"disposition":"discard"}')
r = run('session.create', '--params', json.dumps(dict(width=256, height=256, background='transparent', mode='headless', title='tv-atlas')))['result']
sid, doc = r['sessionId'], r['documentId']
steps = [dict(operation='image.place', params=dict(path=str(HERE / 'sources' / f), name=n, rect=dict(x=x, y=y, width=w, height=h))) for n, f, x, y, w, h in LAYERS]
with tempfile.NamedTemporaryFile('w', suffix='.json', delete=False) as t:
    json.dump(dict(label='D53 TV atlas', steps=steps), t)
rev = run('execute', 'batch', '--session', sid, '--document', doc, '--revision', '1', '--input', t.name)['revision']
os.unlink(t.name)
run('execute', 'inspect', '--session', sid, '--document', doc, '--params', '{}')
run('execute', 'document.save', '--session', sid, '--document', doc, '--revision', str(rev), '--params', json.dumps(dict(path=str(HERE / 'tv-atlas.photon'))))
run('execute', 'export.web', '--session', sid, '--document', doc, '--revision', str(rev), '--params', json.dumps(dict(path=str(HERE / 'tv-atlas.png'), format='png')))
run('execute', 'session.close', '--session', sid, '--document', doc, '--revision', str(rev), '--params', '{"disposition":"discard"}')
print('tv-atlas.photon and tv-atlas.png written')
