from pathlib import Path
import subprocess,hashlib,json,gzip
root=Path.cwd(); evidence=root/'.scratch/web-ui-migration/evidence/ticket14'
def sha(data): return hashlib.sha256(data).hexdigest()
paths=['gui/src/setupbridge.cpp','gui/include/setupbridge.h','gui/src/qmlbackend.cpp','gui/include/qmlbackend.h','gui/src/qmlnativeservices.cpp','gui/include/qmlnativeservices.h','gui/src/discoverymanager.cpp','gui/include/discoverymanager.h','gui/src/qmlsettings.cpp','gui/src/settings.cpp','gui/src/setupwindow.cpp','gui/src/qml/StreamRoot.qml','gui/src/qml/PlaceboSettingsDialog.qml','gui/src/qml/PlaceboColorMappingDialog.qml','gui/web/src/Settings.svelte','gui/web/src/settings.ts','gui/web/src/SettingField.svelte','gui/web/src/KeyboardMouse.svelte','gui/web/src/bridge.ts','gui/web/src/style.css','gui/web/src/App.svelte','gui/web/package-lock.json','gui/tests/embeddedsetup.cpp','gui/tests/CMakeLists.txt']
sources={p:{'sha256':sha((root/p).read_bytes()),'normalizedSha256':sha((root/p).read_bytes().replace(b'\r\n',b'\n'))} for p in paths}
prelaunch={}
for phase in ['red','green','final','layout','layout-fixed']:
 p=evidence/('ticket14-'+phase+'-source.json.gz');data=gzip.decompress(p.read_bytes());saved=json.loads(data)
 prelaunch[phase]={'record':p.name,'fullRecordSha256':sha(data),'source':saved['head'],'executableSha256':saved['exeSha256'],'diffSha256':saved['diffSha256'],'newFiles':saved['newFiles']}
build=Path('C:/Users/david/.cache/chiaki-msvc-native/ticket14-build')
artifacts={str(p):sha(p.read_bytes()) for p in [build/'gui/tests/chiaki-embedded-setup-tests.exe',build/'gui/web/app.js',build/'gui/web/app.css']}
vendor=subprocess.check_output(['git','-C','third-party/gf-complete','diff','--binary'])
files={p.name:sha(p.read_bytes()) for p in evidence.iterdir() if p.is_file() and p.name!='identity.json'}
manifest={'sourceCommit':subprocess.check_output(['git','rev-parse','HEAD']).decode().strip(),'integrationBaseline':'a57660940c4d3d22403c834eb638a947e01e24f2','sources':sources,'prelaunch':prelaunch,'currentBuildArtifacts':artifacts,'localVendorDiffSha256':sha(vendor),'evidence':files,'newProductionFiles':[],'limits':'Offscreen behavior and layout only. 1280 CSS px capture is current scope. Narrow/400 CSS px zoom images are historical. Supported-width 200 percent acceptance remains in ticket23. No GPU, physical focus, live playback, HDR or latency result.'}
(evidence/'identity.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
print('Recorded',len(sources),'source files,',len(files),'evidence files and',len(prelaunch),'prelaunch identities.')
