from pathlib import Path
import hashlib,json,subprocess
root=Path.cwd();evidence=root/'.scratch/web-ui-migration/evidence/ticket13'
def git(*args):return subprocess.check_output(['git',*args],text=True).replace('\r\n','\n')
def body(commit,file,signature):
 s=git('show',f'{commit}:{file}');a=s.index(signature);b=s.find('\n    void ',a+len(signature)) if file.endswith('embeddedsetup.cpp') else s.find('\n}\n',a)+3
 if b<0:b=len(s)
 return s[a:b].strip()
def sha(value):return hashlib.sha256(value).hexdigest()
def comparison(commit,file,signature,extract=lambda value:value,note='Full function body unchanged'):
 old=extract(body(commit,file,signature));new=extract(body('HEAD',file,signature))
 return dict(testedCommit=commit,file=file,signature=signature,comparison=note,unchanged=old==new,bodySha256=sha(new.encode()))
checks=[]
for commit,file,signatures in [
 ('5b5f341b','gui/src/settings.cpp',['bool Settings::ExportPlaceboSettings(', 'bool Settings::ImportPlaceboSettings(']),
 ('5b5f341b','gui/src/qmlsettings.cpp',['QVariantMap QmlSettings::exportPlaceboSettings()', 'QVariantMap QmlSettings::importPlaceboSettings()']),
 ('5b5f341b','gui/tests/embeddedsetup.cpp',['    void rendererFiles()'])]:
 for signature in signatures:checks.append(comparison(commit,file,signature))
checks.append(comparison('62f7f839','gui/tests/embeddedsetup.cpp','    void rendererSettings()',lambda s:s.replace('QmlNativeServices services(&settings, {}, [](const QString &, const QStringList &) { return false; });','QmlNativeServices services(&settings);'),'Only the controlled external process launcher was added to prevent test relaunch. All prior assertions are unchanged.'))
def unrelatedSettings(s):
 a=s.index('    if(name == "vSyncEnabled")');b=s.index('    if(name == "videoPreset")',a);s=s[:a]+s[b:]
 a=s.index('    if(name == "rendererBackend")');b=s.index('    if(name == "displayTargetPrim")',a);return s[:a]+s[b:]
checks.append(comparison('62f7f839','gui/src/setupbridge.cpp','QVariantMap SetupBridge::setRendererSetting(',unrelatedSettings,'All setting branches except the two intentionally changed restart triggers are unchanged.'))
checks.append(comparison('62f7f839','gui/src/setupbridge.cpp','QVariantMap SetupBridge::rendererAction(',lambda s:s[:s.index('    if(action == "restartRenderer")')].strip(),'File and documentation action branches before restartRenderer are unchanged.'))
assert all(c['unchanged'] for c in checks),checks
paths=['gui/src/settings.cpp','gui/include/settings.h','gui/src/qmlsettings.cpp','gui/include/qmlsettings.h','gui/src/qmlbackend.cpp','gui/include/qmlbackend.h','gui/src/qmlnativeservices.cpp','gui/include/qmlnativeservices.h','gui/src/setupbridge.cpp','gui/include/setupbridge.h','gui/web/src/settings.ts','gui/web/src/SettingField.svelte','gui/web/src/Settings.svelte','gui/web/src/bridge.ts','gui/tests/embeddedsetup.cpp','gui/tests/CMakeLists.txt','gui/src/qml/PlaceboSettingsDialog.qml','gui/src/qml/SettingsDialog.qml']
artifacts=[Path('C:/Users/david/.cache/chiaki-msvc-native/ticket13-build/gui/tests/chiaki-embedded-setup-tests.exe'),Path('C:/Users/david/.cache/chiaki-msvc-native/ticket13-build/gui/tests/chiaki-console-screen-tests.exe')]
def recordedHash(name):return json.loads((evidence/name).read_text(encoding='utf-8-sig'))['Hash'].lower()
info=dict(sourceCommit=git('rev-parse','HEAD').strip(),compiledCommit='5a83520f57f1c479f815e4e3096087f70fa94a5b',sourceHashes={str(p):sha((root/p).read_bytes()) for p in paths},artifacts={str(p):sha(p.read_bytes()) for p in artifacts},restartRedCommit='b055ee1200b98e08583b92f9d258a11d64c9570c',restartRedArtifactSha256=recordedHash('ticket13-restart-red-artifact.json'),restartGreenArtifactSha256=recordedHash('ticket13-restart-green-artifact.json'),earlierLayoutArtifactSha256='c72fbfbccc340cf1090c19c2d760d9e6dcf6202c4e5bc23ca9cce6c3b9949a10',settingsRunArtifactSha256='f81ed040c082ebdaaca39e8fd6a46e0e97acbc661b66c0974062ac9e4573f66f',fileRunArtifactSha256=None,fileRunArtifactLimit='The 5b5f341b executable was not hashed before a later test-only rebuild.',bodyComparisons=checks,vendorDiffSha256=sha(git('-C','third-party/gf-complete','diff').encode()))
assert info['artifacts'][str(artifacts[0])] == info['restartGreenArtifactSha256']
(evidence/'identity.json').write_text(json.dumps(info,indent=2)+'\n')
print(json.dumps({'comparisons':len(checks),'allUnchanged':all(c['unchanged'] for c in checks),'artifact':info['artifacts']},indent=2))
