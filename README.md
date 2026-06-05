# FigmentEngine

<p align="center">
  <a href="https://godotengine.org">
    <img src="https://github.com/Xephyrose/FigmentEngine/blob/main/figment-engine.png" width="400" alt="Logo">
  </a>
</p>

## What is Figment Engine?

Figment Engine is a 2D & 3D game engine written in C++, utilizing the SDL_GPU API. This engine is primarily being written to teach myself basic engine programming, but possibly to make some games down the line. It currently features GLTF meshes in the form of .glb files, as well as texture loading (.png). Physics are not yet implemented, but will eventually use [Jolt Physics](https://github.com/jrouwe/joltphysics) for 3D, and [Box2D](https://github.com/erincatto/box2d) for 2D. 

## How do I use it?

For the time being, there's no main place to put a game loop. In main.cpp, there's SDL_AppInit(), which runs once upon the game starting. Contast to SDL_AppQuit(), which runs when the game is closed. SDL_AppEvent() runs when there is a user input, and SDL_AppIterate() is the main game loop.

To get started, you can add children to the root node. Here's an example, spawning a FreeCam3D that will allow you to fly around in a 3d environment:

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

The primary Nodes are Node, Node2D/Node3D, Camera2D/Camera3D, MeshInstance3D, and Sprite2D. Most systems can be built off of these.

## Compiling
Currently, compiling is limited to Linux. This is due to the use of system libraries, which must be accessible via path; this currently just being [OpenGL Mathematics (GLM)](https://github.com/g-truc/glm).