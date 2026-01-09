# MiniEngineConfig.cmake - Common configuration settings for MiniEngine

# Common Windows system libraries
set(MINIENGINE_SYSTEM_LIBS
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    comdlg32.lib
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib
    d3d12.lib
    dxgi.lib
    dxguid.lib
    d3d11.lib
    winmm.lib
    comctl32.lib
)

# Function to configure a MiniEngine target with common settings
function(configure_miniengine_target TARGET_NAME)
    # Add system libraries
    target_link_libraries(${TARGET_NAME} PRIVATE ${MINIENGINE_SYSTEM_LIBS})

    # Add D3D12 Agility SDK
    target_link_libraries(${TARGET_NAME} PRIVATE Microsoft::D3D12AgilitySDK)

    # Add WinPixEventRuntime
    target_link_libraries(${TARGET_NAME} PRIVATE Microsoft::WinPixEventRuntime)

    # Disable specific warnings that are common in the codebase
    target_compile_options(${TARGET_NAME} PRIVATE
        /wd4201  # nonstandard extension: nameless struct/union
        /wd4238  # nonstandard extension: class rvalue used as lvalue
        /wd4239  # nonstandard extension: argument rvalue to non-const reference
        /wd4324  # structure was padded due to __declspec(align())
    )
endfunction()

# Function to deploy D3D12 Agility SDK DLLs to output directory
function(deploy_d3d12_agility_sdk TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/D3D12"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${D3D12_AGILITY_SDK_BIN_DIR}/D3D12Core.dll"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/D3D12/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${D3D12_AGILITY_SDK_BIN_DIR}/d3d12SDKLayers.dll"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/D3D12/"
        COMMENT "Deploying D3D12 Agility SDK to ${TARGET_NAME} output directory"
    )
endfunction()

# Function to deploy WinPixEventRuntime DLL
function(deploy_winpix_runtime TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WINPIX_BIN_DIR}/WinPixEventRuntime.dll"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/"
        COMMENT "Deploying WinPixEventRuntime to ${TARGET_NAME} output directory"
    )
endfunction()

# Function to copy assets to output directory
function(deploy_assets TARGET_NAME ASSET_DIR DEST_SUBDIR)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSET_DIR}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/${DEST_SUBDIR}"
        COMMENT "Deploying assets from ${ASSET_DIR} to ${DEST_SUBDIR}"
    )
endfunction()

# Function to configure a Windows application (as opposed to console)
function(configure_windows_app TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    # High DPI awareness
    target_link_options(${TARGET_NAME} PRIVATE
        "/MANIFEST:EMBED"
        "/MANIFESTINPUT:${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../dpiaware.manifest"
    )
endfunction()
