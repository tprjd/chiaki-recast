@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket16-build\third-party\steam-tools;%PATH%
set CHIAKI_EMBEDDED_EVIDENCE=C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket16-green02
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket16-build\gui\tests\chiaki-embedded-setup-tests.exe controllerSettings controllerMapping -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket16-green02/qt.txt,txt



