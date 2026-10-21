"""Check the normal production target and the files that CMake consumed."""
from pathlib import Path
import hashlib,json,subprocess
cache=Path('C:/Users/david/.cache/chiaki-ui-native');repo=Path(__file__).resolve().parents[4];here=Path(__file__).resolve().parent
sha=lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
paths=subprocess.check_output(['git','ls-files','CMakeLists.txt','cmake','third-party','gui','lib','include','cli'],cwd=repo,text=True).splitlines()
for name in paths:
 if (repo/name).is_file(): assert sha(repo/name)==sha(cache/'source'/name),name
exe=cache/'stream-toolbar-normal-bin/chiaki.exe';blob=exe.read_bytes()
for marker in [b'initializeDisconnectQa',b'nativeQaImage',b'CHIAKI_DISCONNECT_QA_ROOT',b'StreamHarness.qml',b'tst_StreamToolbar.qml']:
 assert marker not in blob,marker
resource=(cache/'source/gui/res/resources.qrc').read_text()
for marker in ['prototype','fixtures','tst_Stream','StreamHarness']:
 assert marker not in resource,marker
record={'allProductionInputsMatched':True,'comparedInputs':sum((repo/n).is_file() for n in paths),'qaSymbolsAbsent':True,'testAndPrototypeResourcesAbsent':True,'revision':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'executableSha256':sha(exe),'qtCoreSha256':sha(cache/'msys64/ucrt64/bin/Qt6Core.dll'),'qtQuickSha256':sha(cache/'msys64/ucrt64/bin/Qt6Quick.dll'),'sources':{n:sha(repo/n) for n in paths if (repo/n).is_file()}}
(here/'normal-checks.json').write_text(json.dumps(record,indent=2));print('All',record['comparedInputs'],'production inputs match; QA hooks and test/prototype resources are absent')
