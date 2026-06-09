#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <unordered_map>
#include "Terrain.h"
#include "Chunk.h"
#include "InputHandler.h"
#include "Spawner.h"
#include "Minimap.h"
#include "InteractionManager.h"
#include "Entities/EntityManager.h"
#include <cstdlib>
#include <ctime>
#include "Core/TimeManager.h"
#include "CivSystems/ZoneManager.h"
#include "CivSystems/FactionManager.h"
#include "Industry/BlueprintManager.h"
#include "Industry/MachineGraph.h"
#include "Industry/ChronicleDatabase.h"
TerrainSystem terrainSystem;


unsigned int selectionVAO, selectionVBO;
int selectionVertexCount = 0;
// --- Shaders ---
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n" // <--- Change vec3 to vec4 here
    "out vec4 ourColor;\n"                     // <--- Change vec3 to vec4 here
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "uniform vec3 overrideColor;\n"
    "void main() {\n"
    "   if (overrideColor.x >= 0.0f) {\n"
    "       FragColor = vec4(overrideColor, 1.0);\n" // Render the highlight wireframe
    "   } else {\n"
    "       FragColor = ourColor;\n"               // Render standard blocks
    "   }\n"
    "}\0";


void initSelectionBox()
{
    float cubeLines[] = {
        // bottom square
        0,0,0,  1,0,0,
        1,0,0,  1,0,1,
        1,0,1,  0,0,1,
        0,0,1,  0,0,0,

        // top square
        0,1,0,  1,1,0,
        1,1,0,  1,1,1,
        1,1,1,  0,1,1,
        0,1,1,  0,1,0,

            // vertical edges
        0,0,0,  0,1,0,
        1,0,0,  1,1,0,
        1,0,1,  1,1,1,
        0,0,1,  0,1,1
    };

    selectionVertexCount = 24;

    glGenVertexArrays(1, &selectionVAO);
    glGenBuffers(1, &selectionVBO);

    glBindVertexArray(selectionVAO);
    glBindBuffer(GL_ARRAY_BUFFER, selectionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeLines), cubeLines, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}


/**struct RaycastResult {
    bool hit;
    int x, y, z;      // The block that was hit
    int nx, ny, nz;   // The empty space next to the hit face (for placing)
    int fx, fy, fz;
};

RaycastResult castRay(glm::vec3 start, glm::vec3 dir, World& world)
{
    RaycastResult result{};
    result.hit = false;
    int faceX = 0, faceY = 0, faceZ = 0;

    // Current voxel the ray is in
    int x = (int)floor(start.x);
    int y = (int)floor(start.y);
    int z = (int)floor(start.z);

    // Direction steps
    int stepX = (dir.x > 0) ? 1 : -1;
    int stepY = (dir.y > 0) ? 1 : -1;
    int stepZ = (dir.z > 0) ? 1 : -1;

    // Avoid divide-by-zero
    float tDeltaX = (dir.x == 0) ? 1e30f : fabs(1.0f / dir.x);
    float tDeltaY = (dir.y == 0) ? 1e30f : fabs(1.0f / dir.y);
    float tDeltaZ = (dir.z == 0) ? 1e30f : fabs(1.0f / dir.z);

    // Distance to first voxel boundary
    float tMaxX, tMaxY, tMaxZ;

    auto voxelBoundary = [](float s, int step) {
        if (step > 0)
            return (floor(s) + 1.0f);
        else
            return (floor(s));
    };

    tMaxX = (dir.x == 0) ? 1e30f :
        (voxelBoundary(start.x, stepX) - start.x) / dir.x;

    tMaxY = (dir.y == 0) ? 1e30f :
        (voxelBoundary(start.y, stepY) - start.y) / dir.y;

    tMaxZ = (dir.z == 0) ? 1e30f :
        (voxelBoundary(start.z, stepZ) - start.z) / dir.z;

    int lastX = x, lastY = y, lastZ = z;

    float maxDistance = 5.0f;
    float traveled = 0.0f;

    while (traveled < maxDistance)
    {
        // Check block
        if (world.getBlock(x, y, z) != 0)
        {
            result.hit = true;
            result.x = x;
            result.y = y;
            result.z = z;

            // Previous empty position (for placement)
            result.nx = lastX;
            result.ny = lastY;
            result.nz = lastZ;

            result.fx = faceX;
            result.fy = faceY;
            result.fz = faceZ;

            return result;
        }

        // Step to next voxel boundary
        lastX = x;
        lastY = y;
        lastZ = z;

        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                x += stepX;
                traveled = tMaxX;
                tMaxX += tDeltaX;
                faceX = -stepX;
                faceY = 0;
                faceZ = 0;
            }
            else
            {
                z += stepZ;
                traveled = tMaxZ;
                tMaxZ += tDeltaZ;
            }
        }
        else
        {
            if (tMaxY < tMaxZ)
            {
                y += stepY;
                traveled = tMaxY;
                tMaxY += tDeltaY;
                faceX = 0;
                faceY = -stepY;
                faceZ = 0;
            }
            else
            {
                z += stepZ;
                traveled = tMaxZ;
                tMaxZ += tDeltaZ;
                faceX = 0;
                faceY = 0;
                faceZ = -stepZ;
            }
        }
    }

    return result;
}

void renderSelectionBox(unsigned int modelLoc, glm::vec3 blockPos)
{
    glm::mat4 model = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(blockPos.x, blockPos.y, blockPos.z)
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(selectionVAO);
    glDrawArrays(GL_LINES, 0, selectionVertexCount);
}**/


