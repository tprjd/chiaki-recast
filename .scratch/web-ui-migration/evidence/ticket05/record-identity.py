import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[4]
EVIDENCE = Path(__file__).resolve().parent
BUILD = Path('C:/Users/david/.cache/chiaki-msvc-native/ticket05-build')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def show(commit, path):
    return subprocess.check_output(['git', '-C', str(ROOT), 'show', f'{commit}:{path}']).decode('utf-8').replace('\r\n', '\n')


def method(text, symbol):
    start = re.search(r'^[^\n]*\b' + re.escape(symbol) + r'\(', text, re.M)
    assert start, symbol
    following = re.search(r'^\w[^\n]*::\w+\(', text[start.end():], re.M)
    end = start.end() + following.start() if following else len(text)
    return text[start.start():end].rstrip()


def test_case(text, name):
    start = text.index(f'    void {name}()\n')
    following = re.search(r'^    void \w+\(', text[start + 1:], re.M)
    end = start + 1 + following.start() if following else text.index('\n};', start)
    return text[start:end].rstrip()


source_commit = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
paths = [
    'gui/include/setupbridge.h', 'gui/src/setupbridge.cpp', 'gui/src/qmlbackend.cpp',
    'gui/web/src/App.svelte', 'gui/web/src/ConsoleManagement.svelte', 'gui/web/src/Dialog.svelte',
    'gui/web/src/ConnectAction.svelte', 'gui/web/src/bridge.ts', 'gui/web/src/style.css',
    'gui/web/package-lock.json', 'gui/tests/embeddedsetup.cpp', 'gui/tests/CMakeLists.txt',
    'gui/src/qml/qml.qrc', 'gui/src/qml/MainView.qml', 'gui/src/qml/ConsolePinContent.qml',
    'gui/src/qml/LiveConsolePinHost.qml', 'gui/src/qml/LiveConsolePinContent.qml',
    'gui/src/qml/StreamRoot.qml', 'gui/src/applicationcontroller.cpp',
]
comparisons = []
base = '5bf0b24d'
for name in ('saveManualAddress', 'manageConsole', 'connectConsole', 'identity', 'resolveConsole'):
    path = 'gui/src/setupbridge.cpp'
    same = method(show(base, path), 'SetupBridge::' + name) == method((ROOT / path).read_text(encoding='utf-8'), 'SetupBridge::' + name)
    comparisons.append({'base': base, 'path': path, 'method': name, 'matches': same})
for name in ('hosts', 'connectToHost', 'connectionTargetKey', 'resolveConnectionTarget', 'connectionTargetValid', 'createSession', 'releaseSession'):
    path = 'gui/src/qmlbackend.cpp'
    same = method(show(base, path), 'QmlBackend::' + name) == method((ROOT / path).read_text(encoding='utf-8'), 'QmlBackend::' + name)
    comparisons.append({'base': base, 'path': path, 'method': name, 'matches': same})
for path in ('gui/web/src/App.svelte', 'gui/web/src/ConsoleManagement.svelte', 'gui/web/src/Dialog.svelte',
             'gui/web/src/ConnectAction.svelte', 'gui/src/applicationcontroller.cpp', 'gui/src/qml/StreamRoot.qml',
             'gui/src/qml/ConsolePinContent.qml', 'gui/src/qml/LiveConsolePinContent.qml', 'gui/src/qml/LiveConsolePinHost.qml'):
    current = (ROOT / path).read_text(encoding='utf-8')
    note = None
    if path == 'gui/web/src/ConsoleManagement.svelte':
        result_focus = """      await tick();
      document.querySelector<HTMLElement>(!reply.accepted && mode === 'pin' && validTarget
        ? '[data-saved-pin]' : '[data-close-management]')?.focus();
"""
        assert current.count(result_focus) == 1
        current = current.replace(result_focus, '')
        note = 'Excludes the tested saved-PIN result focus addition.'
    comparisons.append({'base': base, 'path': path, 'matches': show(base, path) == current, 'note': note})
for name in ('manualAddress', 'manualAddressValidation', 'consoleManagement', 'consoleTargetGuards', 'manualHandoff', 'consoleLibrary'):
    path = 'gui/tests/embeddedsetup.cpp'
    current = test_case((ROOT / path).read_text(encoding='utf-8'), name)
    note = None
    if name == 'consoleManagement':
        assertion = '''        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-close-management')").toBool());
'''
        assert current.count(assertion) == 1
        current = current.replace(assertion, '')
        note = 'Excludes the added saved-PIN result focus assertion.'
    same = test_case(show(base, path), name) == current
    comparisons.append({'base': base, 'path': path, 'test': name, 'matches': same, 'note': note})
assert all(row['matches'] for row in comparisons), [row for row in comparisons if not row['matches']]

artifacts = [BUILD / 'gui/chiaki.exe', BUILD / 'gui/tests/chiaki-embedded-setup-tests.exe', BUILD / 'gui/web/app.js', BUILD / 'gui/web/app.css']
record = {
    'sourceCommit': source_commit,
    'source': [{'path': path, 'sha256': sha(ROOT / path),
                'gitBlobSha256': hashlib.sha256(subprocess.check_output(['git', '-C', str(ROOT), 'show', f'HEAD:{path}'])).hexdigest()}
               for path in paths],
    'artifacts': [{'path': str(path), 'sha256': sha(path)} for path in artifacts],
    'evidence': [{'path': path.name, 'sha256': sha(path)} for path in sorted(EVIDENCE.iterdir())
                 if path.is_file() and path.name not in ('identity.json', 'verification.md')],
    'reusedSourceComparisons': comparisons,
    'desktopSubmodules': subprocess.check_output(['git', '-C', str(ROOT), 'ls-tree', 'HEAD',
        'test/munit', 'third-party/nanopb', 'third-party/jerasure', 'third-party/gf-complete',
        'third-party/curl', 'third-party/cpp-steam-tools'], text=True).splitlines(),
    'buildCacheSha256': sha(BUILD / 'CMakeCache.txt'),
    'vendorPatch': {'path': 'third-party/gf-complete/src/gf_cpu.c', 'sha256': sha(ROOT / 'third-party/gf-complete/src/gf_cpu.c'), 'committed': False},
}
(EVIDENCE / 'identity.json').write_text(json.dumps(record, indent=2) + '\n')
print(f"Recorded {len(paths)} source files, {len(artifacts)} artifacts, and {len(comparisons)} passing source comparisons.")
