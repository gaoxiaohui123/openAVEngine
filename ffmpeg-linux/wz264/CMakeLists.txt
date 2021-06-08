cmake_minimum_required (VERSION 2.8.11)
project (wz264_sample_encoder)

## Include Header path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

## Target
add_executable (wz264_sample_encoder "${CMAKE_CURRENT_SOURCE_DIR}/samples/wz264_sample_encoder.c")

## Link libraries
find_library(WZ264ENC wz264 ${CMAKE_CURRENT_SOURCE_DIR}/lib/)
target_link_libraries(wz264_sample_encoder ${WZ264ENC})

## Copy dll to exe
add_custom_command(TARGET wz264_sample_encoder POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/bin/wz264.dll ${CMAKE_BINARY_DIR}/$<CONFIG>)
