# Model library CMakeLists.txt

set(MODEL_SOURCES
    Animation.cpp
    BuildH3D.cpp
    glTF.cpp
    IndexOptimizePostTransform.cpp
    LightManager.cpp
    MeshConvert.cpp
    Model.cpp
    ModelConvert.cpp
    ModelH3D.cpp
    ModelLoader.cpp
    ParticleEffects.cpp
    Renderer.cpp
    SponzaRenderer.cpp
    TextureConvert.cpp
)

set(MODEL_HEADERS
    Animation.h
    ConstantBuffers.h
    glTF.h
    IndexOptimizePostTransform.h
    json.hpp
    LightManager.h
    MeshConvert.h
    Model.h
    ModelH3D.h
    ModelLoader.h
    ParticleEffects.h
    Renderer.h
    SponzaRenderer.h
    TextureConvert.h
)

# Create static library
add_library(Model STATIC ${MODEL_SOURCES} ${MODEL_HEADERS})

# Model depends on Core (public so consumers also get Core includes)
target_link_libraries(Model PUBLIC Core)

# Link DirectXTex and DirectXMesh
target_link_libraries(Model PUBLIC DirectXTex DirectXMesh)

# Include directories
target_include_directories(Model
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Core
)

# No precompiled header for Model (based on vcxproj: <PrecompiledHeader>NotUsing</PrecompiledHeader>)

# Configure common settings
configure_miniengine_target(Model)

# Shader type overrides - all Model shaders need explicit types
# because none of them follow the standard suffix convention
set(MODEL_SHADER_TYPE_OVERRIDES
    # Pixel shaders
    CutoutDepthPS:Pixel
    DefaultNoTangentNoUV1PS:Pixel
    DefaultNoTangentPS:Pixel
    DefaultNoUV1PS:Pixel
    DefaultPS:Pixel
    DepthViewerPS:Pixel
    ModelViewerPS:Pixel
    SkyboxPS:Pixel
    # Vertex shaders
    CutoutDepthSkinVS:Vertex
    CutoutDepthVS:Vertex
    DefaultNoTangentNoUV1SkinVS:Vertex
    DefaultNoTangentNoUV1VS:Vertex
    DefaultNoTangentSkinVS:Vertex
    DefaultNoTangentVS:Vertex
    DefaultNoUV1SkinVS:Vertex
    DefaultNoUV1VS:Vertex
    DefaultSkinVS:Vertex
    DefaultVS:Vertex
    DepthOnlySkinVS:Vertex
    DepthOnlyVS:Vertex
    DepthViewerVS:Vertex
    ModelViewerVS:Vertex
    SkyboxVS:Vertex
    # Compute shaders (these follow the CS suffix convention, no override needed)
    # FillLightGridCS_8, FillLightGridCS_16, FillLightGridCS_24, FillLightGridCS_32
)

# Compile shaders - Model shaders can include Core shaders and their own includes
compile_hlsl_shaders(
    TARGET Model
    SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/Shaders/Model"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/Core/Shaders"
    SHADER_TYPES ${MODEL_SHADER_TYPE_OVERRIDES}
)

# Set static library options to suppress LNK4221 (MSVC only)
if(MSVC)
    set_target_properties(Model PROPERTIES
        STATIC_LIBRARY_OPTIONS "/IGNORE:4221"
    )
endif()

# Source grouping for IDE
source_group("Source Files" FILES ${MODEL_SOURCES})
source_group("Header Files" FILES ${MODEL_HEADERS})

# Shader files for IDE visibility
file(GLOB MODEL_SHADER_FILES "Shaders/*.hlsl" "Shaders/*.hlsli")
target_sources(Model PRIVATE ${MODEL_SHADER_FILES})
source_group("Shaders" FILES ${MODEL_SHADER_FILES})
set_source_files_properties(${MODEL_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
