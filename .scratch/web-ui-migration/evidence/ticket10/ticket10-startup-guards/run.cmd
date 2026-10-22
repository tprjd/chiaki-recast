@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket10-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=windows
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket10-build\gui\tests\chiaki-embedded-setup-tests.exe startupNativeConnection:local-ready startupNativeConnection:local-wake startupNativeConnection:psn-ready startupNativeConnection:psn-ps4 startupNativeConnection:psn-removed startupNativeConnection:psn-replaced -o C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket10-startup-guards\qt.txt,txt > C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket10-startup-guards\runtime.txt 2>&1



