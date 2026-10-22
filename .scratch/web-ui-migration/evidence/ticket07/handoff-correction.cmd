@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket07-build\third-party\steam-tools;%PATH%"
set QT_QPA_PLATFORM=
set QT_QUICK_BACKEND=
set QTWEBENGINE_CHROMIUM_FLAGS=
set CHIAKI_HANDOFF_REQUIRE_FOREGROUND=
C:\Users\david\.cache\chiaki-msvc-native\ticket07-build\gui\tests\chiaki-embedded-setup-tests.exe registrationHandoff -o C:/Users/david/.codex/worktrees/web-ui-ticket-07/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket07/handoff-correction-result.txt,txt

