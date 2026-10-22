@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket04-build\third-party\steam-tools;%PATH%"
set "QT_QPA_PLATFORM=offscreen"
set "QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu"
set "CHIAKI_EMBEDDED_EVIDENCE=C:/Users/david/.codex/worktrees/web-ui-ticket-04/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket04"
C:\Users\david\.cache\chiaki-msvc-native\ticket04-build\gui\tests\chiaki-embedded-setup-tests.exe suspendClosesSignIn -o C:/Users/david/.codex/worktrees/web-ui-ticket-04/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket04/green-suspend-sign-in.txt,txt






