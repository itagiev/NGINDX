# Static library NGINDX
add_library(NGINDX STATIC)

target_compile_features(NGINDX PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(NGINDX PRIVATE
        /W4
        /permissive-
    )
endif()

# Specify the path where the compiler searches for header files
# PRIVATE - occurs only within the target
# PUBLIC - occurs within the target and for all targets that link with this target
# INTERFACE - occurs only for the target that links with this target; not needed within the target itself
target_include_directories(NGINDX
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Recursively search for all .h header files in the NGINDX folder
file(GLOB_RECURSE NGINDX_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/NGINDX/*.h
)

# Recursively search for all .cpp files in the NGINDX folder
file(GLOB_RECURSE NGINDX_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/NGINDX/*.cpp
)

# Add files to the NGINDX library
# PRIVATE - files are used only within NGINDX
# (unlike PUBLIC/INTERFACE, they do not affect targets that link with NGINDX)
target_sources(NGINDX
    PRIVATE
        ${NGINDX_HEADERS}
        ${NGINDX_SOURCES}
)

# Build the project structure in the IDE the same as in the project with the code
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        ${NGINDX_HEADERS}
        ${NGINDX_SOURCES}
)

target_precompile_headers(NGINDX PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/NGINDX/pch.h)

target_compile_definitions(NGINDX
    PUBLIC
        UNICODE
        _UNICODE
)

# Link headers
target_include_directories(NGINDX PUBLIC
    "${PROJECT_SOURCE_DIR}/vendor/DirectX-Headers/include"
)

# Link libraries
target_link_libraries(NGINDX PUBLIC DirectXTK12)
