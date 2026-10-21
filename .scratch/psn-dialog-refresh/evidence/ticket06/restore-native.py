from pathlib import Path
import hashlib
import json
import os
import shutil
import sys

root = Path(r'C:/Users/david/.cache/chiaki-ui-native')
source = root / 'source'
qa = root / (sys.argv[1] if len(sys.argv) > 1 else 'disconnect-prep01')
manifest = json.loads((qa / 'manifest.json').read_text())
for name, expected in manifest.items():
    target = source / name
    assert target.resolve().is_relative_to(source.resolve()), name
    if expected is None:
        if target.exists():
            target.unlink()
    else:
        shutil.copyfile(qa / 'backup' / name, target)
        os.utime(target, None)
    actual = hashlib.sha256(target.read_bytes()).hexdigest() if target.exists() else None
    assert actual == expected, name
output = next(line.split('=', 1)[1] for line in (qa / 'CMakeCache.txt').read_text().splitlines()
              if line.startswith('CMAKE_RUNTIME_OUTPUT_DIRECTORY:'))
record = {'restoredPaths': len(manifest), 'allOriginalHashesMatch': True, 'originalOutputDirectory': output}
(qa / 'restored-source.json').write_text(json.dumps(record, indent=2))
print(json.dumps(record))
