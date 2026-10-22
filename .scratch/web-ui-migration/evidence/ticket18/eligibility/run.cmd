@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket18-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=offscreen
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
C:\Users\david\.cache\chiaki-msvc-native\ticket18-build\gui\tests\chiaki-embedded-setup-tests.exe setupReminderEligibility wakeConsole -o C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket18-eligibility\qt.txt,txt > C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket18-eligibility\runtime.txt 2>&1
