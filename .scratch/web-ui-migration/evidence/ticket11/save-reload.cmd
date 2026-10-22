@echo off
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket11-build\third-party\steam-tools;%PATH%"
set "CHIAKI_EMBEDDED_PREVIEW=C:/Users/david/.codex/worktrees/web-ui-ticket-11/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/library-1280.png"
set "CHIAKI_EMBEDDED_SCREENSHOT=C:/Users/david/.codex/worktrees/web-ui-ticket-11/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket03/library-390.png"
C:\Users\david\.cache\chiaki-msvc-native\ticket11-build\gui\tests\chiaki-embedded-setup-tests.exe generalSettings -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket11-basic-green.txt,txt
