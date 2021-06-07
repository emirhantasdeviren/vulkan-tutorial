@echo off

pushd build
C:\VulkanSDK\1.2.162.1\Bin\glslc.exe ..\src\shader\shader.vert -o vert.spv
C:\VulkanSDK\1.2.162.1\Bin\glslc.exe ..\src\shader\shader.frag -o frag.spv
popd
pause
