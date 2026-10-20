cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(chiaki-console-screen-tests LANGUAGES CXX)
    enable_testing()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
endif()

find_package(Qt6 REQUIRED COMPONENTS QuickTest Quick Svg)

add_executable(chiaki-console-screen-tests
    main.cpp
    ../src/qmlsvgprovider.cpp
    ../res/resources.qrc)
target_include_directories(chiaki-console-screen-tests PRIVATE ../include)
target_link_libraries(chiaki-console-screen-tests PRIVATE Qt6::QuickTest Qt6::Quick Qt6::Svg)
target_compile_definitions(chiaki-console-screen-tests PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")

add_test(NAME chiaki-console-screen COMMAND chiaki-console-screen-tests)
set_tests_properties(chiaki-console-screen PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
