"""Check ticket 05 evidence and the unchanged tested source without running the app."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(sys.argv[1]).resolve()
EVIDENCE = ROOT / '.scratch/web-ui-migration/evidence/ticket05'
PREVIOUS = '24f57a8c'
WORKER = 'c3bf624c1068820aad4c8f333cab8bad2354cd44'

def digest(data):
    return hashlib.sha256(data).hexdigest()

def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])

def text_at(commit, path):
    return git('show', f'{commit}:{path}').decode('utf-8').replace('\r\n', '\n')

def member(text, symbol):
    starts = list(re.finditer(r'^\S[^\n]*\b\w+::\w+\([^\n]*\n\{', text, re.M))
    found = [i for i, match in enumerate(starts) if symbol + '(' in match.group()]
    assert len(found) == 1, symbol
    i = found[0]
    return text[starts[i].start():starts[i + 1].start() if i + 1 < len(starts) else len(text)].rstrip()

def test(text, symbol):
    starts = list(re.finditer(r'^    void (\w+)\(\)\n    \{', text, re.M))
    i = next(i for i, match in enumerate(starts) if match.group(1) == symbol)
    return text[starts[i].start():starts[i + 1].start() if i + 1 < len(starts) else text.index('\n};', starts[i].start())].rstrip()

identity = json.loads((EVIDENCE / 'identity.json').read_text())
assert len(identity['source']) == 19
for row in identity['source']:
    raw = (ROOT / row['path']).read_bytes()
    assert digest(raw) == row['sha256'] or digest(raw.replace(b'\r\n', b'\n')) == row['gitBlobSha256'], row['path']
    assert digest(git('show', f"{identity['sourceCommit']}:{row['path']}")) == row['gitBlobSha256'], row['path']
    assert digest(git('show', f"HEAD:{row['path']}")) == row['gitBlobSha256'], row['path']
assert len(identity['artifacts']) == 4
for row in identity['artifacts']:
    assert digest(Path(row['path']).read_bytes()) == row['sha256'], row['path']
for row in identity['evidence']:
    assert digest((EVIDENCE / row['path']).read_bytes()) == row['sha256'], row['path']

comparisons = []
for row in identity['reusedSourceComparisons']:
    path = row['path']
    old, current = text_at(row['base'], path), (ROOT / path).read_text(encoding='utf-8')
    if 'method' in row:
        owner = 'SetupBridge' if 'setupbridge' in path else 'QmlBackend'
        old, current = [member(value, owner + '::' + row['method']) for value in (old, current)]
    elif 'test' in row:
        old, current = [test(value, row['test']) for value in (old, current)]
        if row['test'] == 'consoleManagement':
            extra = "        QTRY_VERIFY(script(setup.view(), \"document.activeElement.hasAttribute('data-close-management')\").toBool());\n"
            assert current.count(extra) == 1
            current = current.replace(extra, '')
    elif path.endswith('ConsoleManagement.svelte'):
        extra = "      await tick();\n      document.querySelector<HTMLElement>(!reply.accepted && mode === 'pin' && validTarget\n        ? '[data-saved-pin]' : '[data-close-management]')?.focus();\n"
        assert current.count(extra) == 1
        current = current.replace(extra, '')
    assert old == current, row
    comparisons.append({**row, 'independentSha256': digest(current.encode())})
assert len(comparisons) == 27

old_css = text_at(PREVIOUS, 'gui/web/src/style.css')
new_css = text_at('HEAD', 'gui/web/src/style.css')
selector = 'dialog[aria-labelledby="reset-keyboard-title"]'
assert new_css.count(selector) == 3
restored_css = new_css.replace(selector, 'dialog')
restored_css = '\n'.join(line for line in restored_css.split('\n') if not line.startswith(('.console-entry {', '.console-details {')))
assert restored_css == old_css
assert 'aria-labelledby="reset-keyboard-title"' in text_at('HEAD', 'gui/web/src/KeyboardMouse.svelte')
assert 'aria-labelledby="reset-keyboard-title"' not in text_at('HEAD', 'gui/web/src/Dialog.svelte')
assert git('diff', '--name-only', identity['sourceCommit'], 'HEAD', '--', 'gui').strip() == b''
assert git('diff', '--name-only', WORKER, 'HEAD', '--', 'gui').strip() == b''
assert git('diff', '--name-only', PREVIOUS, 'HEAD', '--', 'third-party').strip() == b''
report = {'head': git('rev-parse', 'HEAD').decode().strip(), 'previousIntegration': git('rev-parse', PREVIOUS).decode().strip(),
          'worker': WORKER, 'testedSource': identity['sourceCommit'], 'sourceCount': 19, 'artifactCount': 4,
          'evidenceCount': len(identity['evidence']), 'identitySha256': digest((EVIDENCE / 'identity.json').read_bytes()),
          'preservedSourceComparisons': comparisons, 'cssDeclarationsPreserved': True,
          'guiMatchesTestedSource': True, 'guiMatchesWorker': True, 'vendorGitlinksUnchanged': True}
output = Path(sys.argv[2])
output.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
print(f"PASS: 19 source, 4 artifact, {len(identity['evidence'])} evidence hashes, 27 independent source comparisons, CSS declarations, tested GUI tree, vendor gitlinks.")
