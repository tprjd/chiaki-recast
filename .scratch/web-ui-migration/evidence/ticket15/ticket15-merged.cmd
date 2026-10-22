@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket15-build\third-party\steam-tools;%PATH%"
set CHIAKI_KEYBOARD_MOUSE_EVIDENCE=C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket15-layout
set QT_QPA_FONTDIR=C:/Windows/Fonts
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket15-build\gui\tests\chiaki-embedded-setup-tests.exe keyboardMouseSettings keyboardMouseLegacyAndLifetime rendererSettings -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket15-merged.txt,txt



