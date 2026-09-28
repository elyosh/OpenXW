# Configure before creating targets so dependencies share the modern compiler cache.
option(XW_ENABLE_COMPILER_CACHE "Cache modern C/C++ compilation with ccache" OFF)
set(XW_COMPILER_CACHE_DIR "" CACHE PATH "Shared compiler cache directory (defaults to per-user temporary storage)")

if(NOT XW_ENABLE_COMPILER_CACHE)
    return()
endif()

find_program(XW_CCACHE_EXECUTABLE NAMES ccache REQUIRED)

foreach(language IN ITEMS C CXX)
    if(CMAKE_${language}_COMPILER_ID MATCHES "^(AppleClang|Clang)$"
            AND NOT CMAKE_${language}_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options("$<$<COMPILE_LANGUAGE:${language}>:-fdebug-compilation-dir=.>")
    elseif(CMAKE_${language}_COMPILER_ID STREQUAL "GNU")
        add_compile_options("$<$<COMPILE_LANGUAGE:${language}>:-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.>")
    else()
        message(FATAL_ERROR "XW_ENABLE_COMPILER_CACHE requires a GNU-compatible Clang or GCC ${language} compiler")
    endif()
endforeach()

if(NOT XW_COMPILER_CACHE_DIR)
    if(NOT UNIX)
        message(FATAL_ERROR "Set XW_COMPILER_CACHE_DIR to a private shared cache directory on this platform")
    endif()
    execute_process(COMMAND id -u
        OUTPUT_VARIABLE xw_cache_user_id
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    if(DEFINED ENV{TMPDIR} AND NOT "$ENV{TMPDIR}" STREQUAL "")
        set(xw_cache_temp_dir "$ENV{TMPDIR}")
    else()
        set(xw_cache_temp_dir "/tmp")
    endif()
    set(XW_COMPILER_CACHE_DIR "${xw_cache_temp_dir}/openxw-ccache-${xw_cache_user_id}"
        CACHE PATH "Shared compiler cache directory (defaults to per-user temporary storage)" FORCE)
    file(MAKE_DIRECTORY "${XW_COMPILER_CACHE_DIR}")
    file(CHMOD "${XW_COMPILER_CACHE_DIR}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
endif()

if(NOT IS_ABSOLUTE "${XW_COMPILER_CACHE_DIR}")
    message(FATAL_ERROR "XW_COMPILER_CACHE_DIR must be an absolute path")
endif()

# Store settings in the launcher so direct builds and every worker phase agree.
# Checking --version identifies the selected toolchain behind /usr/bin/clang on macOS.
set(xw_compiler_cache_launcher
    "${XW_CCACHE_EXECUTABLE}"
    "cache_dir=${XW_COMPILER_CACHE_DIR}"
    "base_dir=${CMAKE_SOURCE_DIR}"
    "compiler_check=%compiler% --version")
set(CMAKE_C_COMPILER_LAUNCHER ${xw_compiler_cache_launcher})
set(CMAKE_CXX_COMPILER_LAUNCHER ${xw_compiler_cache_launcher})
message(STATUS "OpenXW compiler cache: ${XW_COMPILER_CACHE_DIR}")
