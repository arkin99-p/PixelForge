add_executable(PixelForge_rotated main.cpp)

target_link_libraries(PixelForge_rotated PRIVATE PixelForge)

file(COPY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_BINARY_DIR}/resources)