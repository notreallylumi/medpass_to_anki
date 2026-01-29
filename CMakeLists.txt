cmake_minimum_required(VERSION 3.11)
project(medpass-to-anki C)

set(CMAKE_C_STANDARD C11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)


find_package(PkgConfig REQUIRED)

pkg_check_modules(TESSERACT REQUIRED IMPORTED_TARGET tesseract)
pkg_check_modules(LEPTONICA REQUIRED IMPORTED_TARGET lept)
pkg_check_modules(CURL      REQUIRED IMPORTED_TARGET libcurl)

add_executable(medpass-to-anki
    src/main.c
)

target_include_directories(medpass-to-anki PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

target_link_libraries(medpass-to-anki PRIVATE
    raylib
    PkgConfig::TESSERACT
    PkgConfig::LEPTONICA
    PkgConfig::CURL
)

target_link_libraries(medpass-to-anki PRIVATE m pthread dl rt X11 GL)

set_target_properties(medpass-to-anki PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
