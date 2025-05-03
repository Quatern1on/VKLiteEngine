# VKLiteEngine

VKLiteEngine is a simple game engine built primarily to learn Vulkan.

## Features
- **Deferred Shading**
- **ACES Tone Mapping**
- **Bloom** effect based on the Call of Duty algorithm.
- **Simple Entity Component System**

## Controls
- **WASD** - Move the camera.
- **E** - Move Up.
- **Q** - Move Down.
- **Shift** - Move faster.

### Post-processing and Channel Toggles:
- **1** - Show post-processed output.
- **2** - Show main output without post-processing.
- **3** - Show Albedo channel.
- **4** - Show Normals.
- **5** - Show Metallic Roughness.
- **6** - Show Bloom texture.

## Requirements
- **CMake**
- **Microsoft Visual Studio Compiler (MSVC)**
- **Vulkan SDK** version 1.3 or higher.

## Building

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build --config Release
```

## Scenes
Modify ```src/Core/Engine.cpp``` ```Engine::init``` method to select which scene is loaded.