# CompileShaders.cmake - HLSL shader compilation using DXC

# Find DXC (DirectX Shader Compiler)
# Prefers the fetched DXC from NuGet (required for cooperative vectors)
function(find_dxc_compiler)
    # First, check if we have a fetched DXC from FetchDependencies.cmake
    if(DXC_FETCHED_EXECUTABLE AND EXISTS "${DXC_FETCHED_EXECUTABLE}")
        file(TO_CMAKE_PATH "${DXC_FETCHED_EXECUTABLE}" DXC_EXECUTABLE)
        message(STATUS "Using fetched DXC: ${DXC_EXECUTABLE}")
        set(DXC_EXECUTABLE "${DXC_EXECUTABLE}" CACHE FILEPATH "DXC executable path" FORCE)
        return()
    endif()

    # Fallback: Try to find DXC from Windows SDK or Vulkan SDK
    find_program(DXC_EXECUTABLE dxc
        HINTS
            "$ENV{WindowsSdkVerBinPath}/x64"
            "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64"
            "$ENV{WindowsSdkDir}/bin/10.0.26100.0/x64"
            "$ENV{WindowsSdkDir}/bin/10.0.22621.0/x64"
            "$ENV{WindowsSdkDir}/bin/10.0.22000.0/x64"
            "$ENV{WindowsSdkDir}/bin/10.0.19041.0/x64"
            "$ENV{VULKAN_SDK}/Bin"
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22000.0/x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0/x64"
        DOC "DirectX Shader Compiler (dxc.exe)"
    )

    if(NOT DXC_EXECUTABLE)
        message(FATAL_ERROR "Could not find DXC (DirectX Shader Compiler). "
            "Please ensure Windows SDK 10.0.19041.0+ is installed or set DXC_EXECUTABLE manually.")
    endif()

    # Normalize path (convert backslashes to forward slashes for CMake)
    file(TO_CMAKE_PATH "${DXC_EXECUTABLE}" DXC_EXECUTABLE)

    message(STATUS "Found DXC (fallback): ${DXC_EXECUTABLE}")
    message(WARNING "Using system DXC. Cooperative vectors may not be supported. "
        "Consider ensuring DXC is fetched via FetchDependencies.cmake.")
    set(DXC_EXECUTABLE "${DXC_EXECUTABLE}" CACHE FILEPATH "DXC executable path" FORCE)
endfunction()

# Determine shader type and profile from filename
# Returns: TYPE (cs/vs/ps/gs/hs/ds) and PROFILE (e.g., cs_6_9)
function(get_shader_type_from_filename FILENAME OUT_TYPE OUT_PROFILE)
    get_filename_component(NAME_WE ${FILENAME} NAME_WE)

    # Check suffix for shader type
    if(NAME_WE MATCHES "CS$")
        set(${OUT_TYPE} "cs" PARENT_SCOPE)
        set(${OUT_PROFILE} "cs_6_9" PARENT_SCOPE)
    elseif(NAME_WE MATCHES "VS$")
        set(${OUT_TYPE} "vs" PARENT_SCOPE)
        set(${OUT_PROFILE} "vs_6_9" PARENT_SCOPE)
    elseif(NAME_WE MATCHES "PS$")
        set(${OUT_TYPE} "ps" PARENT_SCOPE)
        set(${OUT_PROFILE} "ps_6_9" PARENT_SCOPE)
    elseif(NAME_WE MATCHES "GS$")
        set(${OUT_TYPE} "gs" PARENT_SCOPE)
        set(${OUT_PROFILE} "gs_6_9" PARENT_SCOPE)
    elseif(NAME_WE MATCHES "HS$")
        set(${OUT_TYPE} "hs" PARENT_SCOPE)
        set(${OUT_PROFILE} "hs_6_9" PARENT_SCOPE)
    elseif(NAME_WE MATCHES "DS$")
        set(${OUT_TYPE} "ds" PARENT_SCOPE)
        set(${OUT_PROFILE} "ds_6_9" PARENT_SCOPE)
    else()
        # Default to compute shader (matching VS project behavior)
        set(${OUT_TYPE} "cs" PARENT_SCOPE)
        set(${OUT_PROFILE} "cs_6_9" PARENT_SCOPE)
    endif()
endfunction()

