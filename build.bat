@echo off

mkdir build
pushd build
cl -Zi ..\src\main.c /I C:\VulkanSDK\1.2.162.1\Include\ user32.lib
popd
