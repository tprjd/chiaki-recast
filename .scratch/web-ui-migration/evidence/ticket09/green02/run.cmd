@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket09-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=windows
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket09-build\gui\tests\chiaki-embedded-setup-tests.exe automaticRegistration -o C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket09-green02\qt.txt,txt > C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket09-green02\runtime.txt 2>&1



