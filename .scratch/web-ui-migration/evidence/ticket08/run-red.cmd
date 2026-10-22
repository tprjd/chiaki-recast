@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket08-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket08-build\gui\tests\chiaki-embedded-setup-tests.exe psnRemoteRefresh -o C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket08\red.txt,txt
exit /b %ERRORLEVEL%
