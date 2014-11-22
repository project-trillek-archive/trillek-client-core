# PowerShell script to install all dependencies 

new-item lib -type directory
new-item lib\x86 -type directory
new-item lib\x86\Release -type directory
new-item lib\include -type directory
new-item build -type directory

# Get Raknet
git submodule update --init --depth 1 -- .\raknet

# Get Crypto++
git submodule update --init --depth 1 -- .\crypto++

# Get GLEW
Start-FileDownload 'http://softlayer-ams.dl.sourceforge.net/project/glew/glew/1.10.0/glew-1.10.0-win32.zip'
7z -y x glew-1.10.0-win32.zip
move-item -path glew-1.10.0\lib\Release\Win32\*.lib -destination lib/x86/Debug
move-item -Path glew-1.10.0\include\* -destination lib/include

# Get GLFW
Start-FileDownload 'http://skylink.dl.sourceforge.net/project/glfw/glfw/3.0.4/glfw-3.0.4.bin.WIN32.zip'
7z -y x glfw-3.0.4.bin.WIN32.zip
move-item -path glfw-3.0.4.bin.WIN32\lib-msvc120\*.lib -destination lib/x86/Debug
move-item -path glfw-3.0.4.bin.WIN32\include\* -destination lib/include

# Get RapidJSON
Start-FileDownload 'http://rapidjson.googlecode.com/files/rapidjson-0.11.zip'
7z -y x rapidjson-0.11.zip
move-item -path rapidjson\include\* -destination lib/include

# Get GLM
Start-FileDownload 'http://heanet.dl.sourceforge.net/project/ogl-math/glm-0.9.5.3/glm-0.9.5.3.7z'
7z -y x glm-0.9.5.3.7z
move-item -path glm\* -destination lib/include

# Get Bullet
Start-FileDownload 'https://bullet.googlecode.com/files/bullet-2.82-r2704.zip'
7z -y x bullet-2.82-r2704.zip
pushd
Set-Location bullet-2.82-r2704
cmake -G "Visual Studio 12" -D CMAKE_BUILD_TYPE:String=Release -DINSTALL_LIBS=on -DBUILD_SHARED_LIBS=on -DBUILD_DEMOS=off -DUSE_DOUBLE_PRECISION=on  .
msbuild BULLET_PHYSICS.sln /p:Configuration=Release /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
Get-ChildItem lib/Release/*
popd
move-item -path bullet-2.82-r2704\lib\Release\include\* -destination lib/include
move-item -path bullet-2.82-r2704\lib\Release\*.lib -destination lib/x86/Release
move-item -path bullet-2.82-r2704\lib\Release\*.dll -destination lib/x86/Release

# Get and build GTest
Start-FileDownload 'http://googletest.googlecode.com/files/gtest-1.7.0.zip'
7z -y x gtest-1.7.0.zip
pushd
Set-Location gtest-1.7.0
cmake -G "Visual Studio 12" -D CMAKE_BUILD_TYPE:String=Release .
msbuild gtest.sln /p:Configuration=Release /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
popd
move-item -path gtest-1.7.0\include\* -destination lib/include
move-item -path gtest-1.7.0\Release\*.lib -destination lib/x86/Release
move-item -path gtest-1.7.0\Release\*.dll -destination lib/x86/Release
Get-ChildItem lib/x86/Release/*
