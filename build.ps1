cmake -G "Visual Studio 12" .
msbuild Trillek_Client.vcxproj /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
cd tests
cmake -G "Visual Studio 12" .
msbuild TCCTests.vcxproj /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"