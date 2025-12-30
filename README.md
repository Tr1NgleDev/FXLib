# FXLib    [BETA]

A [4D-Modding](https://4d-modding.com/) library for various graphics effects and optimizations using OpenGL 4.5 Core.  

-----------------

## Features:
- Particle System
- Trails
- Post-Processing Passes
- Shader Storage Buffers
- Texture Buffers
- Instanced Mesh Rendering
- Shader Patching

-----------------

## Usage:
- Download the latest release of `fxlib-dev` and unzip it somewhere in your mod's project (For example: `ProjectFolder/fxlib-dev/LICENSE, FXLib.lib, include/, ...`)
- Link the `FXLib.lib` file in your mod project (In `Project Properties -> Linker -> Input -> Additional Dependencies`)
- Add the `include` folder in your mod project (In `Project Properties -> C/C++ -> General -> Additional Include Directories`)

`ParticleSystem` and `PostPass` usage examples can be found in [test.cpp](./test.cpp) at the moment.  
