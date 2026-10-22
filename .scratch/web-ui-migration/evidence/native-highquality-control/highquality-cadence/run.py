import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

evidence = Path(__file__).parent
stage = evidence.parent / 'ticket03-package'
source = Path(r'C:\Users\david\.cache\chiaki-msvc-native\ticket03-build\gui\tests\chiaki-embedded-setup-tests.exe')
exe = stage / 'chiaki-highquality-cadence-probe.exe'
shutil.copy2(source, exe)
env = os.environ.copy()
env.update({
    'PATH': str(stage) + r';C:\Windows\System32;C:\Windows',
    'QT_PLUGIN_PATH': str(stage),
    'QML_IMPORT_PATH': str(stage / 'qml'),
    'QTWEBENGINEPROCESS_PATH': str(stage / 'QtWebEngineProcess.exe'),
    'QTWEBENGINE_RESOURCES_PATH': str(stage / 'resources'),
    'QTWEBENGINE_LOCALES_PATH': str(stage / 'translations' / 'qtwebengine_locales'),
    'CHIAKI_GL_DIAGNOSTIC': '1',
    'QT_QPA_PLATFORM': 'windows',
})
for key in ('QTWEBENGINE_CHROMIUM_FLAGS', 'QTWEBENGINE_DISABLE_SANDBOX',
            'CHIAKI_FORCE_VULKAN_FALLBACK', 'QML2_IMPORT_PATH', 'QSG_RHI_BACKEND',
            'CHIAKI_GL_NO_COMPUTE_DIAGNOSTIC'):
    env.pop(key, None)
with (evidence / 'console.txt').open('w', encoding='utf-8') as console:
    result = subprocess.run(
        [str(exe), 'highQualityBufferDiagnostic', '-o', str(evidence / 'native.txt') + ',txt'],
        cwd=stage, env=env, stdout=console, stderr=subprocess.STDOUT, timeout=30,
    )
record = {'exitCode': result.returncode, 'diagnosticOnly': True,
          'sha256': hashlib.sha256(exe.read_bytes()).hexdigest()}
(evidence / 'result.json').write_text(json.dumps(record, indent=2), encoding='utf-8')
print(json.dumps(record))
