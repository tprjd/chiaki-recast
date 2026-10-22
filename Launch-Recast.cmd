@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\launch-local-windows.ps1"
if errorlevel 1 (
    echo Recast could not start. See the error above.
    pause
    exit /b 1
)
