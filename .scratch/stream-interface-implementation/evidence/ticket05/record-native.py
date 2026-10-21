"""Retain exact native source, build identity, settings check, and app-only images."""
from pathlib import Path
import hashlib, json, shutil, subprocess, sys
here=Path(__file__).resolve().parent;repo=here.parents[3]
cache=Path('C:/Users/david/.cache/chiaki-ui-native');run=cache/sys.argv[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
result=json.loads((run/'contracts.json').read_text());assert result['allPassed']
label=sys.argv[2];dest=here/'captures'/label;dest.mkdir(parents=True,exist_ok=False)
for file in run.glob('*.png'):
    assert file.name!='desktop-full.png'
    shutil.copyfile(file,dest/file.name)
shutil.copyfile(run/'contracts.json',here/(label+'.json'))
subprocess.run([sys.executable,str(repo/'.scratch/psn-dialog-refresh/evidence/ticket06/check-post-exit.py'),str(run)],check=True,stdout=(here/(label+'-settings.json')).open('w'))
paths=subprocess.check_output(['git','ls-files','CMakeLists.txt','cmake','third-party','gui','lib','include','cli'],cwd=repo,text=True).splitlines()
paths=[name for name in paths if (repo/name).is_file()]
metadata={'sourceRevision':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
          'run':str(run),'runtimeRendererBackend':result['runtimeRendererBackend'],
          'executableSha256':sha(cache/'stream-acceptance-qa-bin/chiaki.exe'),
          'qtCoreSha256':sha(cache/'msys64/ucrt64/bin/Qt6Core.dll'),
          'nativeHelperSha256':sha(cache/sys.argv[3]/'native-disconnect.cpp'),
          'productionSources':{name:sha(repo/name) for name in paths},
          'stagedSources':{name:sha(cache/'source'/name) for name in paths},
          'captureHashes':{p.name:sha(p) for p in dest.glob('*.png')}}
(here/(label+'-source.json')).write_text(json.dumps(metadata,indent=2))
print(label,result['checks'],'checks; actual DPR',result.get('dpr','baseline'))
