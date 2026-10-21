"""Verify the final normal native target, resources, and preserved application."""
from pathlib import Path
import hashlib, json, subprocess

here=Path(__file__).resolve().parent
repo=here.parents[3]
cache=Path('C:/Users/david/.cache/chiaki-ui-native')
sha=lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
paths=subprocess.check_output(['git','ls-files','CMakeLists.txt','cmake','third-party','gui','lib','include','cli'],cwd=repo,text=True).splitlines()
paths=[name for name in paths if (repo/name).is_file()]
for name in paths:
    assert sha(repo/name)==sha(cache/'source'/name),name
exe=cache/'stream-acceptance-normal-bin/chiaki.exe'
for marker in [b'initializeDisconnectQa',b'nativeQa',b'CHIAKI_DISCONNECT_QA_ROOT',b'qaBackground',b'AcceptanceLabels',b'StreamHarness.qml',b'tst_StreamToolbar.qml']:
    assert marker not in exe.read_bytes(),marker
resource_inputs=[]
for resource in ['gui/res/resources.qrc','gui/src/qml/qml.qrc']:
    text=(cache/'source'/resource).read_text()
    for marker in ['prototype','fixtures','tst_Stream','StreamHarness','ticket05']:
        assert marker not in text,(resource,marker)
for path in (cache/'build/gui/CMakeFiles/chiaki_autogen.dir').glob('AutoRcc*Info.json'):
    data=json.loads(path.read_text())
    for name in data['INPUTS']:
        actual=Path(name).resolve()
        assert actual.is_relative_to((cache/'source').resolve()),name
        relative=actual.relative_to((cache/'source').resolve()).as_posix()
        assert relative in paths,relative
        assert sha(actual)==sha(repo/relative),relative
        resource_inputs.append(relative)
link_line=next(line for line in (cache/'build/build.ninja').read_text().splitlines() if line.startswith('build ') and 'chiaki.exe:' in line)
for marker in ['native-disconnect','screen-tests','/tests/','ticket05']:
    assert marker not in link_line,marker
record={'revision':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
        'allProductionInputsMatch':True,'inputCount':len(paths),'qaSymbolsAbsent':True,
        'testAndPrototypeResourcesAbsent':True,'resourceInputs':resource_inputs,'normalLinkLine':link_line,
        'executable':str(exe),'executableSha256':sha(exe),'steamDllSha256':sha(exe.with_name('libcpp-steam-tools.dll')),
        'qtCoreSha256':sha(cache/'msys64/ucrt64/bin/Qt6Core.dll'),'qtQuickSha256':sha(cache/'msys64/ucrt64/bin/Qt6Quick.dll'),
        'sources':{name:sha(repo/name) for name in paths}}
(here/'normal-checks.json').write_text(json.dumps(record,indent=2))
print('Verified',len(paths),'production inputs,',len(resource_inputs),'generated resource inputs, normal link and binary')
