@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket10-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=windows
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket10-build\gui\tests\chiaki-embedded-setup-tests.exe startupNativeConnection:psn-ps4 startupNativeConnection:psn-ps4-ambiguous -o C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket10-ps4-green\qt.txt,txt > C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket10-ps4-green\runtime.txt 2>&1



