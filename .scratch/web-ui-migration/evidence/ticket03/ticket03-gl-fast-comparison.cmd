@echo off
set CHIAKI_GL_DIAGNOSTIC=1
set CHIAKI_GL_FAST_COMPARISON=1
set "PATH=C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package;C:\Windows\System32;C:\Windows"
set "QTDIR="
set "QT_PLUGIN_PATH=C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package"
set "QML_IMPORT_PATH=C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package\qml"
set "QML2_IMPORT_PATH="
set "QTWEBENGINEPROCESS_PATH=C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package\QtWebEngineProcess.exe"
set "QTWEBENGINE_RESOURCES_PATH=C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package\resources"
set "QTWEBENGINE_LOCALES_PATH=C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package\translations\qtwebengine_locales"
set "QTWEBENGINE_CHROMIUM_FLAGS="
set "QTWEBENGINE_DISABLE_SANDBOX="
set "CHIAKI_EMBEDDED_EVIDENCE=C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03"
set "CHIAKI_NATIVE_GPU_ONLY="
set "CHIAKI_NATIVE_MATCHED_API="
set "CHIAKI_EMBEDDED_PREVIEW=C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/package-library-1280.png"
set "CHIAKI_EMBEDDED_SCREENSHOT=C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/package-library-390.png"
cd /d C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket03-package
where node.exe > C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/package-environment.txt 2>&1
where qmake.exe >> C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/package-environment.txt 2>&1
chiaki-embedded-setup-tests.exe graphicsCoexistence -o C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/gl-fast-comparison.txt,txt

echo %ERRORLEVEL% > C:/Users/david/.codex/worktrees/web-ui-ticket-03/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/gl-fast-comparison-exit.txt



