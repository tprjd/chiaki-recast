@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket14-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=offscreen
set QT_QUICK_BACKEND=software
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket14-build\gui\tests\chiaki-embedded-setup-tests.exe colorMappingSettings -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket14-red-result.txt,txt

