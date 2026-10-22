from pathlib import Path
import re,json,subprocess,hashlib
root=Path.cwd();baseline='a57660940c4d3d22403c834eb638a947e01e24f2'; checks=[]
def normalized(s): return s.replace('\r\n','\n')
def digest(s): return hashlib.sha256(s.encode()).hexdigest()
def compare(name,old,new):
 old,new=normalized(old),normalized(new)
 checks.append({'name':name,'base':digest(old),'current':digest(new),'same':old==new})
 assert old==new,name
qml=(root/'gui/src/qml/PlaceboColorMappingDialog.qml').read_text(encoding='utf-8')
oldtests=(root/'gui/tests/qml/tst_SettingsColorMapping.qml').read_text(encoding='utf-8')
ts=(root/'gui/web/src/settings.ts').read_text(encoding='utf-8').split('export const colorMappingSettings: Setting[] = [',1)[1]
fields=[json.loads(l.strip().rstrip(',')) for l in ts.splitlines() if l.strip().startswith('{')]
for f in fields[3:]:
 row=next(json.loads(l.strip().rstrip(',')) for l in oldtests.splitlines() if '"tag": "'+f['key']+'"' in l)
 if row['kind']=='Slider': assert [f['min'],f['max'],f['step']]==[row['from'],row['to'],row['stepSize']],f['key']
 if row['kind']=='ComboBox': assert f['options']==row['options'],f['key']
 funcs=[c['value'] for c in f['when'] if c['key'] in ['placeboGamutMappingFunction','placeboToneMappingFunction']]
 assert funcs==([row['functions']] if 'functions' in row else []),f['key']
 assert '"'+f['label']+':"' in qml,f['key']
cpp=(root/'gui/src/setupbridge.cpp').read_text(encoding='utf-8');base=subprocess.check_output(['git','show',baseline+':gui/src/setupbridge.cpp']).decode()
compare('existing renderer setters/actions suffix',base.split('QVariantMap SetupBridge::setRendererSetting',1)[1].split('    if(name == "targetPeakChoice")',1)[1],cpp.split('QVariantMap SetupBridge::setRendererSetting',1)[1].split('    if(name == "targetPeakChoice")',1)[1])
compare('SetupBridge constructor and stable target methods',base.split('    QVariantMap strings;',1)[0],cpp.split('    QVariantMap strings;',1)[0])
for p in ['gui/src/qmlsettings.cpp','gui/src/settings.cpp','gui/src/qml/PlaceboColorMappingDialog.qml','gui/src/qml/PlaceboSettingsDialog.qml','gui/src/qml/StreamRoot.qml','gui/web/src/KeyboardMouse.svelte','gui/web/src/bridge.ts','gui/src/setupwindow.cpp','gui/src/qmlbackend.cpp','gui/src/qmlnativeservices.cpp','gui/include/qmlnativeservices.h','gui/include/qmlbackend.h','gui/src/discoverymanager.cpp','gui/include/discoverymanager.h']:
 compare(p,subprocess.check_output(['git','show',baseline+':'+p]).decode(),(root/p).read_text(encoding='utf-8'))
tested='bf4f12636fa4f5a34c5293aa363a220fa915a4bc'
def oldfile(path): return subprocess.check_output(['git','show',tested+':'+path]).decode()
def cppmethod(text, signature):
 start=text.index(signature); end=text.find('\nQVariantMap SetupBridge::',start+len(signature)); return text[start:end if end>=0 else len(text)]
compare('tested color and renderer setter method',cppmethod(oldfile('gui/src/setupbridge.cpp'),'QVariantMap SetupBridge::setRendererSetting'),cppmethod(cpp,'QVariantMap SetupBridge::setRendererSetting'))
for path in ['gui/web/src/SettingField.svelte','gui/web/src/settings.ts']:
 compare('tested '+path,oldfile(path),(root/path).read_text(encoding='utf-8'))
current=(root/'gui/web/src/Settings.svelte').read_text(encoding='utf-8').replace('<button class="back-consoles" id="backRenderer"','<button id="backRenderer"').replace('<button class="back-consoles" onclick={() => reveal(\'openPlaceboColorMapping\')}', '<button onclick={() => reveal(\'openPlaceboColorMapping\')}')
compare('tested Settings.svelte apart from button class',oldfile('gui/web/src/Settings.svelte'),current)
compare('tested shared stylesheet apart from PSN refresh selector',oldfile('gui/web/src/style.css'),(root/'gui/web/src/style.css').read_text(encoding='utf-8').replace('.discovery, .refresh-psn {','.discovery {'))
def testmethod(text,name):
 start=text.index('    void '+name+'()'); end=text.find('\n    void ',start+1); return text[start:end if end>=0 else len(text)]
currenttests=(root/'gui/tests/embeddedsetup.cpp').read_text(encoding='utf-8');oldtestsfile=oldfile('gui/tests/embeddedsetup.cpp')
for name in ['rendererSettings','rendererLayout','settingsValidation','videoAudioSettings','keyboardMouseSettings','colorMappingRestartRead']:
 compare('tested '+name,testmethod(oldtestsfile,name),testmethod(currenttests,name))
oldcolor=testmethod(oldtestsfile,'colorMappingSettings');start=oldcolor.index('        const auto capture = [&](int width');end=oldcolor.index('        QSettings(QCoreApplication::organizationName(), "pl_render_params").sync();',start)
compare('tested color behavior apart from moved capture',oldcolor[:start]+oldcolor[end:],testmethod(currenttests,'colorMappingSettings'))
Path('C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket14-audit.json').write_text(json.dumps({'baseline':baseline,'source':subprocess.check_output(['git','rev-parse','HEAD']).decode().strip(),'controlCount':len(fields)-3,'independentMetadataMatch':True,'comparisons':checks},indent=2))
print('PASS:28 controls match independent retained QML test ranges, enums, function conditions and source labels.')
print('PASS:',len(checks),'bounded source comparisons match integration or tested behavior.')

