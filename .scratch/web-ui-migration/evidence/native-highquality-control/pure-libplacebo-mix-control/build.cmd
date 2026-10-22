@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "CMAKE=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
"%CMAKE%" -S C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/pure-libplacebo-mix-control -B C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/pure-libplacebo-mix-control/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER=lld-link -DCMAKE_PREFIX_PATH=C:/Users/david/.cache/chiaki-native-qt/6.9.3/msvc2022_64
if errorlevel 1 exit /b %ERRORLEVEL%
"%CMAKE%" --build C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/pure-libplacebo-mix-control/build
