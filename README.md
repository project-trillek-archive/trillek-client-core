Feature - Core Infrastructure
==============

*DO NOT MERGE - DO NOT MERGE - DO NOT MERGE*

This branch lays the groundwork for the future growth of the TCC codebase by providing some core infrastructure for the engine and its client.  It also sets the standard for how the codebase will be organized in the future.

## Codebase Organization

After considering the issue further, I've decided to continue pressing for a grouped directory structure.  This will allow us to logically organize the code into modules.  Each module will have a root include that brings most, or all, of that modules *public* functionality into the current translation unit.  An example directory structure is shown below some of which is present in this branch:

```
Engine
    |-Core
    |    |- Core.h
    |    |- Platform
    |    |    |- Platform.h
    |    |    |- PlatformInclude.h
    |    |- PlatformBase
    |    |- PlatformMac
    |    |- (Other platform folders)
    |    |- Container
    |    |- Math
    |    |- Misc
    |    |    |- Build.h
    |    |- ...
    |-Renderer
    |    |- Renderer.h
    |    |- ...
    |-...
```

### Engine

Parent of all the modules which live in the engine.  Even though we're not building a generic engine all engine specific code should go here.

#### Core

Files pertinent to all parts of the engine.  This module depends on no other and most other modules will depend on it.  Contains platform and compiler abstractions, includes most of the system headers needed by the rest of the engine.  Custom containers and reusable building blocks should be here.

* Misc/Build.h - Build options only.  Everything in this file is meant to be overridden by the build settings.

* Platform/Platform.h - Platform detection and compiler feature/intrinsic abstraction.  This file includes the platform header for each platform.

* Platform/PlatformIncludes.h - Most of the system header includes for the Core should be here or in a <Platform>PlatfromIncludes.h file in the specific platform's directory.

* Math - Any math related types or functions should be placed here.

Add other folders for collections of files that are logically related and provide functionality that should live in the core.  If you're adding a one-off file, it should probably go in `Misc`.

Coalesce includes into `PlatformInclude.h`, `<Platform>PlatformInclude.h` or `Core.h`.  I'm not sure yet how core-level dependency includes (ex. GLM) should be handled.  Probably best to add them to `Core.h`.

## Cmake Changes

Each module directory has a CMakeLists.txt which exports variables containing the following to the parent CMakeLists.

* Include Directories - Directories to be added to the include path.  This should include most sub-directories present in each module except for those that contain platform specific code.

* Includes - All header files in the module.  Do not include platform specific headers unless we're building on the matching platform.

* Sources - All source files in the module.  Do not include platform specific sources unless we're building on the matching platform.

See Engine/Core/CMakeLists.txt for a template.

When building for an IDE, the root CMakeLists will group all the files to match their on-disk structure.  This happens automatically.

### Questions

#### Why Add all the directories in each module to the Include Path?

Because we couldn't have duplicate header names anyway.


>>>>>>> Begin Core Infrastructure Feature
