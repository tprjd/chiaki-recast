from pathlib import Path
import difflib
import hashlib
import json
import os
import shutil
import sys

# This helper stages test-only hooks in the reserved local native build cache.
root = Path(r'C:\Users\david\.cache\chiaki-ui-native')
source = root / 'source'
work = Path(__file__).resolve().parents[3]
restoring = len(sys.argv) > 1 and sys.argv[1] == '--restore'
name = sys.argv[2] if restoring else sys.argv[1] if len(sys.argv) > 1 else 'psn-setup-prep-rerun'
qa = (root / name).resolve()
assert qa.parent == root.resolve() and name.startswith('psn-setup-prep'), 'Use a PSN setup preparation directory in the native cache'

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.exists() else None

if restoring:
    manifest = json.loads((qa / 'manifest.json').read_text(encoding='utf-8'))
    tested = {}
    patch = []
    for name, original in manifest.items():
        path = (source / name).resolve()
        assert path.is_relative_to(source.resolve())
        tested[name] = digest(path)
        before = (qa / 'backup' / name).read_text(encoding='utf-8').splitlines(True) if original else []
        after = path.read_text(encoding='utf-8').splitlines(True) if path.exists() else []
        patch.extend(difflib.unified_diff(before, after, fromfile='a/' + name, tofile='b/' + name))
        if original:
            shutil.copy2(qa / 'backup' / name, path)
            os.utime(path, None)
        elif path.exists():
            path.unlink()
        assert digest(path) == original
    (qa / 'tested-source-sha256.json').write_text(json.dumps(tested, indent=2), encoding='utf-8')
    (qa / 'qa.patch').write_text(''.join(patch), encoding='utf-8')
    shutil.copy2(qa / 'CMakeCache.txt', root / 'build/CMakeCache.txt')
    (qa / 'restored-source-sha256.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
    print('Source hashes and original CMake cache restored. Reconfigure and rebuild the original target before releasing the cache.')
    raise SystemExit(0)

qa.mkdir(exist_ok=False)
paths = [path.relative_to(work).as_posix() for folder in ['gui/src', 'gui/include'] for path in (work / folder).rglob('*') if path.is_file()]
paths.append('gui/CMakeLists.txt')
manifest = {}
for name in paths:
    path = source / name
    manifest[name] = digest(path)
    if path.exists():
        backup = qa / 'backup' / name
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, backup)
    path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(work / name, path)
    os.utime(path, None)
(qa / 'manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
shutil.copy2(root / 'build/CMakeCache.txt', qa / 'CMakeCache.txt')
shutil.copy2(Path(__file__).with_name('ticket01-native-contracts.cpp'), qa / 'contracts.cpp')

def edit(name, old, new):
    path = source / name
    text = path.read_text(encoding='utf-8')
    assert old in text, name + ': missing QA edit'
    path.write_text(text.replace(old, new), encoding='utf-8')

edit('gui/src/main.cpp', 'QApplication app(argc, argv);', 'QApplication app(argc, argv);\n    extern void initializePsnSetupQa(); initializePsnSetupQa();')
edit('gui/src/settings.cpp', 'settings(QCoreApplication::organizationName(),', 'settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),')
edit('gui/src/settings.cpp', '\n\tsettings.setFallbacksEnabled(false);', '\n    if(!QFileInfo(settings.fileName()).absoluteFilePath().startsWith(qEnvironmentVariable("CHIAKI_PSN_SETUP_QA_ROOT")) || !QFileInfo(default_settings.fileName()).absoluteFilePath().startsWith(qEnvironmentVariable("CHIAKI_PSN_SETUP_QA_ROOT")) || !QFileInfo(placebo_settings.fileName()).absoluteFilePath().startsWith(qEnvironmentVariable("CHIAKI_PSN_SETUP_QA_ROOT"))) qFatal("PSN settings escaped isolation");\n\tsettings.setFallbacksEnabled(false);')
edit('gui/src/qmlbackend.cpp', 'setDiscoveryEnabled(true);', 'setDiscoveryEnabled(false);')
edit('gui/src/qmlbackend.cpp', 'void QmlBackend::updatePsnHosts()\n{', 'void QmlBackend::updatePsnHosts()\n{\n    qApp->setProperty("qaHostRefreshes", qApp->property("qaHostRefreshes").toInt()+1); return;')
edit('gui/src/qmlbackend.cpp', 'QDesktopServices::openUrl(url);', 'qApp->setProperty("qaBrowserOpens", qApp->property("qaBrowserOpens").toInt()+1);')
edit('gui/src/qmlbackend.cpp', 'window->showMaximized();', ';')
edit('gui/src/jsonrequester.cpp', '#include <QTimer>', '#include <QTimer>\n#include <QCoreApplication>\n#include <QVariant>')
edit('gui/src/jsonrequester.cpp', '    QUrl q_url(url);', '    auto requests = qApp->property("qaJsonRequests").toList(); requests.append(QVariant::fromValue(static_cast<QObject *>(this))); qApp->setProperty("qaJsonRequests", requests); return;\n    QUrl q_url(url);')
for method in ['showMaximized();', 'showNormal();', 'showFullScreen();']:
    edit('gui/src/qmlmainwindow.cpp', method, ';')
edit('gui/include/qmlmainwindow.h', 'bool hasVideo() const;', 'QImage nativeQaImage();\n    bool hasVideo() const;')
path = source / 'gui/src/qmlmainwindow.cpp'
path.write_text(path.read_text(encoding='utf-8') + '\nQImage QmlMainWindow::nativeQaImage() { if (render_backend != RenderBackend::OpenGL || !quick_fbo || !makeOpenGLContextCurrent()) return {}; const QImage image = quick_fbo->toImage(); doneOpenGLContextCurrent(); return image; }\n', encoding='utf-8')
path = source / 'gui/CMakeLists.txt'
path.write_text(path.read_text(encoding='utf-8') + '\ntarget_sources(chiaki PRIVATE ' + (qa / 'contracts.cpp').as_posix() + ')\n', encoding='utf-8')
print('Staged isolated PSN setup contracts at', qa)
