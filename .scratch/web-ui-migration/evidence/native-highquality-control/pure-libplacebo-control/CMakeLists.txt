cmake_minimum_required(VERSION 3.21)
project(ChiakiPlaceboControl LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 6.9.3 REQUIRED COMPONENTS Core Gui)
set(DEPS "C:/Users/david/.cache/chiaki-msvc-native")
set(FFMPEG "C:/Users/david/.cache/chiaki-ui-native/ffmpeg/ffmpeg-n7.1-latest-win64-gpl-shared-7.1")
add_executable(chiaki-libplacebo-control main.cpp)
target_include_directories(chiaki-libplacebo-control PRIVATE "${DEPS}/libplacebo-prefix/include" "${FFMPEG}/include")
target_link_libraries(chiaki-libplacebo-control PRIVATE Qt6::Core Qt6::Gui
    "${DEPS}/libplacebo-prefix/lib/libplacebo.lib"
    "${DEPS}/ticket03-build/gui/placebo-libav-impl.lib"
    "${FFMPEG}/lib/avutil.lib" "${FFMPEG}/lib/avcodec.lib" "${FFMPEG}/lib/avformat.lib")
