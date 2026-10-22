@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
cmake --build C:/Users/david/.cache/chiaki-msvc-native/ticket15-build --target chiaki-embedded-setup-tests --parallel 8
