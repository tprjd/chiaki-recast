from pathlib import Path
import shutil, subprocess, hashlib, json, sys
root=Path(r'C:/Users/david/.cache/chiaki-ui-native'); work=Path.cwd(); qa=root/(sys.argv[1] if len(sys.argv)>1 else 'disconnect-prep01'); source=root/'source'
qa.mkdir(exist_ok=False)
paths=list(dict.fromkeys(subprocess.check_output(['git','ls-files','gui'],text=True).splitlines()+['gui/src/qml/StreamDisconnectDialog.qml']))
manifest={}
for name in paths:
    p=source/name
    if not (work/name).is_file(): continue
    manifest[name]=hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else None
    if p.exists():
        backup=qa/'backup'/name; backup.parent.mkdir(parents=True,exist_ok=True); shutil.copy2(p,backup)
    p.parent.mkdir(parents=True,exist_ok=True); shutil.copyfile(work/name,p)
(qa/'manifest.json').write_text(json.dumps(manifest,indent=2))
shutil.copyfile(root/'build/CMakeCache.txt',qa/'CMakeCache.txt')
(qa/'production-source.json').write_text(json.dumps({n:hashlib.sha256((source/n).read_bytes()).hexdigest() for n in manifest},indent=2))
def edit(name,old,new):
    p=source/name; text=p.read_text(); assert old in text,name+': '+old; p.write_text(text.replace(old,new))
edit('gui/src/main.cpp','QApplication app(argc, argv);','QApplication app(argc, argv);\n    extern void initializeDisconnectQa(); initializeDisconnectQa();')
edit('gui/src/settings.cpp','settings(QCoreApplication::organizationName(),','settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),')
edit('gui/src/settings.cpp','\n\tsettings.setFallbacksEnabled(false);','''
    if (!QFileInfo(settings.fileName()).absoluteFilePath().startsWith(qEnvironmentVariable("CHIAKI_DISCONNECT_QA_ROOT")) || !QFileInfo(default_settings.fileName()).absoluteFilePath().startsWith(qEnvironmentVariable("CHIAKI_DISCONNECT_QA_ROOT")) || !QFileInfo(placebo_settings.fileName()).absoluteFilePath().startsWith(qEnvironmentVariable("CHIAKI_DISCONNECT_QA_ROOT"))) qFatal("Disconnect QA settings escaped isolation");
    settings.setFallbacksEnabled(false);''')
edit('gui/src/qmlbackend.cpp','setDiscoveryEnabled(true);','setDiscoveryEnabled(false);')
edit('gui/src/qmlbackend.cpp','void QmlBackend::refreshPsnToken()\n{','void QmlBackend::refreshPsnToken()\n{\n    return;')
edit('gui/src/qmlbackend.cpp','void QmlBackend::updatePsnHosts()\n{','void QmlBackend::updatePsnHosts()\n{\n    return;')
edit('gui/src/qmlbackend.cpp','    try {\n        discovery_manager.SendWakeup','    return true;\n    try {\n        discovery_manager.SendWakeup')
edit('gui/src/sessionlog.cpp','QString CreateLogFilename()\n{','QString CreateLogFilename()\n{\n    return {};')
edit('gui/include/qmlbackend.h','void profileChanged();','void profileChanged();\n    Settings *nativeQaSettings() { return settings; }')
edit('gui/include/streamsession.h','bool IsConnecting()','void nativeQaEvent(ChiakiEvent *event) { Event(event); }\n        bool nativeQaInputBlocked() const { return input_block == 1; }\n        bool IsConnecting()')
for method,property in [('Start','qaStarts'),('Stop','qaStops'),('GoToBed','qaSleeps')]:
    edit('gui/src/streamsession.cpp','void StreamSession::'+method+'()\n{','void StreamSession::'+method+'()\n{\n    qApp->setProperty("'+property+'", qApp->property("'+property+'").toInt()+1); return;')
edit('gui/include/qmlmainwindow.h','bool hasVideo() const;','bool hasVideo() const;\n    int nativeQaGrabCount() const { return grab_input; }\n    QImage nativeQaImage();')
edit('gui/src/qmlmainwindow.cpp','.sampleable = true,\n        .renderable = true,','.sampleable = true,\n        .renderable = true,\n        .host_readable = true,')
edit('gui/src/qmlmainwindow.cpp','void QmlMainWindow::render()\n{','void QmlMainWindow::render()\n{\n    qApp->setProperty("qaRenderCalls",qApp->property("qaRenderCalls").toInt()+1);')
edit('gui/src/qmlmainwindow.cpp','quick_render->render();','quick_render->render(); qApp->setProperty("qaQuickPaints",qApp->property("qaQuickPaints").toInt()+1);')
p=source/'gui/src/qmlmainwindow.cpp'; p.write_text(p.read_text()+Path(__file__).with_name('native-render-readback.cpp.inc').read_text())
shutil.copyfile(Path(__file__).with_name('native-contracts.cpp'),qa/'native-disconnect.cpp')
p=source/'gui/CMakeLists.txt'; p.write_text(p.read_text()+'\ntarget_sources(chiaki PRIVATE '+(qa/'native-disconnect.cpp').as_posix()+')\n')
(qa/'tested-source.json').write_text(json.dumps({n:hashlib.sha256((source/n).read_bytes()).hexdigest() for n in manifest},indent=2))
print('Staged',len(manifest),'GUI files with isolated native hooks')
