@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket08-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=windows
set QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu
set QT_QUICK_BACKEND=
set QT_QPA_FONTDIR=C:\Windows\Fonts
python -c "import subprocess; s=subprocess.STARTUPINFO(); s.dwFlags=subprocess.STARTF_USESHOWWINDOW; s.wShowWindow=1; r=subprocess.run([r'C:\Users\david\.cache\chiaki-msvc-native\ticket08-build\gui\tests\chiaki-embedded-setup-tests.exe','remoteNativeConnection','-o',r'C:\Users\david\AppData\Local\Temp\chiaki-web-ui-migration\ticket08\native.txt,txt'],startupinfo=s); raise SystemExit(r.returncode)"
exit /b %ERRORLEVEL%
