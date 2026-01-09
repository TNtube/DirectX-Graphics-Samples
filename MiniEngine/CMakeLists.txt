cmake_minimum_required(VERSION 3.21)

# Project definition - Windows x64 only
project(MiniEngine
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "MiniEngine DirectX 12 Graphics Engine"
)

# Enforce x64 only
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "MiniEngine requires 64-bit builds only")
endif()

# Require Windows
if(NOT WIN32)
    message(FATAL_ERROR "MiniEngine only supports Windows")
endif()

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include our custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchDependencies)
include(CompileShaders)
include(MiniEngineConfig)

# Detect if this is a multi-config generator (Visual Studio, Ninja Multi-Config) or single-config (Ninja, Make)
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(IS_MULTI_CONFIG)
    # Multi-config generators: Debug, Profile, Release
    set(CMAKE_CONFIGURATION_TYPES "Debug;Profile;Release" CACHE STRING "" FORCE)
else()
    # Single-config generators: Default to Release if not specified
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug, Profile, Release)" FORCE)
    endif()
    # Validate build type
    set(VALID_BUILD_TYPES Debug Profile Release)
    if(NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
        message(WARNING "Unknown CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}', defaulting to Release")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Output")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Output")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Output")

# Per-config output directories (only for multi-config generators)
if(IS_MULTI_CONFIG)
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG}/Output")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG}/Output")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG}/Output")
    endforeach()
endif()

# Fetch all external dependencies via FetchContent
fetch_miniengine_dependencies()

# Find DXC compiler for shader compilation
find_dxc_compiler()

# Global compile definitions for all targets
add_compile_definitions(
    _GAMING_DESKTOP=1
    __WRL_NO_DEFAULT_LIB__
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _HAS_STD_BYTE=0         # Disable std::byte to avoid conflict with Windows byte
)

# Per-configuration compile definitions
add_compile_definitions(
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Profile>:NDEBUG>
    $<$<CONFIG:Profile>:PROFILE>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

# Global compile options for MSVC
if(MSVC)
    add_compile_options(
        /W4                     # Warning level 4
        /fp:fast                # Fast floating point
        /Oi                     # Enable intrinsics
        /Zc:__cplusplus         # Correct __cplusplus macro
        /EHsc                   # Standard exception handling
        /utf-8                  # Source and execution charset UTF-8
        $<$<CONFIG:Debug>:/Od>          # No optimization
        $<$<CONFIG:Debug>:/MDd>         # Debug runtime
        $<$<CONFIG:Debug>:/Zi>          # Debug info
        $<$<CONFIG:Debug>:/RTC1>        # Runtime checks
        $<$<CONFIG:Profile>:/O2>        # Full optimization
        $<$<CONFIG:Profile>:/MD>        # Release runtime
        $<$<CONFIG:Profile>:/Ob2>       # Inline expansion
        $<$<CONFIG:Profile>:/Zi>        # Debug info for profiling
        $<$<CONFIG:Profile>:/Gy>        # Function-level linking
        $<$<CONFIG:Release>:/O2>        # Full optimization
        $<$<CONFIG:Release>:/MD>        # Release runtime
        $<$<CONFIG:Release>:/Ob2>       # Inline expansion
        $<$<CONFIG:Release>:/Gy>        # Function-level linking
        $<$<CONFIG:Release>:/GL>        # Whole program optimization
    )

    # Link options
    add_link_options(
        $<$<CONFIG:Profile>:/OPT:REF>
        $<$<CONFIG:Profile>:/OPT:ICF>
        $<$<CONFIG:Profile>:/DEBUG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
        $<$<CONFIG:Release>:/LTCG>
    )
endif()

# Add subdirectories
add_subdirectory(Core)
add_subdirectory(Model)
add_subdirectory(ModelViewer)
add_subdirectory(ModelConverter)
add_subdirectory(Tools/SDFFontCreator)

# Print configuration summary
message(STATUS "")
message(STATUS "MiniEngine Configuration Summary:")
message(STATUS "  CMake version: ${CMAKE_VERSION}")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
if(IS_MULTI_CONFIG)
    message(STATUS "  Build configurations: ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
endif()
message(STATUS "  DXC compiler: ${DXC_EXECUTABLE}")
message(STATUS "")
