from pathlib import Path
import hashlib
import json
import subprocess

ROOT = Path(__file__).resolve().parents[4]
OUT = Path(__file__).resolve().parent
BUILD = Path(r'C:/Users/david/.cache/chiaki-msvc-native/ticket04-build')

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)

def source(ref, name):
    return git('show', f'{ref}:{name}').decode('utf-8')

def method(text, signature):
    start = text.index(signature)
    marker = '\n    }' if signature.startswith('    ') else '\n}'
    end = text.index(marker, start) + len(marker)
    return text[start:end]

checks = []
def same_file(ref, name):
    checks.append({'reference': ref, 'file': name, 'equal': source(ref, name) == source('HEAD', name)})

def same_method(ref, name, signature):
    checks.append({'reference': ref, 'file': name, 'method': signature.strip(),
        'equal': method(source(ref, name), signature) == method(source('HEAD', name), signature)})

for name in ['gui/src/applicationcontroller.cpp', 'gui/include/applicationcontroller.h',
             'gui/src/main.cpp', 'gui/src/qmlmainwindow.cpp', 'gui/include/qmlmainwindow.h',
             'gui/src/streamsession.cpp', 'gui/include/streamsession.h',
             'gui/src/qmlbackend.cpp', 'gui/include/qmlbackend.h',
             'gui/src/setupwindow.cpp', 'gui/include/setupwindow.h',
             'gui/src/psnsigninwindow.cpp', 'gui/include/psnsigninwindow.h',
             'gui/tests/nativeservices.cpp']:
    same_file('7cd7ec8e', name)
for name in ['gui/src/applicationcontroller.cpp', 'gui/src/qmlmainwindow.cpp',
             'gui/include/qmlmainwindow.h', 'gui/src/streamsession.cpp', 'gui/include/streamsession.h']:
    same_file('55140aa9', name)
for signature in ['void QmlBackend::releaseSession()', 'void QmlBackend::detachWindow(',
                  'void QmlBackend::createSession(']:
    same_method('55140aa9', 'gui/src/qmlbackend.cpp', signature)
for signature in ['    void decodedSessionCleanup()', '    void nativeInitializationFailure()']:
    same_method('55140aa9', 'gui/tests/embeddedsetup.cpp', signature)
same_method('7cd7ec8e', 'gui/tests/embeddedsetup.cpp', '    void suspendClosesSignIn()')
for signature in ['void SetupBridge::setSuspended(', 'QVariantMap SetupBridge::connectConsole(',
                  'QVariantMap SetupBridge::dismissStreamError(']:
    same_method('55140aa9', 'gui/src/setupbridge.cpp', signature)
same_method('2abe34f5', 'gui/src/setupbridge.cpp', 'QVariantMap SetupBridge::setMediaSetting(')
assert all(item['equal'] for item in checks), checks
(OUT/'source-equivalence.json').write_text(json.dumps(checks, indent=2)+'\n')

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

changed = git('diff', '--name-only', 'b90c7f27', 'HEAD', '--', 'gui').decode().splitlines()
paths = [name for name in changed if (ROOT/name).is_file()]
artifacts = [BUILD/'gui/chiaki.exe', BUILD/'gui/tests/chiaki-embedded-setup-tests.exe',
             BUILD/'gui/tests/chiaki-native-services-tests.exe', BUILD/'gui/web/app.js', BUILD/'gui/web/app.css']
identity = {
    'sourceCommit': git('rev-parse', 'HEAD').decode().strip(),
    'sourceTree': git('rev-parse', 'HEAD^{tree}').decode().strip(),
    'integrationTip': '2abe34f588a9c91d7e39cbb1b991559e69bc1666',
    'initialClaim': 'b90c7f27d4f8c36c5272502fe764058fc9664fcf',
    'worktree': str(ROOT), 'build': str(BUILD),
    'finalBuildArtifacts': {str(path): digest(path) for path in artifacts},
    'finalBuildRuntimePolicy': 'Compiled after additive ticket12 merge; runtime evidence reused by source equality. The affected runtime group binaries are in tested-affected-artifacts.json.',
    'sourceFilesSha256': {name: digest(ROOT/name) for name in paths},
    'frontendLockSha256': digest(ROOT/'gui/web/package-lock.json'),
    'vendorPatchExcluded': 'scripts/windows-vc/gf-complete.patch',
    'vendorPatchSha256': digest(ROOT/'scripts/windows-vc/gf-complete.patch'),
    'earlierRuntimeBinaryDigests': None,
    'earlierRuntimeIdentityLimit': 'The pre-merge init/decoded harness binaries were overwritten before SHA256 recording. Logs and source-equivalence checks are retained; no retroactive binary identity is claimed.'
}
(OUT/'identity.json').write_text(json.dumps(identity, indent=2)+'\n')
print(f"Recorded {len(checks)} passing source equality checks, {len(paths)} source hashes, and {len(artifacts)} final artifact hashes.")
