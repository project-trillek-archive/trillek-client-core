trillek-client
==============
How To Build The Engine (Windows instructions)
1. Download and install cmake http://www.cmake.org/
2. Download the latest version of the ending
3. Open cmake gui, and set the source to the root folder
4. Set build to the build folder
5. Check advanced
6. Download https://www.dropbox.com/s/8zc5u1rfj4lqj9m/trillek-win32-lib.zip
7. Put into root of engine folder
8. Click configure
9. Set to the 32 bit Visual Studio 2013 (will NOT compile in 64 bit)
9. Set all of the not found to their respective libraries in lib/x86
10. Make sure to set all files to the .lib, not the other file extensions(eg. BULLET_DYNAMICS_LIBRARY to BulletDynamics_Debug.lib
11. Make sure to set both GLEW and GLEW_DEBUG to glew32sd.lib
12. Check static glew
12. Click generate
13. Open the resulting .sln (found in /build) in Visual Studio
14. Build the projects (hit f5)
15. Download https://www.dropbox.com/s/55h0940ur8ss50f/Trillek-0.0.2.zip
15. Go to build/bin/debug and place the allure32.dll there
16. Download https://github.com/trillek-team/trillek-assets/tree/lua
17. Rename extracted file to assets, and place into /build/bin/debug
18. Run trillek-standalone in /build/bin/debug

If you follow these steps, you will have built the trillek engine
