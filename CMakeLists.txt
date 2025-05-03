cmake_minimum_required(VERSION 3.25)
project(VKLiteEngine)

set(CMAKE_CXX_STANDARD 20)

option(VULKAN_DEBUG "Put additional debug information into graphics code" ON)

if (VULKAN_DEBUG)
    add_compile_definitions(VULKAN_DEBUG)
endif ()

add_subdirectory(third_party EXCLUDE_FROM_ALL)

set(SRC
        src/main.cpp
        src/Core/Engine.cpp
        src/Core/Engine.h
        src/Core/Window.cpp
        src/Core/Window.h
        src/Core/InputSystem.cpp
        src/Core/InputSystem.h
        src/Core/KeyCodes.h
        src/Vulkan/VulkanContext.cpp
        src/Vulkan/VulkanContext.h
        src/Core/Utils.cpp
        src/Core/Utils.h
        src/Vulkan/ValidationLayersMessenger.cpp
        src/Vulkan/ValidationLayersMessenger.h
        src/Vulkan/SwapChain.cpp
        src/Vulkan/SwapChain.h
        src/Rendering/FrameRenderer.cpp
        src/Rendering/FrameRenderer.h
        src/Assets/AssetDescriptor.cpp
        src/Assets/AssetDescriptor.h
        src/Rendering/DeferredShading/GBuffer.cpp
        src/Rendering/DeferredShading/GBuffer.h
        src/Rendering/DeferredShading/DeferredShading.cpp
        src/Rendering/DeferredShading/DeferredShading.h
        src/Vulkan/Allocator/Allocator.cpp
        src/Vulkan/Allocator/Allocator.h
        src/Vulkan/Allocator/AllocatedBuffer.cpp
        src/Vulkan/Allocator/AllocatedBuffer.h
        src/Vulkan/Allocator/AllocatedImage.cpp
        src/Vulkan/Allocator/AllocatedImage.h
        src/Scene/Mesh/Material.cpp
        src/Scene/Mesh/Material.h
        src/Vulkan/Allocator/StagingBuffer.cpp
        src/Vulkan/Allocator/StagingBuffer.h
        src/Scene/Mesh/MeshLoader.cpp
        src/Scene/Mesh/MeshLoader.h
        src/Vulkan/VulkanUtils.cpp
        src/Vulkan/VulkanUtils.h
        src/Core/Assert.h
        src/Assets/AssetFactory.cpp
        src/Assets/AssetFactory.h
        src/Scene/SceneObject.cpp
        src/Scene/SceneObject.h
        src/Scene/TransformComponent.cpp
        src/Scene/TransformComponent.h
        src/Scene/Component.cpp
        src/Scene/Component.h
        src/Scene/Camera/Camera.cpp
        src/Scene/Camera/Camera.h
        src/Scene/Camera/PerspectiveCamera.cpp
        src/Scene/Camera/PerspectiveCamera.h
        src/Scene/Camera/Camera.cpp
        src/Scene/Camera/Camera.h
        src/Scene/Mesh/Mesh.cpp
        src/Scene/Mesh/Mesh.h
        src/Scene/Scene.cpp
        src/Scene/Scene.h
        src/Scene/Mesh/MeshComponent.cpp
        src/Scene/Mesh/MeshComponent.h
        src/Scene/Camera/CameraComponent.cpp
        src/Scene/Camera/CameraComponent.h
        src/Vulkan/UniformBuffer.cpp
        src/Vulkan/UniformBuffer.h
        src/Scene/BehaviourComponent.cpp
        src/Scene/BehaviourComponent.h
        src/Scene/Camera/FirstPersonControllerComponent.cpp
        src/Scene/Camera/FirstPersonControllerComponent.h
        src/Rendering/DeferredShading/GeometryPass.cpp
        src/Rendering/DeferredShading/GeometryPass.h
        src/Vulkan/DescriptorGenerator.cpp
        src/Vulkan/DescriptorGenerator.h
        src/Scene/SpinComponent.cpp
        src/Scene/SpinComponent.h
        src/Vulkan/SingleCommandBuffer.cpp
        src/Vulkan/SingleCommandBuffer.h
        src/Scene/Light/PointLightComponent.cpp
        src/Scene/Light/PointLightComponent.h
        src/Rendering/DeferredShading/LightPass.cpp
        src/Rendering/DeferredShading/LightPass.h
        src/Scene/LissajousLightsComponent.cpp
        src/Scene/LissajousLightsComponent.h
        src/Rendering/DeferredShading/LightVolumeMesh.cpp
        src/Rendering/DeferredShading/LightVolumeMesh.h
        src/Scene/Texture/Texture2D.cpp
        src/Scene/Texture/Texture2D.h
        src/Scene/Texture/TextureFactory.cpp
        src/Scene/Texture/TextureFactory.h
        src/Rendering/PostProcessing/Bloom.cpp
        src/Rendering/PostProcessing/Bloom.h
        src/Rendering/PostProcessing/ToneMapping.cpp
        src/Rendering/PostProcessing/ToneMapping.h)

add_executable(VKLiteEngine ${SRC})

target_include_directories(VKLiteEngine PUBLIC src)

target_link_libraries(VKLiteEngine PRIVATE Vulkan::Vulkan)
target_link_libraries(VKLiteEngine PRIVATE glfw)
target_link_libraries(VKLiteEngine PRIVATE VulkanMemoryAllocator)
target_link_libraries(VKLiteEngine PRIVATE glm)
target_link_libraries(VKLiteEngine PRIVATE glog::glog)
target_link_libraries(VKLiteEngine PRIVATE assimp)
target_link_libraries(VKLiteEngine PRIVATE stb_image)

target_precompile_headers(VKLiteEngine PRIVATE src/pch.h)

set_target_properties(VKLiteEngine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
)

# Shader compilation
set(SHADERS_SRC
        shaders/deferred/geometry.vert
        shaders/deferred/geometry.frag
        shaders/deferred/point_light.vert
        shaders/deferred/point_light.frag
        shaders/post_processing/fullscreen_triangle.vert
        shaders/post_processing/bloom_downsample.frag
        shaders/post_processing/bloom_upsample.frag
        shaders/post_processing/aces.frag)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

foreach (GLSL ${SHADERS_SRC})
    message(STATUS "BUILDING SHADER")
    set(FULL_PATH ${CMAKE_SOURCE_DIR}/${GLSL})
    set(SPIRV "${PROJECT_SOURCE_DIR}/bin/${GLSL}.spv")
    message(STATUS ${GLSL})
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${FULL_PATH} -o ${SPIRV} -g
            DEPENDS ${FULL_PATH})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(VKLiteEngine Shaders)
