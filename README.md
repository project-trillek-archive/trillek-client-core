trillek-client
==============

Contributing
===

The easiest way to contribute is to fork, experiment, and make PRs.

There are 3 domains where contribution is needed:

* Game logic
* Engine
* Artistic assets

The game logic is a lot more dev-friendly than the engine itself and needs less experience. 
We put some efforts to create an abstraction layer for the game logic that allows
 to create easily some features.

Before starting, you must read these docs:

* [Coding standards and practices](https://github.com/trillek-team/trillek-client-core/wiki/Coding-Standards).
* [ECS framework](https://github.com/trillek-team/trillek-client-core/wiki/ECS-State-System)

If you don't know where to put your game logic code, use [physics.cpp](https://github.com/trillek-team/trillek-client-core/blob/develop/src/systems/physics.cpp#L31)
as a sandbox. There are also some sample codes here you should see. We will move them in a right location later.

The dev community is mainly on IRC : #project-trillek on Freenode.


Installing
===

To build Trillek from source you must have installed [CMake](http://www.cmake.org/) and one of the following compilers:

* GCC 4.8 or newer;
* Clang 3.4 or newer (Xcode 5.1 or newer on OS X);
* Visual Studio 2010 or newer;

Nightly binary distributions are not yet available.

Dependencies
------------
Trillek depends on the following libraries:

- [GLFW3](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [Bullet Physics](https://github.com/bulletphysics/bullet3) (bullet with double precision)
- [GLEW](https://github.com/nigels-com/glew)
- [OpenAL](http://kcat.strangesoft.net/openal.html) (with OggVorbis support)
- [Alure](http://kcat.strangesoft.net/alure.html)
- [RapidJSON](https://github.com/miloyip/rapidjson)
- [Lua](http://www.lua.org/download.html)
- [LuaWrapper](https://bitbucket.org/alexames/luawrapper/src)

Getting The Code
----------------

To retrieve a copy of the Trillek code and assets, clone the repository and its submodules.

	git clone https://github.com/trillek-team/trillek-client-core.git
	cd trillek-client-core
	git submodule update --init

Avoid using the **Download Zip** option on GitHub.  This will *not* download the [assets](https://github.com/trillek-team/trillek-assets) submodule.

Building
--------

Platform specific build instructions are available on the Wiki.

[Building on Windows](https://github.com/trillek-team/trillek-client-core/wiki/Building-on-Windows)

[Building on OS X](https://github.com/trillek-team/trillek-client-core/wiki/Building-On-OS-X)

On Linux, you can get some inspiration by checking the [travis-ci script](https://github.com/trillek-team/trillek-client-core/blob/develop/.travis.yml)
that builds the client from scratch.
