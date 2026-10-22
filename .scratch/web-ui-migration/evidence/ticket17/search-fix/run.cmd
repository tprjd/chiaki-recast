@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket17-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=offscreen
set QT_QUICK_BACKEND=software
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket17-build\gui\tests\chiaki-embedded-setup-tests.exe profileSearch -o C:\Users\david\.codex\worktrees\web-ui-ticket-17\chiaki-ng-ui\.scratch\web-ui-migration\evidence\ticket17\search-fix\qt.txt,txt > C:\Users\david\.codex\worktrees\web-ui-ticket-17\chiaki-ng-ui\.scratch\web-ui-migration\evidence\ticket17\search-fix\runtime.txt 2>&1
