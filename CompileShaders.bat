@echo off
set GLSLC=D:/VulkanSDK/1.4.335.0/Bin/glslc.exe
set SHADER_PATH=SandBox\assets\shaders

echo ========================================
echo Compiling shaders...
echo ========================================
echo.

:: Компиляция всех вершинных шейдеров
echo [Vertex Shaders]
for %%f in (%SHADER_PATH%\*.vert) do (
    echo   Compiling: %%~nxf
    %GLSLC% "%%f" -o "%%f.spv"
    if errorlevel 1 (
        echo   [ERROR] Failed to compile %%~nxf
        pause
        exit /b 1
    )
)

echo.

:: Компиляция всех фрагментных шейдеров
echo [Fragment Shaders]
for %%f in (%SHADER_PATH%\*.frag) do (
    echo   Compiling: %%~nxf
    %GLSLC% "%%f" -o "%%f.spv"
    if errorlevel 1 (
        echo   [ERROR] Failed to compile %%~nxf
        pause
        exit /b 1
    )
)

echo.
echo ========================================
echo Shaders compiled successfully!
echo ========================================
pause