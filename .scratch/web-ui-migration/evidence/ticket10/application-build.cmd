@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build C:/Users/david/.cache/chiaki-msvc-native/ticket10-build --target chiaki -j 4 > C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket10-application-build.txt 2>&1

