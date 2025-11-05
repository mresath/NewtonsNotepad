cmake_minimum_required(VERSION 3.10)

project(NewtonsNotepad)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(SFML_DIR "${CMAKE_SOURCE_DIR}/vendor/SFML/lib/cmake/SFML")
elseif(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml")
endif()
find_package(SFML 3.0.2 COMPONENTS Graphics Window System REQUIRED)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")
set(IMGUI_SFML_FIND_SFML OFF)
add_subdirectory(vendor/imgui-sfml)

add_subdirectory(vendor/fmt)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/core/World.cpp
    src/core/Tools.cpp
    src/objects/Object.cpp
    src/engine/ODE.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System ImGui-SFML::ImGui-SFML fmt::fmt)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/vendor/SFML/bin"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying SFML DLLs to build directory..."
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy_assets.cmake
)
add_dependencies(${PROJECT_NAME} copy_assets)