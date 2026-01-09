# ModelConverter executable CMakeLists.txt

set(MODELCONVERTER_SOURCES
    IndexOptimizePostTransform.cpp
    Main.cpp
    ModelAssimp.cpp
    ModelOptimize.cpp
)

set(MODELCONVERTER_HEADERS
    IndexOptimizePostTransform.h
    ModelAssimp.h
)

# Create console application
add_executable(ModelConverter ${MODELCONVERTER_SOURCES} ${MODELCONVERTER_HEADERS})

# Link dependencies - Model brings Core transitively
target_link_libraries(ModelConverter PRIVATE
    Model
    assimp
    zlibstatic
)

# Include directories
target_include_directories(ModelConverter PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/Model
)

# Configure common settings
configure_miniengine_target(ModelConverter)

# Console subsystem
set_target_properties(ModelConverter PROPERTIES
    WIN32_EXECUTABLE FALSE
)

# Linker options
target_link_options(ModelConverter PRIVATE
    $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
    /NODEFAULTLIB:LIBCMT
    /SUBSYSTEM:CONSOLE
)

# Source grouping for IDE
source_group("Source Files" FILES ${MODELCONVERTER_SOURCES})
source_group("Header Files" FILES ${MODELCONVERTER_HEADERS})