# Main function to compile shaders for a target
# Usage: compile_hlsl_shaders(
#     TARGET <target_name>
#     SHADER_DIR <path_to_shaders>
#     OUTPUT_DIR <path_for_compiled_headers>
#     [INCLUDE_DIRS <additional_include_dirs>...]
#     [SHADER_TYPES <shader_name>:<type> ...]  # Override type for specific shaders
# )
function(compile_hlsl_shaders)
    cmake_parse_arguments(
        SHADER
        ""
        "TARGET;SHADER_DIR;OUTPUT_DIR"
        "INCLUDE_DIRS;SHADER_TYPES"
        ${ARGN}
    )

    if(NOT SHADER_TARGET)
        message(FATAL_ERROR "compile_hlsl_shaders: TARGET is required")
    endif()
    if(NOT SHADER_SHADER_DIR)
        message(FATAL_ERROR "compile_hlsl_shaders: SHADER_DIR is required")
    endif()
    if(NOT SHADER_OUTPUT_DIR)
        message(FATAL_ERROR "compile_hlsl_shaders: OUTPUT_DIR is required")
    endif()

    # Find all .hlsl files
    file(GLOB HLSL_FILES "${SHADER_SHADER_DIR}/*.hlsl")

    if(NOT HLSL_FILES)
        message(STATUS "No HLSL files found in ${SHADER_SHADER_DIR}")
        return()
    endif()

    # Find all .hlsli files for dependencies
    file(GLOB HLSLI_FILES "${SHADER_SHADER_DIR}/*.hlsli")

    # Build include directory list
    set(ALL_INCLUDE_DIRS "${SHADER_SHADER_DIR}")
    if(SHADER_INCLUDE_DIRS)
        list(APPEND ALL_INCLUDE_DIRS ${SHADER_INCLUDE_DIRS})
    endif()

    # Build -I flags for DXC
    set(DXC_INCLUDE_FLAGS "")
    foreach(INC_DIR ${ALL_INCLUDE_DIRS})
        list(APPEND DXC_INCLUDE_FLAGS "-I${INC_DIR}")
    endforeach()

    # Parse shader type overrides into a map-like structure
    set(TYPE_OVERRIDES "")
    foreach(OVERRIDE ${SHADER_SHADER_TYPES})
        list(APPEND TYPE_OVERRIDES "${OVERRIDE}")
    endforeach()

    # Set output directory (will be created by custom commands)
    set(COMPILED_SHADERS_DIR "${SHADER_OUTPUT_DIR}/CompiledShaders")

    set(COMPILED_SHADER_HEADERS "")

    foreach(HLSL_FILE ${HLSL_FILES})
        get_filename_component(SHADER_NAME ${HLSL_FILE} NAME_WE)
        get_filename_component(SHADER_FILENAME ${HLSL_FILE} NAME)

        # Check for type override
        set(SHADER_TYPE "")
        set(SHADER_PROFILE "")
        foreach(OVERRIDE ${TYPE_OVERRIDES})
            if(OVERRIDE MATCHES "^${SHADER_NAME}:(.+)$")
                set(OVERRIDE_TYPE "${CMAKE_MATCH_1}")
                if(OVERRIDE_TYPE STREQUAL "Vertex")
                    set(SHADER_TYPE "vs")
                    set(SHADER_PROFILE "vs_6_9")
                elseif(OVERRIDE_TYPE STREQUAL "Pixel")
                    set(SHADER_TYPE "ps")
                    set(SHADER_PROFILE "ps_6_9")
                elseif(OVERRIDE_TYPE STREQUAL "Compute")
                    set(SHADER_TYPE "cs")
                    set(SHADER_PROFILE "cs_6_9")
                elseif(OVERRIDE_TYPE STREQUAL "Geometry")
                    set(SHADER_TYPE "gs")
                    set(SHADER_PROFILE "gs_6_9")
                elseif(OVERRIDE_TYPE STREQUAL "Hull")
                    set(SHADER_TYPE "hs")
                    set(SHADER_PROFILE "hs_6_9")
                elseif(OVERRIDE_TYPE STREQUAL "Domain")
                    set(SHADER_TYPE "ds")
                    set(SHADER_PROFILE "ds_6_9")
                endif()
                break()
            endif()
        endforeach()

        # If no override, determine from filename
        if(NOT SHADER_TYPE)
            get_shader_type_from_filename(${SHADER_FILENAME} SHADER_TYPE SHADER_PROFILE)
        endif()

        # Output header file path
        set(OUTPUT_HEADER "${COMPILED_SHADERS_DIR}/${SHADER_NAME}.h")

        # Variable name: g_p<ShaderName>
        set(VARIABLE_NAME "g_p${SHADER_NAME}")

        # Build DXC command
        # Common flags:
        # -T <profile>: Target profile (cs_6_9, vs_6_9, etc.)
        # -E main: Entry point
        # -HV 2021: HLSL version 2021
        # -Fh <file>: Output header file
        # -Vn <name>: Variable name in header
        # Debug: -Zi -Qembed_debug for debug info
        # Release: -O3 for optimization

        # We use generator expressions for config-specific flags
        add_custom_command(
            OUTPUT "${OUTPUT_HEADER}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${COMPILED_SHADERS_DIR}"
            COMMAND "${DXC_EXECUTABLE}"
                -T ${SHADER_PROFILE}
                -E main
                -HV 2021
                -Fh "${OUTPUT_HEADER}"
                -Vn "${VARIABLE_NAME}"
                ${DXC_INCLUDE_FLAGS}
                -D _GAMING_DESKTOP=1
                "$<$<CONFIG:Debug>:-Zi>"
                "$<$<CONFIG:Debug>:-Qembed_debug>"
                "$<$<CONFIG:Debug>:-Od>"
                "$<$<NOT:$<CONFIG:Debug>>:-O3>"
                "${HLSL_FILE}"
            DEPENDS "${HLSL_FILE}" ${HLSLI_FILES}
            WORKING_DIRECTORY "${SHADER_SHADER_DIR}"
            COMMENT "Compiling HLSL [${SHADER_PROFILE}]: ${SHADER_FILENAME} -> ${SHADER_NAME}.h"
            VERBATIM
            COMMAND_EXPAND_LISTS
        )

        list(APPEND COMPILED_SHADER_HEADERS "${OUTPUT_HEADER}")
    endforeach()

    # Create a custom target for the shaders
    add_custom_target(${SHADER_TARGET}_Shaders
        DEPENDS ${COMPILED_SHADER_HEADERS}
        COMMENT "Compiling shaders for ${SHADER_TARGET}"
    )

    # Add dependency so main target waits for shaders
    add_dependencies(${SHADER_TARGET} ${SHADER_TARGET}_Shaders)

    # Add the output directory to include path
    target_include_directories(${SHADER_TARGET} PRIVATE
        "${SHADER_OUTPUT_DIR}"
    )

    # Export the list of compiled headers
    set(${SHADER_TARGET}_COMPILED_SHADERS ${COMPILED_SHADER_HEADERS} PARENT_SCOPE)

    list(LENGTH HLSL_FILES SHADER_COUNT)
    message(STATUS "Configured ${SHADER_COUNT} shaders for ${SHADER_TARGET}")
endfunction()
