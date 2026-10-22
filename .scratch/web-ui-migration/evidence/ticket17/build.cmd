@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
cmake --build C:/Users/david/.cache/chiaki-msvc-native/ticket17-build --target chiaki-embedded-setup-tests chiaki-console-screen-tests chiaki -j 8
