@echo off
g++ src/*.cpp src/Core/*.cpp src/Entities/*.cpp src/CivSystems/*.cpp src/Industry/*.cpp src/gl.c -I deps/include -L deps/lib -lglfw3 -lgdi32 -lopengl32 -luser32 -lshell32 -o engine.exe 
if %errorlevel% neq 0 (
    echo Compilation Failed!
    pause
    exit /b %errorlevel%
)
echo Build Successful! Running...
engine.exe