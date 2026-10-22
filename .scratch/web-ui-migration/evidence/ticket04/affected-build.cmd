@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build C:/Users/david/.cache/chiaki-msvc-native/ticket04-build --target chiaki chiaki-embedded-setup-tests chiaki-native-services-tests -j 6


