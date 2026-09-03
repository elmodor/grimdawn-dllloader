cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_HOST_WIN32)
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
endif()

project(gd_dllloader)
enable_language(CXX)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(DEF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/winmm.def")
set(EXPORT_NAMES "${CMAKE_CURRENT_BINARY_DIR}/export_names.inc")
set(EXPORTS_ASM "${CMAKE_CURRENT_BINARY_DIR}/exports.S")
set(GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/generate_exports.py")
add_custom_command(
    OUTPUT
        "${EXPORT_NAMES}"
        "${EXPORTS_ASM}"
    COMMAND
        "${Python3_EXECUTABLE}"
        "${GENERATOR}"
        "${DEF_FILE}"
        "${EXPORT_NAMES}"
        "${EXPORTS_ASM}"
    DEPENDS
        "${DEF_FILE}"
        "${GENERATOR}"
    COMMENT "Generating export_names.inc and exports.S"
)
add_custom_target(generate_exports
    DEPENDS
        "${EXPORT_NAMES}"
        "${EXPORTS_ASM}"
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NOLOG NDEBUG WIN32_LEAN_AND_MEAN)
    add_compile_options(
        -O3
        -g0
        -DNDEBUG
        -ffunction-sections
        -fdata-sections
        -fno-ident
        -flto
    )
    add_link_options(
        -s
        -flto
        -Wl,--gc-sections
        -Wl,--strip-all
    )
endif()
if(NOLOG)
    add_compile_definitions(NOLOG)
endif()

enable_language(ASM)
add_library(winmm SHARED
    proxy.cpp
    "${EXPORTS_ASM}"
)
add_library(gd_dllloader::winmm ALIAS winmm)
add_dependencies(winmm generate_exports)
target_include_directories(winmm PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
)
set_target_properties(winmm PROPERTIES
    PREFIX ""
    OUTPUT_NAME "winmm"
)
