add_library(PixelForge STATIC
    math.cpp
    pixel_forge.cpp
    texture.cpp
)

target_include_directories(PixelForge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(PixelForge PRIVATE SDL3::SDL3)

target_include_directories(PixelForge PUBLIC ${CMAKE_SOURCE_DIR}/external)