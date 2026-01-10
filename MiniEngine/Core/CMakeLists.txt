# Core library CMakeLists.txt

# Collect source files
set(CORE_SOURCES
    BitonicSort.cpp
    BuddyAllocator.cpp
    BufferManager.cpp
    Camera.cpp
    CameraController.cpp
    Color.cpp
    ColorBuffer.cpp
    CommandAllocatorPool.cpp
    CommandContext.cpp
    CommandListManager.cpp
    CommandSignature.cpp
    DDSTextureLoader.cpp
    DepthBuffer.cpp
    DepthOfField.cpp
    DescriptorHeap.cpp
    Display.cpp
    DynamicDescriptorHeap.cpp
    EngineProfiling.cpp
    EngineTuning.cpp
    FileUtility.cpp
    FXAA.cpp
    GameCore.cpp
    GpuBuffer.cpp
    GpuTimeManager.cpp
    GraphicsCommon.cpp
    GraphicsCore.cpp
    GraphRenderer.cpp
    ImageScaling.cpp
    Input.cpp
    LinearAllocator.cpp
    MotionBlur.cpp
    ParticleEffect.cpp
    ParticleEffectManager.cpp
    ParticleEmissionProperties.cpp
    PipelineState.cpp
    PixelBuffer.cpp
    PostEffects.cpp
    ReadbackBuffer.cpp
    RootSignature.cpp
    SamplerManager.cpp
    ShadowBuffer.cpp
    ShadowCamera.cpp
    SSAO.cpp
    SystemTime.cpp
    TemporalEffects.cpp
    TextRenderer.cpp
    Texture.cpp
    TextureManager.cpp
    UploadBuffer.cpp
    Utility.cpp
    Math/BoundingSphere.cpp
    Math/Frustum.cpp
    Math/Random.cpp
    Util/CommandLineArg.cpp
    pch.cpp
)

set(CORE_HEADERS
    BitonicSort.h
    BuddyAllocator.h
    BufferManager.h
    Camera.h
    CameraController.h
    Color.h
    ColorBuffer.h
    CommandAllocatorPool.h
    CommandContext.h
    CommandListManager.h
    CommandSignature.h
    d3dx12.h
    dds.h
    DDSTextureLoader.h
    DepthBuffer.h
    DepthOfField.h
    DescriptorHeap.h
    Display.h
    DynamicDescriptorHeap.h
    EngineProfiling.h
    EngineTuning.h
    EsramAllocator.h
    FileUtility.h
    FXAA.h
    GameCore.h
    GameInput.h
    GpuBuffer.h
    GpuResource.h
    GpuTimeManager.h
    GraphicsCommon.h
    GraphicsCore.h
    GraphRenderer.h
    Hash.h
    ImageScaling.h
    LinearAllocator.h
    MotionBlur.h
    ParticleEffect.h
    ParticleEffectManager.h
    ParticleEffectProperties.h
    ParticleShaderStructs.h
    pch.h
    PipelineState.h
    PixelBuffer.h
    PostEffects.h
    ReadbackBuffer.h
    RootSignature.h
    SamplerManager.h
    ShadowBuffer.h
    ShadowCamera.h
    SSAO.h
    SystemTime.h
    TemporalEffects.h
    TextRenderer.h
    Texture.h
    TextureManager.h
    UploadBuffer.h
    Utility.h
    VectorMath.h
    Math/BoundingBox.h
    Math/BoundingPlane.h
    Math/BoundingSphere.h
    Math/Common.h
    Math/Frustum.h
    Math/Functions.inl
    Math/Matrix3.h
    Math/Matrix4.h
    Math/Quaternion.h
    Math/Random.h
    Math/Scalar.h
    Math/Transform.h
    Math/Vector.h
    Util/CommandLineArg.h
    Benchmark/Statistics.h
)

# Create static library
add_library(Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Precompiled header setup
target_precompile_headers(Core PRIVATE pch.h)

# Include directories
target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Math
        ${CMAKE_CURRENT_SOURCE_DIR}/Benchmark
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Util
)

# Configure common settings
configure_miniengine_target(Core)

# Link nlohmann_json for benchmark system
target_link_libraries(Core PUBLIC nlohmann_json::nlohmann_json)

# Shader type overrides for shaders that don't follow naming convention
# These are extracted from Core.vcxproj where explicit <ShaderType> is specified
set(CORE_SHADER_TYPE_OVERRIDES
    # Pixel shaders
    BicubicHorizontalUpsamplePS:Pixel
    BicubicUpsampleGammaPS:Pixel
    BicubicUpsamplePS:Pixel
    BicubicVerticalUpsamplePS:Pixel
    BilinearUpsamplePS:Pixel
    BlendUIHDRPS:Pixel
    BufferCopyPS:Pixel
    CompositeHDRPS:Pixel
    CompositeSDRPS:Pixel
    DownsampleDepthPS:Pixel
    LanczosHorizontalPS:Pixel
    LanczosVerticalPS:Pixel
    MagnifyPixelsPS:Pixel
    MotionBlurFinalPassPS:Pixel
    ParticlePS:Pixel
    PerfGraphPS:Pixel
    PresentHDRPS:Pixel
    PresentSDRPS:Pixel
    ScaleAndCompositeHDRPS:Pixel
    ScaleAndCompositeSDRPS:Pixel
    SharpeningUpsampleGammaPS:Pixel
    SharpeningUpsamplePS:Pixel
    TextAntialiasPS:Pixel
    TextShadowPS:Pixel
    # Vertex shaders
    ParticleNoSortVS:Vertex
    ParticleVS:Vertex
    PerfGraphBackgroundVS:Vertex
    PerfGraphVS:Vertex
    ScreenQuadCommonVS:Vertex
    ScreenQuadPresentVS:Vertex
    TextVS:Vertex
)

# Compile shaders - output to a fixed location under build dir
compile_hlsl_shaders(
    TARGET Core
    SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/Shaders/Core"
    SHADER_TYPES ${CORE_SHADER_TYPE_OVERRIDES}
)

# Set static library options to suppress LNK4221 (MSVC only)
if(MSVC)
    set_target_properties(Core PROPERTIES
        STATIC_LIBRARY_OPTIONS "/IGNORE:4221"
    )
endif()

# Source grouping for IDE
source_group("Source Files" FILES ${CORE_SOURCES})
source_group("Header Files" FILES ${CORE_HEADERS})
source_group("Math" REGULAR_EXPRESSION "Math/.*")
source_group("Util" REGULAR_EXPRESSION "Util/.*")
source_group("Benchmark" REGULAR_EXPRESSION "Benchmark/.*")

# Shader files for IDE visibility (not compiled by C++ compiler)
file(GLOB CORE_SHADER_FILES "Shaders/*.hlsl" "Shaders/*.hlsli")
target_sources(Core PRIVATE ${CORE_SHADER_FILES})
source_group("Shaders" FILES ${CORE_SHADER_FILES})
set_source_files_properties(${CORE_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
