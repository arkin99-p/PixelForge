add_executable(PixelForge_textured main.cpp)

target_link_libraries(PixelForge_textured PRIVATE PixelForge)

file(COPY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_BINARY_DIR}/resources)