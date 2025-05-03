cmake_minimum_required(VERSION 3.25)

find_package(Vulkan REQUIRED GLOBAL)

set(BUILD_SHARED_LIBS off CACHE BOOL "Build shared libraries")
set(WITH_GFLAGS off CACHE BOOL "Use gflags")

add_subdirectory(glfw EXCLUDE_FROM_ALL)
add_subdirectory(VulkanMemoryAllocator EXCLUDE_FROM_ALL)
add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(glog EXCLUDE_FROM_ALL)
add_subdirectory(assimp EXCLUDE_FROM_ALL)
add_subdirectory(stb_image EXCLUDE_FROM_ALL)