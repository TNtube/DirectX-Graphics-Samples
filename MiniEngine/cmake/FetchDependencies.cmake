# FetchDependencies.cmake - External dependency management via FetchContent

include(FetchContent)

function(fetch_miniengine_dependencies)
    message(STATUS "Fetching MiniEngine dependencies...")

    set(FETCHCONTENT_QUIET OFF)

    # ============================================
    # Microsoft.Direct3D.D3D12 Agility SDK 1.717.1-preview
    # ============================================
    FetchContent_Declare(
        D3D12AgilitySDK
        URL "https://api.nuget.org/v3-flatcontainer/microsoft.direct3d.d3d12/1.717.1-preview/microsoft.direct3d.d3d12.1.717.1-preview.nupkg"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(D3D12AgilitySDK)

    # Create imported target for D3D12 Agility SDK
    add_library(Microsoft::D3D12AgilitySDK INTERFACE IMPORTED GLOBAL)
    target_include_directories(Microsoft::D3D12AgilitySDK INTERFACE
        "${d3d12agilitysdk_SOURCE_DIR}/build/native/include"
    )

    # Store DLL locations for runtime deployment
    set(D3D12_AGILITY_SDK_BIN_DIR
        "${d3d12agilitysdk_SOURCE_DIR}/build/native/bin/x64"
        CACHE PATH "D3D12 Agility SDK binary directory" FORCE
    )

    # ============================================
    # Microsoft.Direct3D.DXC 1.8.2505.28
    # (Required for Cooperative Vectors shader compilation)
    # ============================================
    FetchContent_Declare(
        DXC
        URL "https://api.nuget.org/v3-flatcontainer/microsoft.direct3d.dxc/1.8.2505.28/microsoft.direct3d.dxc.1.8.2505.28.nupkg"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(DXC)

    # Store DXC executable path for shader compilation
    set(DXC_FETCHED_EXECUTABLE
        "${dxc_SOURCE_DIR}/build/native/bin/x64/dxc.exe"
        CACHE FILEPATH "Fetched DXC executable path" FORCE
    )
    set(DXC_BIN_DIR
        "${dxc_SOURCE_DIR}/build/native/bin/x64"
        CACHE PATH "DXC binary directory" FORCE
    )

    # ============================================
    # WinPixEventRuntime (GPU debugging)
    # ============================================
    FetchContent_Declare(
        WinPixEventRuntime
        URL "https://api.nuget.org/v3-flatcontainer/winpixeventruntime/1.0.240308001/winpixeventruntime.1.0.240308001.nupkg"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(WinPixEventRuntime)

    add_library(Microsoft::WinPixEventRuntime INTERFACE IMPORTED GLOBAL)
    target_include_directories(Microsoft::WinPixEventRuntime INTERFACE
        "${winpixeventruntime_SOURCE_DIR}/Include/WinPixEventRuntime"
    )
    target_link_directories(Microsoft::WinPixEventRuntime INTERFACE
        "${winpixeventruntime_SOURCE_DIR}/bin/x64"
    )
    target_link_libraries(Microsoft::WinPixEventRuntime INTERFACE WinPixEventRuntime)

    set(WINPIX_BIN_DIR
        "${winpixeventruntime_SOURCE_DIR}/bin/x64"
        CACHE PATH "WinPixEventRuntime binary directory" FORCE
    )

    # ============================================
    # DirectXTex (texture processing)
    # ============================================
    FetchContent_Declare(
        DirectXTex
        GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
        GIT_TAG oct2024
        GIT_SHALLOW TRUE
    )
    set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
    set(BUILD_DX11 OFF CACHE BOOL "" FORCE)
    set(BUILD_DX12 ON CACHE BOOL "" FORCE)
    set(BC_USE_OPENMP OFF CACHE BOOL "" FORCE)
    set(ENABLE_OPENEXR_SUPPORT OFF CACHE BOOL "" FORCE)
    set(ENABLE_LIBJPEG_SUPPORT OFF CACHE BOOL "" FORCE)
    set(ENABLE_LIBPNG_SUPPORT OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(DirectXTex)

    # ============================================
    # DirectXMesh (mesh optimization)
    # ============================================
    FetchContent_Declare(
        DirectXMesh
        GIT_REPOSITORY https://github.com/microsoft/DirectXMesh.git
        GIT_TAG oct2024
        GIT_SHALLOW TRUE
    )
    set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
    set(BUILD_DX12 ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(DirectXMesh)

    # ============================================
    # Assimp (ModelConverter only)
    # Assimp builds its own zlib internally, which we'll reuse
    # ============================================
    FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.4.3
        GIT_SHALLOW TRUE
    )
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(assimp)

    # Make assimp's zlib headers available globally
    # The zlibstatic target is created by assimp's internal zlib build
    target_include_directories(zlibstatic PUBLIC
        "${assimp_SOURCE_DIR}/contrib/zlib"
        "${assimp_BINARY_DIR}/contrib/zlib"
    )

    # ============================================
    # FreeType (SDFFontCreator only)
    # ============================================
    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG VER-2-13-3
        GIT_SHALLOW TRUE
    )
    set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(freetype)

    message(STATUS "All dependencies fetched successfully")
endfunction()
