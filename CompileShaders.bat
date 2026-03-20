@echo off
set GLSLC=D:/VulkanSDK/1.4.335.0/Bin/glslc.exe

%GLSLC% SandBox\assets\shaders\shader.vert -o SandBox\assets\shaders\shader.vert.spv
%GLSLC% SandBox\assets\shaders\shader.frag -o SandBox\assets\shaders\shader.frag.spv

echo Shaders compiled successfully!
pause