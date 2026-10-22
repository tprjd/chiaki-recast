@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket10-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket10-build\gui\tests\chiaki-embedded-setup-tests.exe autoConnectCancellation -o C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket10-cancel-green\qt.txt,txt > C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket10-cancel-green\runtime.txt 2>&1



