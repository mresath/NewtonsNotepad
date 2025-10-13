cmake_minimum_required(VERSION 3.10)

project(NewtonsNotepad)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_DIR "${CMAKE_SOURCE_DIR}/vendor/SFML/lib/cmake/SFML")

find_package(SFML 2.6.1 COMPONENTS graphics window system REQUIRED)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/objects/Body.cpp
    src/core/World.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/vendor/SFML/bin"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying SFML DLLs to build directory..."
)