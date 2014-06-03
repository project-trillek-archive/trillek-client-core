# PowerShell script to install all dependencies 

new-item lib -type directory
new-item lib\x86 -type directory
new-item lib\x86\Debug -type directory
new-item lib\include -type directory
new-item build -type directory

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

# Get and build GTest
Start-FileDownload 'http://googletest.googlecode.com/files/gtest-1.7.0.zip'
7z -y x gtest-1.7.0.zip
pushd
Set-Location gtest-1.7.0
cmake -G "Visual Studio 12" .
msbuild gtest.sln /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
popd
move-item -path gtest-1.7.0\include\* -destination lib/include
[Environment]::SetEnvironmentVariable("GTEST_ROOT", "C:\projects\trillek-client-core\gtest-1.7.0", "Machine")