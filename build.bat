@echo off
g++ src/main.cpp src/gl.c src/Terrain.cpp src/Chunk.cpp src/InputHandler.cpp src/Spawner.cpp src/Minimap.cpp src/InteractionManager.cpp -o engine.exe -I deps/include -L deps/lib -lglfw3 -lgdi32 -lopengl32
if %errorlevel% neq 0 (
    echo Compilation Failed!
    pause
    exit /b %errorlevel%
)
echo Build Successful! Running...
engine.exe