void drawBlockHighlight(unsigned int modelLoc, glm::vec3 pos)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    model = glm::scale(model, glm::vec3(1.02f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Disable Attribute 1 safely so it doesn't read junk color data for the lines
    glDisableVertexAttribArray(1); 

    glBindVertexArray(selectionVAO);
    glDrawArrays(GL_LINES, 0, selectionVertexCount);

    // Re-enable it immediately for your chunks on the next frame pass
    glEnableVertexAttribArray(1); 
}

void initSelectionBox();
void drawBlockHighlight(unsigned int modelLoc, glm::vec3 pos);

int w=800, h=600;
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    w = width;
    h = height;

    if (h == 0) h = 1; // prevent crash

    glViewport(0, 0, width, height);
}

bool leftMousePressed = false;
bool rightMousePressed = false;

// --- Main ---
int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800,600,"Voxel Engine",0,0);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwPollEvents(); // forces GLFW to update size
    glfwGetFramebufferSize(window, &w, &h);

    if (!window) { glfwTerminate(); return -1; }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        InputHandler::mouseCallback(w, x, y);
    });
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    initSelectionBox();
    Minimap::initialize();
    EntityManager::initializeRenderer();

    // 1. Establish factions
    FactionManager::createFaction("Kingdom of Red", glm::vec3(0.8f, 0.1f, 0.1f));
    FactionManager::createFaction("Tribes of Blue", glm::vec3(0.1f, 0.1f, 0.8f));

    // 2. Build local infrastructure around the actual citizen spawn coordinates
    ZoneManager::createZone("Local Stockpile", ZoneType::STOCKPILE, glm::ivec3(-185, 40, 120), glm::ivec3(-165, 60, 140));
    ZoneManager::activeZones.back().owningFactionId = 1; // Assign to Kingdom of Red

    ZoneManager::createZone("Local Stone Trench", ZoneType::MINING_DISTRICT, glm::ivec3(-190, 40, 110), glm::ivec3(-160, 60, 150));
    ZoneManager::activeZones.back().owningFactionId = 1;

    BlueprintManager::initializeRegistry();

    // Set up an automated mining assembly line near the settlement trench
    unsigned int minerId = MachineGraph::registerMachine(MachineType::AUTO_MINER, glm::ivec3(-170, 48, 115), 1);
    unsigned int furnaceId = MachineGraph::registerMachine(MachineType::BLAST_FURNACE, glm::ivec3(-170, 48, 120), 1);

    // Wire them together so the miner automatically sends items to the furnace!
    MachineGraph::connectMachines(minerId, furnaceId);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs); glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    terrainSystem.initialize(12345); // Any integer seed

    World myWorld;
    for(int x=0;x<10;x++)
    {
        for(int y=0;y<8;y++)
        {
            for(int z=0;z<10;z++)
            {
                ChunkPos pos{x,y,z};
                myWorld.chunks[pos] = new Chunk(x,y,z,&myWorld);
            }
        }
    }

    // Populate meshes
    for (auto& pair : myWorld.chunks) {
        pair.second->generateMesh();
    }

    // FLAWLESS SPAWNING RUNS HERE: Chunk blocks exist in memory now!
    glm::vec3 safeLocation = Spawner::findSafeSpawn(100, 100, myWorld);
    InputHandler::initialize(safeLocation.x, safeLocation.y, safeLocation.z);

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int overrideColorLoc = glGetUniformLocation(shaderProgram, "overrideColor");
    
    // Core loop frame-timing definitions
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // 1. Process Input, Gravity, and Movement via our New Modulated Class
        InputHandler::processKeyboard(window, myWorld, deltaTime);

        Minimap::checkInput(window);

        static bool pKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            if (!pKeyWasPressed) {
                EntityManager::spawnCitizen(InputHandler::cameraPos);
                pKeyWasPressed = true;
            }
        } else {
            pKeyWasPressed = false;
        }

        // Pressing 'O' prints out the detailed financial/survival roster to your terminal
        static bool oKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            if (!oKeyWasPressed) {
                EntityManager::debugPrintSettlement();
                oKeyWasPressed = true;
            }
        } else {
            oKeyWasPressed = false;
        }
        // Update citizen simulation states, pathfinding tracking, and metabolic rates
        EntityManager::updateEntities(deltaTime, myWorld);
        // Update industry conveyor nodes every frame
        MachineGraph::updateNetwork(deltaTime);

        // Dynamic Chunk Loading Check
        int playerChunkX = floor(InputHandler::cameraPos.x / 16.0f);
        int playerChunkZ = floor(InputHandler::cameraPos.z / 16.0f);
        const int renderDistance = 4;

        for(int dx=-renderDistance; dx<=renderDistance; dx++) {
            for(int dz=-renderDistance; dz<=renderDistance; dz++) {
                for(int dy=0; dy<8; dy++) {
                    ChunkPos pos { playerChunkX + dx, dy, playerChunkZ + dz };

                    if(myWorld.chunks.find(pos) == myWorld.chunks.end()) {
                        myWorld.chunks[pos] = new Chunk(pos.x, pos.y, pos.z, &myWorld);
                        myWorld.chunks[pos]->generateMesh();
                    }
                }
            }
        }

        for (auto it = myWorld.chunks.begin(); it != myWorld.chunks.end(); ) {
            Chunk* c = it->second;
            
            // Calculate distance in chunk-space instead of block-space for efficiency
            int dx = c->chunkX - playerChunkX;
            int dz = c->chunkZ - playerChunkZ;
            
            // If the chunk is outside our render distance (plus a buffer of 2 chunks to avoid 
            // constantly loading/unloading chunks right on the border), delete it.
            if (abs(dx) > (renderDistance + 2) || abs(dz) > (renderDistance + 2)) {
                delete c;                  // Free GPU/CPU memory (calls Chunk destructor)
                it = myWorld.chunks.erase(it); // Remove from our map and safely grab next iterator
            } else {
                ++it; // Move to next chunk normally
            }
        }

        // Update the global time states
        TimeManager::update(deltaTime);

        // Dynamically color-mix the atmosphere based on the game hour
        glm::vec3 skyColor = TimeManager::getDynamicSkyColor();
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Block Destruction/Placement Mechanics
        // 2. Block Destruction/Placement Mechanics managed beautifully externally
        // Change this inside your main rendering loop:
        InteractionManager::handleMouseInteraction(window, myWorld, InputHandler::cameraPos, InputHandler::cameraFront);

        // 3. Matrix View Setup and Rendering
        glUseProgram(shaderProgram);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)w / (float)h,0.1f,500.0f);
        glm::mat4 view = glm::lookAt(InputHandler::cameraPos, InputHandler::cameraPos + InputHandler::cameraFront, InputHandler::cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (auto& pair : myWorld.chunks) {
            Chunk* c = pair.second;
            if (!c) continue;

            glm::vec3 chunkPos(c->chunkX * 16 + 8, c->chunkY * 16 + 8, c->chunkZ * 16 + 8);

            // Frustum/Distance culling check using isolated positions
            if (glm::distance(InputHandler::cameraPos, chunkPos) < 150.0f) {
                c->render(modelLoc);
            }
        }

        EntityManager::renderEntities(shaderProgram, modelLoc, overrideColorLoc);

        glDisable(GL_CULL_FACE); 
        for (auto& pair : myWorld.chunks) {
            Chunk* c = pair.second;
            if (!c) continue;
            glm::vec3 chunkPos(c->chunkX * 16 + 8, c->chunkY * 16 + 8, c->chunkZ * 16 + 8);
            if (glm::distance(InputHandler::cameraPos, chunkPos) < 150.0f) {
                c->renderWater(modelLoc); // Declared in header now!
            }
        }
        glEnable(GL_CULL_FACE);

        // 4. Highlight Selected Looking Block Target 
        // 4. Highlight Selected Looking Block Target 
        // Pass a larger maxDistance (like 15.0f) to let the ray reach deeper water beds
        RaycastResult res = InteractionManager::castRay(InputHandler::cameraPos, InputHandler::cameraFront, myWorld, 15.0f);
        if (res.hit) {
            glUseProgram(shaderProgram);
            glUniform3f(glGetUniformLocation(shaderProgram, "overrideColor"), 1.0f, 1.0f, 0.0f);
            
            // Coordinates match up with your block vertices perfectly now
            // Update this line inside section "4. Highlight Selected Looking Block Target"
            drawBlockHighlight(modelLoc, glm::vec3(res.x - 0.5f, res.y - 0.5f, res.z - 0.5f));
            glUniform3f(glGetUniformLocation(shaderProgram, "overrideColor"), -1.0f, -1.0f, -1.0f);
        } else {
            glUseProgram(shaderProgram);
            glUniform3f(glGetUniformLocation(shaderProgram, "overrideColor"), -1.0f, -1.0f, -1.0f);
        }
        Minimap::updateAndRender(InputHandler::cameraPos, w, h);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Inside main.cpp right before closing the app / terminating window context
    std::cout << "Window execution terminated by user." << std::endl;
    ChronicleDatabase::dumpChroniclesToConsole();
    
    Minimap::cleanup();
    glfwTerminate();
    return 0;
}