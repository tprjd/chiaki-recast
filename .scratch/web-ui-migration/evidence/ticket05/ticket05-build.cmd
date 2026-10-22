@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
cmake --build C:/Users/david/.cache/chiaki-msvc-native/ticket05-build --target chiaki chiaki-embedded-setup-tests -j 8

