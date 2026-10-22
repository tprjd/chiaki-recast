@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
cmake --build C:\Users\david\.cache\chiaki-msvc-native\ticket18-build --target chiaki-embedded-setup-tests chiaki -j 4
