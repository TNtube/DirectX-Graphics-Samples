# ModelViewer executable CMakeLists.txt

set(MODELVIEWER_SOURCES
    ModelViewer.cpp
)

set(MODELVIEWER_HEADERS
    Animateable.h
)

# Create Windows application (not console)
add_executable(ModelViewer WIN32 ${MODELVIEWER_SOURCES} ${MODELVIEWER_HEADERS})

# Link dependencies - Model brings Core transitively
target_link_libraries(ModelViewer PRIVATE
    Model
    zlibstatic
)

# Include directories
target_include_directories(ModelViewer PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/Model
)

# Configure common settings
configure_miniengine_target(ModelViewer)

# Debug linker option - avoid mixing debug/release CRT
target_link_options(ModelViewer PRIVATE
    $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
)

# Deploy D3D12 Agility SDK DLLs
deploy_d3d12_agility_sdk(ModelViewer)

# Deploy WinPixEventRuntime DLL
deploy_winpix_runtime(ModelViewer)

# Deploy assets - Sponza scene and textures
deploy_assets(ModelViewer "${CMAKE_CURRENT_SOURCE_DIR}/Sponza" "Sponza")
deploy_assets(ModelViewer "${CMAKE_CURRENT_SOURCE_DIR}/Textures" "Textures")

# Source grouping for IDE
source_group("Source Files" FILES ${MODELVIEWER_SOURCES})
source_group("Header Files" FILES ${MODELVIEWER_HEADERS})

# Set working directory for debugging in Visual Studio
set_target_properties(ModelViewer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ModelViewer>"
)
