import json
import os
from pathlib import Path
import subprocess

evidence = Path(__file__).parent
stage = evidence.parent / 'ticket03-package'
env = os.environ.copy()
env.update({'PATH': str(stage) + r';C:\Windows\System32;C:\Windows',
            'QT_PLUGIN_PATH': str(stage), 'QT_QPA_PLATFORM': 'windows'})
for key in ('QT_OPENGL', 'QSG_RHI_BACKEND', 'QTWEBENGINE_CHROMIUM_FLAGS',
            'QTWEBENGINE_DISABLE_SANDBOX', 'LIBGL_ALWAYS_SOFTWARE'):
    env.pop(key, None)
results = []
for mode in ('compute-control', 'no-compute'):
    args = [str(stage / 'chiaki-libplacebo-mix-control.exe')]
    if mode == 'no-compute':
        args.append(mode)
    with (evidence / (mode + '.txt')).open('w', encoding='utf-8') as output:
        result = subprocess.run(args, cwd=stage, env=env, stdout=output,
                                stderr=subprocess.STDOUT, timeout=15)
    results.append({'mode': mode, 'exitCode': result.returncode, 'diagnosticOnly': True})
(evidence / 'results.json').write_text(json.dumps(results, indent=2), encoding='utf-8')
print(json.dumps(results))
