from pathlib import Path
import subprocess,hashlib,json,gzip,sys
root=Path.cwd(); out=root/'.scratch/web-ui-migration/evidence/ticket17'/sys.argv[1];out.mkdir(parents=True,exist_ok=True)
def git(*args): return subprocess.check_output(['git',*args],cwd=root)
base='360c2903'
paths=git('diff','--name-only',base,'--','gui').decode().splitlines()
paths += git('ls-files','--others','--exclude-standard','--','gui').decode().splitlines()
sources={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for directory in ['gui','lib','cmake','scripts'] for p in (root/directory).rglob('*') if p.is_file() and not any(x in p.parts for x in ['node_modules','.git'])}
artifacts={}
build=Path('C:/Users/david/.cache/chiaki-msvc-native/ticket17-build')
for name in ['gui/chiaki.exe','gui/tests/chiaki-embedded-setup-tests.exe','gui/tests/chiaki-console-screen-tests.exe','gui/web/app.js','gui/web/app.css']:
 p=build/name;artifacts[name]=hashlib.sha256(p.read_bytes()).hexdigest()
record=dict(commit=git('rev-parse','HEAD').decode().strip(),sourceHashes=sources,sourceDiff=git('diff','--','gui').decode(),fullChangedSources={p:(root/p).read_text() for p in paths if (root/p).is_file()},artifacts=artifacts,base=base)
(out/'identity.json.gz').write_bytes(gzip.compress(json.dumps(record,indent=2).encode(),mtime=0))
(out/'source.patch').write_bytes(git('diff','--','gui'))
print(json.dumps(artifacts,indent=2))
