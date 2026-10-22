@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket12-build\third-party\steam-tools;%PATH%"
set CHIAKI_MEDIA_PREVIEW=C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket12-settings.png
set CHIAKI_MEDIA_RESTART_LOG=C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket12-restart-child.txt
set CHIAKI_MEDIA_EVIDENCE=C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket12-layout
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket12-build\gui\tests\chiaki-embedded-setup-tests.exe videoAudioDevices videoAudioPreview -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket12-devices-layout.txt,txt







