from pathlib import Path
import hashlib,json,subprocess
root=Path.cwd();evidence=root/'.scratch/web-ui-migration/evidence/ticket13'
def git(*args):return subprocess.check_output(['git',*args],text=True).replace('\r\n','\n')
def body(commit,file,signature):
 s=git('show',f'{commit}:{file}');a=s.index(signature);b=s.find('\n    void ',a+len(signature)) if file.endswith('embeddedsetup.cpp') else s.find('\n}\n',a)+3
 if b<0:b=len(s)
 return s[a:b].strip()
def sha(value):return hashlib.sha256(value).hexdigest()
checks=[]
for commit,file,signatures in [
 ('5b5f341b','gui/src/settings.cpp',['bool Settings::ExportPlaceboSettings(', 'bool Settings::ImportPlaceboSettings(']),
 ('5b5f341b','gui/src/qmlsettings.cpp',['QVariantMap QmlSettings::exportPlaceboSettings()', 'QVariantMap QmlSettings::importPlaceboSettings()']),
 ('5b5f341b','gui/tests/embeddedsetup.cpp',['    void rendererFiles()']),
 ('62f7f839','gui/tests/embeddedsetup.cpp',['    void rendererSettings()']),
 ('62f7f839','gui/src/setupbridge.cpp',['QVariantMap SetupBridge::setRendererSetting(', 'QVariantMap SetupBridge::rendererAction('])]:
 for signature in signatures:
  old=body(commit,file,signature);new=body('HEAD',file,signature)
  checks.append(dict(testedCommit=commit,file=file,signature=signature,unchanged=old==new,bodySha256=sha(new.encode())))
assert all(c['unchanged'] for c in checks),checks
paths=['gui/src/settings.cpp','gui/include/settings.h','gui/src/qmlsettings.cpp','gui/include/qmlsettings.h','gui/src/qmlbackend.cpp','gui/src/setupbridge.cpp','gui/include/setupbridge.h','gui/web/src/settings.ts','gui/web/src/SettingField.svelte','gui/web/src/Settings.svelte','gui/web/src/bridge.ts','gui/tests/embeddedsetup.cpp','gui/tests/CMakeLists.txt','gui/src/qml/PlaceboSettingsDialog.qml']
artifacts=[Path('C:/Users/david/.cache/chiaki-msvc-native/ticket13-build/gui/tests/chiaki-embedded-setup-tests.exe'),Path('C:/Users/david/.cache/chiaki-msvc-native/ticket13-build/gui/tests/chiaki-console-screen-tests.exe')]
info=dict(sourceCommit=git('rev-parse','HEAD').strip(),compiledCommit='dab7697b8d7e1827ac388424512f419cf3308d79',sourceHashes={str(p):sha((root/p).read_bytes()) for p in paths},artifacts={str(p):sha(p.read_bytes()) for p in artifacts},settingsRunArtifactSha256='f81ed040c082ebdaaca39e8fd6a46e0e97acbc661b66c0974062ac9e4573f66f',fileRunArtifactSha256=None,fileRunArtifactLimit='The 5b5f341b executable was not hashed before a later test-only rebuild.',bodyComparisons=checks,vendorDiffSha256=sha(git('-C','third-party/gf-complete','diff').encode()))
(evidence/'identity.json').write_text(json.dumps(info,indent=2)+'\n')
print(json.dumps({'unchangedBodies':len(checks),'allUnchanged':all(c['unchanged'] for c in checks),'artifact':info['artifacts']},indent=2))
