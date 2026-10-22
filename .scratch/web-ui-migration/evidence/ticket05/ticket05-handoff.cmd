@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket05-build\third-party\steam-tools;%PATH%"
set QT_QPA_PLATFORM=
set QTWEBENGINE_CHROMIUM_FLAGS=
set CHIAKI_HANDOFF_REQUIRE_FOREGROUND=
C:\Users\david\.cache\chiaki-msvc-native\ticket05-build\gui\tests\chiaki-embedded-setup-tests.exe manualHandoff -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket05-handoff.txt,txt
