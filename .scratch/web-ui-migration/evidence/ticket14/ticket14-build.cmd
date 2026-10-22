@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
cmake --build C:/Users/david/.cache/chiaki-msvc-native/ticket14-build --target chiaki-embedded-setup-tests -j 6
