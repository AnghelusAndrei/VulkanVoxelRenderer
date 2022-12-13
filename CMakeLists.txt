cmake_minimum_required(VERSION 3.3)
project(VulkanVoxelRenderer) 
set(CXX_STANDARD 17)
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

set(GLFW_LIBRARY_TYPE SHARED)
set(BUILD_SHARED_LIBS 1)
add_subdirectory(lib/glfw)

add_subdirectory(lib/glm)

if(WIN32)
    set(cur_dir "C:/VulkanSDK")
    file(GLOB children RELATIVE ${cur_dir} ${cur_dir}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${cur_dir}/${child})
            set(ENV{VulkanSDK} ${cur_dir}/${child})
        endif()
    endforeach()
endif()

find_package(Vulkan REQUIRED)
add_subdirectory(lib/VulkanMemoryAllocator)

file(GLOB SOURCES "src/*.cpp" "usr/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

file(GLOB SHADERS "src/shaders/*.comp")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

find_program(GLSLC "glslc")
add_custom_target(Shaders)
foreach(shader ${SHADERS})
    get_filename_component(shaderfilename ${shader} NAME_WE)

    if(GLSLC)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        COMMAND ${GLSLC} ${shader} -O -o "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        DEPENDS ${shader}
    )
    else()
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        COMMAND $ENV{VULKAN_SDK}/x86_64/glslc shader -O -o "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv"
        DEPENDS ${shader}
    )
    endif()
    
    add_custom_target(${shaderfilename} DEPENDS "${CMAKE_BINARY_DIR}/shaders/${shaderfilename}.spv")
    add_dependencies(Shaders ${shaderfilename}) 

endforeach()

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.45.0 COMPONENTS json) 

if(NOT Boost_FOUND)
    message(SEND_ERROR "Boost not found")
endif()
target_include_directories(${PROJECT_NAME} PUBLIC lib/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC lib/glm)
target_include_directories(${PROJECT_NAME} PUBLIC lib/VulkanMemoryAllocator/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES} Vulkan::Vulkan glfw glm VulkanMemoryAllocator)
add_dependencies(${PROJECT_NAME} Shaders)