# SDFFontCreator tool CMakeLists.txt

set(SDFFONTCREATOR_SOURCES
    SDFFontCreator.cpp
)

# Create console application
add_executable(SDFFontCreator ${SDFFONTCREATOR_SOURCES})

# Link FreeType
target_link_libraries(SDFFontCreator PRIVATE freetype)

# Basic Windows libs
target_link_libraries(SDFFontCreator PRIVATE
    kernel32.lib
    user32.lib
    gdi32.lib
    shell32.lib
)

# Console subsystem
set_target_properties(SDFFontCreator PROPERTIES
    WIN32_EXECUTABLE FALSE
)

# Debug linker option
target_link_options(SDFFontCreator PRIVATE
    $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
)

# Source grouping for IDE
source_group("Source Files" FILES ${SDFFONTCREATOR_SOURCES})
