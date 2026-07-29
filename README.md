# FigmentEngine

<p align="center">
    <img src="https://github.com/Xephyrose/FigmentEngine/blob/main/figment-engine.png" width="600" alt="Logo">
</p>

## What is Figment Engine?

Figment Engine is a 2D & 3D game engine written in C++, utilizing the SDL_GPU API. This engine is primarily being written to teach myself basic engine programming, but possibly to make some games down the line. It currently features GLTF meshes in the form of .glb files, as well as texture loading (.png). Physics are now implemented, using [Box3D](https://github.com/erincatto/box3d) for 3D, and [Box2D](https://github.com/erincatto/box2d) for 2D. 

## How do I use it?

This engine follows a simple Node tree structure. A Node is essentially any kind of object, there are no components.

A good place to start is the Game struct. This struct contains basic game loop functionality, such as Init() which runs one time when the game starts, Event() which runs when there is a user input, and Update() contains the main game loop.

To get started, you can add children to the root node. Here's an example, spawning a FreeCam3D that will allow you to fly around in a 3D environment:

```c++
auto* freeCam = new FreeCam3D();
appState->current_camera_3d = freeCam;
appState->root.addChild(std::unique_ptr<Node>(freeCam));
```
...and another example, spawning a mesh as a child of the root:
```c++
auto* meshInstance = new MeshInstance3D();
meshInstance->mesh = "zulu.glb";
appState->root.addChild(std::unique_ptr<Node>(meshInstance));
```

The primary Nodes are: MeshInstance3D, and Sprite2D. 
- Node / Node2D / Node3D
- Camera2D / Camera3D
- MeshInstance3D
- BoxBody3D / CapsuleBody3D / HeightFieldBody3D
- DirectionalLight3D / PointLight3D / SpotLight3D

Most systems can be built off of these alone.

## Compiling
Compiling has been tested on and supports both windows and linux. While tested on CachyOS & Windows 11, it should be relatively platform-agnostic. To build, run:

```shell
# Windows
cmake --build ./cmake-build-debug --target FigmentEngine -j 10
```
```bash
# Linux
cmake --build ./cmake-build-debug --target FigmentEngine -- -j 10
```

WARNING: Only Vulkan is known to work. Metal is untested, and DirectX fails to bind any pipeline that includes a StructuredBuffer (?). For this reason, DirectX is currently force-disabled.