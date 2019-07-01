# Shatter Engine™
This is a small game engine framework that is used to power some of my personal projects.

![Lofty Lagoon](aicritters-example.png)

# Current Features
Here's a short list of things that are currently possible in the engine.

* Basic asset management for models, shaders, textures and audio
* Import and export of custom engine data such as binary model files
* Render passes
* Loading levels from script files
* Loading sub-levels from level script files
* Entity messaging

# Installation
Make sure you create an EngineMacros.props file for your game projects with a User Macro named EnginePath that points at the location of the engine solution file.

# Third Party
The engine requires a couple of third party libraries which aren't available in this repository.

* ThirdParty/glad/ - GLAD for OpenGL 4.3 Core with the KHR debug extension
* ThirdParty/glfw-3.2.1.bin.WIN64/ - GLFW 3.2.1
* ThirdParty/glm/ - GLM 0.9.8.5
* ThirdParty/imgui-1.70/ - dear imgui 1.70
* ThirdParty/SFML-2.5.1/ - SFML 2.5.1 (for audio)
* ThirdParty/stb/ - STB
* ThirdParty/discord-rpc/ - Discord Rich Presence
