trillek-client
==============

Contributing
===

The easiest way to contribute is to fork and experiment.  Coding standards and practices can be found [here](https://github.com/trillek-team/trillek-client-core/wiki/Coding-Standards).

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
- [Bullet Physics](https://github.com/bulletphysics/bullet3) (builet with double precision)
- [GLEW](https://github.com/nigels-com/glew)
- [OpenAL](http://kcat.strangesoft.net/openal.html) (with OggVoribis support)
- [Alure](http://kcat.strangesoft.net/alure.html)
- [RapidJSON](https://github.com/miloyip/rapidjson)

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
