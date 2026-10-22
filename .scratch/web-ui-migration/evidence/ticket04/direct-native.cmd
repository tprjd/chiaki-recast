@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket04-build\third-party\steam-tools;%PATH%"
set "QT_QPA_PLATFORM=windows"
set "QTWEBENGINE_CHROMIUM_FLAGS="
C:\Users\david\.cache\chiaki-msvc-native\ticket04-build\gui\tests\chiaki-native-services-tests.exe windowRecreation -o C:/Users/david/.codex/worktrees/web-ui-ticket-04/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket04/green-direct-native.txt,txt
