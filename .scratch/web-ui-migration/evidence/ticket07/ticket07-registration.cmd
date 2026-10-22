@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket07-build\third-party\steam-tools;%PATH%"
set QT_QPA_PLATFORM=offscreen
set QT_QUICK_BACKEND=software
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket07-build\gui\tests\chiaki-embedded-setup-tests.exe registrationForm registrationProgress registrationLookup -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket07-registration-result.txt,txt





