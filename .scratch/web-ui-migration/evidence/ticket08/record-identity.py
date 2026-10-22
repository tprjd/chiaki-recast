import hashlib
import json
import pathlib
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[4] if not sys.argv[1:] else pathlib.Path(sys.argv[1]).resolve()
build = pathlib.Path('C:/Users/david/.cache/chiaki-msvc-native/ticket08-build')
base = '5a61e86a7a2a190a147e9036b2c11dbbde98d079'


def git(*args):
    return subprocess.check_output(['git', '-C', str(root), *args]).decode('utf-8')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def source(path):
    return (root / path).read_text(encoding='utf-8').replace('\r\n', '\n')


preserved = [
    'gui/src/qmlmainwindow.cpp',
    'gui/src/applicationcontroller.cpp', 'gui/src/streaminputownership.cpp',
    'gui/src/qml/ConnectionView.qml', 'gui/src/qml/StreamRoot.qml',
    'gui/src/qml/StreamView.qml', 'gui/src/qml/LiveConsolePinHost.qml',
    'gui/src/qml/LiveConsolePinContent.qml',
]
changed = git('diff', '--name-only', base, '--', 'gui').splitlines()
paths = sorted(set(changed + preserved + ['gui/web/package-lock.json', 'gui/include/psnrequests.h']))
paths = [p for p in paths if (root / p).is_file()]
comparisons = []
for path in preserved:
    old = git('show', base + ':' + path).replace('\r\n', '\n')
    current = source(path)
    comparisons.append({'path': path, 'equal': current == old, 'sha256': digest(current.encode())})

session = source('gui/src/streamsession.cpp')
session_changes = [
    ('StreamSession::StreamSession(const StreamSessionConnectInfo &connect_info, QObject *parent,\n\tstd::function<ChiakiErrorCode(ChiakiSession *)> start_request)',
     'StreamSession::StreamSession(const StreamSessionConnectInfo &connect_info, QObject *parent)'),
    ('\n\tstart_request(std::move(start_request)),', ''),
    ('ChiakiErrorCode err = start_request(&session);', 'ChiakiErrorCode err = chiaki_session_start(&session);'),
]
normalized_session = session
for new, old in session_changes:
    assert normalized_session.count(new) == 1, new
    normalized_session = normalized_session.replace(new, old)
comparisons.append({'path': 'gui/src/streamsession.cpp', 'equal': normalized_session == git('show', base + ':gui/src/streamsession.cpp').replace('\r\n', '\n'),
                    'sha256': digest(session.encode()), 'allowedTransportSubstitutions': session_changes})


def function(text, name):
    start = text.index('void QmlBackend::' + name + '(') if 'void QmlBackend::' + name + '(' in text else text.index('bool QmlBackend::' + name + '(')
    return text[start:text.index('\n}\n', start) + 3]


backend = source('gui/src/qmlbackend.cpp')
old_backend = git('show', base + ':gui/src/qmlbackend.cpp').replace('\r\n', '\n')
for name in ['startPsnConnection', 'retryConnection', 'cancelConnection', 'dismissConnection',
             'finishPsnCleanup', 'checkPsnConnection', 'setConnectState', 'releaseSession', 'createSession']:
    current = function(backend, name)
    normalized = current.replace('new StreamSession(session_info, this, psn_requests.startSession)', 'new StreamSession(session_info, this)') if name == 'createSession' else current
    comparisons.append({'function': name, 'equal': normalized == function(old_backend, name), 'sha256': digest(current.encode()),
                        'allowedTransportSubstitution': name == 'createSession'})
artifacts = ['gui/chiaki.exe', 'gui/tests/chiaki-embedded-setup-tests.exe', 'gui/web/app.js', 'gui/web/app.css']
result = {
    'head': git('rev-parse', 'HEAD').strip(), 'tree': git('rev-parse', 'HEAD^{tree}').strip(),
    'status': git('status', '--short'), 'sourceBase': base,
    'sources': {p: digest(source(p).encode()) for p in paths},
    'artifacts': {p: digest((build / p).read_bytes()) for p in artifacts},
    'preserved': comparisons,
}
assert all(row['equal'] for row in comparisons), comparisons
print(json.dumps(result, indent=2))
