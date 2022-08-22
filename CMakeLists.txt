cmake_minimum_required(VERSION 3.3)
project(VulkanVoxelRenderer) 
set(CXX_STANDARD 11)
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

set(GLFW_LIBRARY_TYPE SHARED)
set(BUILD_SHARED_LIBS 1)

add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)

file(GLOB SOURCES "src/*.cpp")
find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC lib/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC lib/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm)

#file(GLOB VULKAN_HEADERS "lib/Vulkan-Hpp/*.hpp")
#target_precompile_headers(${PROJECT_NAME} PUBLIC ${VULKAN_HEADERS})
#file(COPY "assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#file(GLOB_RECURSE shaders "src/*.geom" "src/*.vert" "src/*.frag")
#file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)