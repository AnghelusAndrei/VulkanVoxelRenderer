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

find_package(Vulkan REQUIRED)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

file(GLOB SHADERS "src/*.comp")
file(MAKE_DIRECTORY "shaders")

find_program(GLSLC "glslc")
add_custom_target(Shaders)
foreach(shader ${SHADERS})
    get_filename_component(shaderfilename ${shader} NAME_WE)

    if(GLSLC)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        COMMAND ${GLSLC} ${shader} -g -o "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        DEPENDS ${shader}
    )
    else()
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        COMMAND $ENV{VULKAN_SDK}/x86_64/glslc shader -o "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        DEPENDS ${shader}
    )
    endif()
    
    add_custom_target(${shaderfilename} DEPENDS "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv")
    add_dependencies(Shaders ${shaderfilename}) 

endforeach()




target_include_directories(${PROJECT_NAME} PUBLIC lib/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC lib/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm)
add_dependencies(${PROJECT_NAME} Shaders)
