# Keep the pinned dependency unchanged. Apply the compatibility fixes to a build copy.
set(steam_tools_source "${CMAKE_CURRENT_LIST_DIR}/../third-party/cpp-steam-tools")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/steam-tools.patch")
set(steam_tools_patched "${CMAKE_CURRENT_BINARY_DIR}/cpp-steam-tools-patched")
file(COPY "${steam_tools_source}/" DESTINATION "${steam_tools_patched}" PATTERN ".git" EXCLUDE)
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" apply --ignore-space-change "${CMAKE_CURRENT_LIST_DIR}/steam-tools.patch"
    WORKING_DIRECTORY "${steam_tools_patched}" RESULT_VARIABLE steam_tools_patch_result ERROR_VARIABLE steam_tools_patch_error)
if(NOT steam_tools_patch_result EQUAL 0)
    message(FATAL_ERROR "Cannot patch pinned cpp-steam-tools: ${steam_tools_patch_error}")
endif()
add_subdirectory("${steam_tools_patched}" "${CMAKE_CURRENT_BINARY_DIR}/steam-tools")
