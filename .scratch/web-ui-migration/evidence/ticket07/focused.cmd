@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket07-build\third-party\steam-tools;%PATH%"
set QT_QPA_PLATFORM=offscreen
set QT_QUICK_BACKEND=software
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
set QT_QPA_FONTDIR=C:\Windows\Fonts
set CHIAKI_EMBEDDED_EVIDENCE=C:/Users/david/.codex/worktrees/web-ui-ticket-07/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket07
C:\Users\david\.cache\chiaki-msvc-native\ticket07-build\gui\tests\chiaki-embedded-setup-tests.exe registrationForm registrationProgress registrationLookup registrationLookupTimeout registrationAccountSources registrationNativeValidation registrationStaleTargets registrationAddressCorrection psnSignIn consoleTargetGuards -o C:/Users/david/.codex/worktrees/web-ui-ticket-07/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket07/focused-result.txt,txt

