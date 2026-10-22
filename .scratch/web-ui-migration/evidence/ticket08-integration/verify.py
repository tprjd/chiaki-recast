"""Compare ticket 08 sources and saved evidence without building or running tests."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(sys.argv[1]).resolve()
EVIDENCE = ROOT / '.scratch/web-ui-migration/evidence/ticket08'
WORKER = 'b7866044a9ae0251048a76f823d37b888d3f419d'
TESTS = 'gui/tests/embeddedsetup.cpp'


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


def digest(data):
    return hashlib.sha256(data).hexdigest()


def source(path):
    return (ROOT / path).read_text(encoding='utf-8')


def at(commit, path):
    return git('show', f'{commit}:{path}').decode().replace('\r\n', '\n')


def recorded(group, path):
    """Apply a recorded uncommitted diff in memory, checking every context line."""
    base = (EVIDENCE / f'{group}-source.txt').read_text().strip()
    old = at(base, path).splitlines(keepends=True)
    patch = (EVIDENCE / f'{group}.diff').read_text()
    marker = f'diff --git a/{path} b/{path}\n'
    if marker not in patch:
        return ''.join(old)
    section = patch.split(marker, 1)[1].split('\ndiff --git ', 1)[0].rstrip('\n') + '\n'
    result, cursor = [], 0
    for hunk in re.split(r'(?m)^@@ ', section)[1:]:
        header, body = hunk.split('\n', 1)
        start = int(re.match(r'-(\d+)', header).group(1)) - 1
        result.extend(old[cursor:start])
        cursor = start
        for line in body.splitlines(keepends=True):
            if line.startswith((' ', '-')):
                assert old[cursor] == line[1:], (group, path, cursor, line)
                cursor += 1
            if line.startswith((' ', '+')):
                result.append(line[1:])
    return ''.join(result + old[cursor:])


def test(text, name):
    start = text.index(f'    void {name}()\n    {{')
    end = text.find('\n    void ', start + 1)
    return text[start:end if end >= 0 else text.index('\n};', start)].rstrip()


def member(text, name):
    start = re.search(r'^.*QmlBackend::' + name + r'\(', text, re.M).start()
    return text[start:text.index('\n}\n', start) + 3]


saved = json.loads((EVIDENCE / 'identity.json').read_text())
current = json.loads(subprocess.check_output([
    sys.executable, str(EVIDENCE / 'record-identity.py'), str(ROOT)]))
assert current['sources'] == saved['sources']
assert current['artifacts'] == saved['artifacts']
assert current['preserved'] == saved['preserved']
assert not git('diff', WORKER, 'HEAD', '--', 'gui').strip()
assert not git('diff', saved['head'], 'HEAD', '--', 'gui', ':!gui/tests/README.md').strip()
assert not git('diff', 'fab6df5c', 'HEAD', '--', 'third-party').strip()

comparisons = []
for group, names in {
    'native': ['remoteNativeConnection'],
    'refresh-final': ['psnRemoteRefresh'],
    'isolated': ['psnRemoteRefreshOwnership', 'psnRemoteTokenConnection'],
    'green': ['psnRefreshCredentials', 'psnSignIn'],
    'expanded': ['psnRefreshCredentials', 'consoleLibrary'],
}.items():
    old = recorded(group, TESTS)
    for name in names:
        expected, actual = test(old, name), test(source(TESTS), name)
        # Earlier affected cases used real discovery. This does not prove their isolation.
        normalize = lambda value: value.replace(', {}, {}, controlledDiscovery()', '').replace(', {}, controlledDiscovery()', '')
        assert normalize(expected) == normalize(actual), (group, name)
        comparisons.append({'group': group, 'test': name, 'sha256': digest(actual.encode()),
                            'discoveryArgumentNormalized': expected != actual})

backend = 'gui/src/qmlbackend.cpp'
for name in ['refreshAuth', 'updatePsnHosts', 'updatePsnHostsThread', 'refreshPsnToken', 'watchConnectionToken']:
    old, actual = member(recorded('expanded', backend), name), member(source(backend), name)
    assert old == actual, name
    comparisons.append({'group': 'expanded', 'method': name, 'sha256': digest(actual.encode())})
assert recorded('expanded', 'gui/src/psntoken.cpp') == source('gui/src/psntoken.cpp')
assert source(TESTS).count('QCOMPARE(starts.load(), 0);') == 2
assert 'transport.startSession = [&](ChiakiSession *) { ++starts; return CHIAKI_ERR_CANCELED; };' in source(TESTS)

result = {
    'worker': WORKER, 'integration': git('rev-parse', 'HEAD').decode().strip(),
    'testedSource': saved['head'], 'sourceCount': len(current['sources']),
    'artifactCount': len(current['artifacts']), 'artifacts': current['artifacts'],
    'preserved': current['preserved'], 'runtimeSourceComparisons': comparisons,
    'evidence': {p.name: digest(p.read_bytes()) for p in sorted(EVIDENCE.iterdir()) if p.is_file()},
    'limits': 'No builds or test runs. Earlier discovery traffic and capture zoom remain unproved. Live PSN, physical focus/input, GPU, HighQuality, and platform acceptance remain open.',
}
print(json.dumps(result, indent=2))
