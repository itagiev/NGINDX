# Windows application project Sandbox
add_executable(Sandbox WIN32)

target_compile_features(Sandbox PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(Sandbox PRIVATE
        /W4
        /permissive-
    )
endif()

file(GLOB_RECURSE SANDBOX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(GLOB_RECURSE SANDBOX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

target_sources(Sandbox
    PRIVATE
        ${SANDBOX_HEADERS}
        ${SANDBOX_SOURCES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        ${SANDBOX_HEADERS}
        ${SANDBOX_SOURCES}
)

target_precompile_headers(Sandbox PRIVATE pch.h)

# Link Sandbox with NGINDX
# PRIVATE means that Sandbox dependencies are not transitively visible to other targets
target_link_libraries(Sandbox PRIVATE NGINDX)
