"""Check the original sources/options/output and the separate final normal build."""
from pathlib import Path
import hashlib, json, sys
here=Path(__file__).resolve().parent
cache=Path('C:/Users/david/.cache/chiaki-ui-native')
prep=cache/sys.argv[1]
sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None
original=json.loads((prep/'original-source.json').read_text())
for name,expected in original.items():
    assert sha(cache/'source'/name)==expected,name
def options(path):
    return {line.split(':',1)[0]:line.split('=',1)[1] for line in path.read_text().splitlines() if '=' in line and not line.startswith(('#','//'))}
before=options(prep/'CMakeCache.txt');after=options(cache/'build/CMakeCache.txt')
checked=[name for name in before if name.startswith('CHIAKI_') or name in ['CMAKE_BUILD_TYPE','CMAKE_CXX_COMPILER','CMAKE_MAKE_PROGRAM','Qt6_DIR','CMAKE_RUNTIME_OUTPUT_DIRECTORY']]
for name in checked:
    assert before[name]==after[name],name
for name in ['chiaki.exe','chiaki-console-screen-tests.exe']:
    assert (Path(after['CMAKE_RUNTIME_OUTPUT_DIRECTORY'])/name).is_file(),name
normal=json.loads((here/'normal-checks.json').read_text())
exe=Path(normal['executable'])
assert sha(exe)==normal['executableSha256']
assert sha(exe.with_name('libcpp-steam-tools.dll'))==normal['steamDllSha256']
record={'restoredPaths':len(original),'allOriginalSourceHashesMatch':True,'checkedOptions':len(checked),'allOriginalOptionsMatch':True,'originalOutput':after['CMAKE_RUNTIME_OUTPUT_DIRECTORY'],'originalTargetsRebuilt':True,'normalOutputPreserved':True}
(here/'cache-restoration.json').write_text(json.dumps(record,indent=2))
print(json.dumps(record))
