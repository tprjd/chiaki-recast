cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(chiaki-console-screen-tests LANGUAGES CXX)
    enable_testing()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
endif()

find_package(Qt6 REQUIRED COMPONENTS QuickTest Quick Svg)
find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(chiaki-stream-input-tests streaminput.cpp
    ../include/streaminputownership.h ../src/streaminputownership.cpp)
target_include_directories(chiaki-stream-input-tests PRIVATE ../include)
target_link_libraries(chiaki-stream-input-tests PRIVATE Qt6::Test Qt6::Quick)
add_test(NAME chiaki-stream-input COMMAND chiaki-stream-input-tests)
set_tests_properties(chiaki-stream-input PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")

add_executable(chiaki-console-screen-tests
    main.cpp
    ../include/streaminputownership.h ../src/streaminputownership.cpp
    ../src/qmlsvgprovider.cpp
    ../res/resources.qrc)
target_include_directories(chiaki-console-screen-tests PRIVATE ../include)
target_link_libraries(chiaki-console-screen-tests PRIVATE Qt6::QuickTest Qt6::Quick Qt6::Svg)
target_compile_definitions(chiaki-console-screen-tests PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")

add_test(NAME chiaki-console-screen COMMAND chiaki-console-screen-tests)
set(screen_test_environment
    "QT_QPA_PLATFORM=offscreen"
    "QT_QUICK_BACKEND=software"
    "QT_QUICK_CONTROLS_CONF=${CMAKE_CURRENT_SOURCE_DIR}/../src/qml/qtquickcontrols2.conf")
if(WIN32)
    list(APPEND screen_test_environment "QT_QPA_FONTDIR=$ENV{WINDIR}/Fonts")
endif()
set_tests_properties(chiaki-console-screen PROPERTIES
    ENVIRONMENT "${screen_test_environment}")

if(TARGET chiaki-gui)
    add_executable(chiaki-embedded-setup-tests embeddedsetup.cpp)
    target_link_libraries(chiaki-embedded-setup-tests PRIVATE chiaki-gui Qt6::Test)
    target_compile_definitions(chiaki-embedded-setup-tests PRIVATE CHIAKI_TEST_PYTHON="${PYTHON_EXECUTABLE}")
    add_test(NAME chiaki-embedded-setup COMMAND chiaki-embedded-setup-tests consoleLibrary)
    set_tests_properties(chiaki-embedded-setup PROPERTIES LABELS desktop)
    add_test(NAME chiaki-embedded-psn COMMAND chiaki-embedded-setup-tests psnAccountStatus psnSignIn psnRefreshCredentials psnEmbeddedIsolation psnAccountPresentation)
    set_tests_properties(chiaki-embedded-psn PROPERTIES LABELS desktop)
    add_test(NAME chiaki-embedded-settings COMMAND chiaki-embedded-setup-tests generalSettings settingsValidation settingsRestart)
    set_tests_properties(chiaki-embedded-settings PROPERTIES LABELS desktop)
    add_test(NAME chiaki-embedded-video-audio COMMAND chiaki-embedded-setup-tests videoAudioSettings videoAudioState videoAudioRestart videoAudioDevices)
    set_tests_properties(chiaki-embedded-video-audio PROPERTIES LABELS desktop)
    add_test(NAME chiaki-embedded-graphics COMMAND chiaki-embedded-setup-tests graphicsCoexistence)
    set_tests_properties(chiaki-embedded-graphics PROPERTIES LABELS "desktop;gpu")

    add_executable(chiaki-native-services-tests nativeservices.cpp)
    target_link_libraries(chiaki-native-services-tests PRIVATE chiaki-gui Qt6::Test)
    add_test(NAME chiaki-native-services COMMAND chiaki-native-services-tests)
    set_tests_properties(chiaki-native-services PROPERTIES LABELS desktop)
endif()

if(TARGET chiaki-gui AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(chiaki-mouse-response-tests mouseresponse.cpp)
    target_link_libraries(chiaki-mouse-response-tests PRIVATE chiaki-gui Qt6::Test)
    add_test(NAME chiaki-mouse-response COMMAND chiaki-mouse-response-tests)
    add_executable(chiaki-stream-input-native-tests streaminputnative.cpp)
    target_link_libraries(chiaki-stream-input-native-tests PRIVATE chiaki-gui Qt6::Test)
    target_link_options(chiaki-stream-input-native-tests PRIVATE "-Wl,--wrap=chiaki_session_set_controller_state")
    add_test(NAME chiaki-gameplay-mouse-response COMMAND chiaki-stream-input-native-tests mouseResponseRecapture)
    set_tests_properties(chiaki-gameplay-mouse-response PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_test(NAME chiaki-keyboard-mouse-settings-desktop COMMAND chiaki-stream-input-native-tests keyboardMouseSettingsRoute)
    set_tests_properties(chiaki-keyboard-mouse-settings-desktop PROPERTIES LABELS desktop)
    add_test(NAME chiaki-keyboard-mouse-settings-vulkan COMMAND chiaki-stream-input-native-tests keyboardMouseSettingsRoute)
    set_tests_properties(chiaki-keyboard-mouse-settings-vulkan PROPERTIES LABELS desktop ENVIRONMENT "CHIAKI_INPUT_TEST_RENDERER=vulkan")
    add_test(NAME chiaki-keyboard-mouse-reset COMMAND chiaki-stream-input-native-tests resetKeyboardMouseDefaults)
    set_tests_properties(chiaki-keyboard-mouse-reset PROPERTIES LABELS desktop)
    add_test(NAME chiaki-stream-input-desktop COMMAND chiaki-stream-input-native-tests sessionAndFeedback)
    set_tests_properties(chiaki-stream-input-desktop PROPERTIES LABELS desktop)
    add_test(NAME chiaki-stream-input-vulkan COMMAND chiaki-stream-input-native-tests sessionAndFeedback)
    set_tests_properties(chiaki-stream-input-vulkan PROPERTIES LABELS desktop
        ENVIRONMENT "CHIAKI_INPUT_TEST_RENDERER=vulkan")
    add_test(NAME chiaki-gameplay-keyboard-output COMMAND chiaki-stream-input-native-tests keyboardCapture)
    set_tests_properties(chiaki-gameplay-keyboard-output PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_test(NAME chiaki-gameplay-keyboard-desktop COMMAND chiaki-stream-input-native-tests platformKeyboard)
    set_tests_properties(chiaki-gameplay-keyboard-desktop PROPERTIES LABELS desktop)
    add_test(NAME chiaki-gameplay-mouse-motion COMMAND chiaki-stream-input-native-tests mouseMotion)
    add_test(NAME chiaki-gameplay-mouse-triggers COMMAND chiaki-stream-input-native-tests mouseTriggers)
    set_tests_properties(chiaki-gameplay-mouse-motion chiaki-gameplay-mouse-triggers PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_test(NAME chiaki-gameplay-bindings COMMAND chiaki-stream-input-native-tests editableBindings bindingValidation)
    set_tests_properties(chiaki-gameplay-bindings PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_test(NAME chiaki-gameplay-binding-editor COMMAND chiaki-stream-input-native-tests bindingEditor)
    set_tests_properties(chiaki-gameplay-binding-editor PROPERTIES LABELS desktop)
    add_test(NAME chiaki-gameplay-mouse-desktop COMMAND chiaki-stream-input-native-tests mouseDesktop)
    set_tests_properties(chiaki-gameplay-mouse-desktop PROPERTIES LABELS desktop)
endif()
