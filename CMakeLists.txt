
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(Raytracer)

file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/libs/piccolo/*.h
	${CMAKE_SOURCE_DIR}/libs/piccolo/*.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(libs/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
add_subdirectory(libs/glewlib/build/cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE glew)