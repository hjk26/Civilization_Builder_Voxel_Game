#include "InteractionManager.h"
#include <cmath>
#include "Core/ResourceManager.h"
#include <iostream> // For temporary console logging

bool InteractionManager::leftMousePressed = false;
bool InteractionManager::rightMousePressed = false;

RaycastResult InteractionManager::castRay(glm::vec3 start, glm::vec3 dir, World& world, float maxDistance) {
    RaycastResult result{};
    result.hit = false;

    dir = glm::normalize(dir);

    int x = static_cast<int>(std::floor(start.x));
    int y = static_cast<int>(std::floor(start.y));
    int z = static_cast<int>(std::floor(start.z));

    int stepX = (dir.x > 0) ? 1 : -1;
    int stepY = (dir.y > 0) ? 1 : -1;
    int stepZ = (dir.z > 0) ? 1 : -1;

    float tDeltaX = (dir.x == 0) ? 1e30f : std::fabs(1.0f / dir.x);
    float tDeltaY = (dir.y == 0) ? 1e30f : std::fabs(1.0f / dir.y);
    float tDeltaZ = (dir.z == 0) ? 1e30f : std::fabs(1.0f / dir.z);

    float tMaxX = (dir.x == 0) ? 1e30f : ((stepX > 0 ? std::floor(start.x) + 1.0f - start.x : start.x - std::floor(start.x)) * tDeltaX);
    float tMaxY = (dir.y == 0) ? 1e30f : ((stepY > 0 ? std::floor(start.y) + 1.0f - start.y : start.y - std::floor(start.y)) * tDeltaY);
    float tMaxZ = (dir.z == 0) ? 1e30f : ((stepZ > 0 ? std::floor(start.z) + 1.0f - start.z : start.z - std::floor(start.z)) * tDeltaZ);

    // Track which face was hit: 0 = X, 1 = Y, 2 = Z
    int hitSide = 0; 
    float traveled = 0.0f;

    while (traveled < maxDistance) {
        int currentBlock = world.getBlock(x, y, z);

        // MODIFIED: Only stop the ray if it hits a solid block.
        // It will completely ignore Air (0) and Water (2).
        if (currentBlock != 0 && currentBlock != 2) {
            result.hit = true;
            result.x = x;
            result.y = y;
            result.z = z;
            
            result.nx = x;
            result.ny = y;
            result.nz = z;

            // Shift neighbor target backward along the entry vector path
            if (hitSide == 0)      result.nx -= stepX;
            else if (hitSide == 1) result.ny -= stepY;
            else if (hitSide == 2) result.nz -= stepZ;

            return result;
        }

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                x += stepX;
                traveled = tMaxX;
                tMaxX += tDeltaX;
                hitSide = 0;
            } else {
                z += stepZ;
                traveled = tMaxZ;
                tMaxZ += tDeltaZ;
                hitSide = 2;
            }
        } else {
            if (tMaxY < tMaxZ) {
                y += stepY;
                traveled = tMaxY;
                tMaxY += tDeltaY;
                hitSide = 1;
            } else {
                z += stepZ;
                traveled = tMaxZ;
                tMaxZ += tDeltaZ;
                hitSide = 2;
            }
        }
    }
    return result;
}

void InteractionManager::handleMouseInteraction(GLFWwindow* window, World& world, glm::vec3 cameraPos, glm::vec3 cameraFront) {
    // Left Mouse Button -> Break Block (Set to Air / 0)
    // Inside InteractionManager::handleMouseInteraction (Left Click Block Breaking)
    // Inside InteractionManager::handleMouseInteraction (Left Click Block Breaking)
    // 1. Check if the left mouse button is physically held down
    // Left Mouse Button -> Break Block (Set to Air / 0)
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!leftMousePressed) { 
            RaycastResult res = castRay(cameraPos, cameraFront, world, 15.0f);
            if (res.hit) {
                // 1. Use the integer coordinates straight from the raycaster
                int minedBlockID = world.getBlock(res.x, res.y, res.z);

                if (minedBlockID != 0) { // Safety check: don't mine air
                    // 2. Erase the precise block that was struck
                    world.setBlock(res.x, res.y, res.z, 0); 
                    std::cout << "Successfully broke block ID: " << minedBlockID << " at (" << res.x << "," << res.y << "," << res.z << ")" << std::endl;
                    
                    // 3. Track resources accurately
                    if (minedBlockID == 5) { 
                        ResourceManager::addResource(ResourceType::STONE, 1); // Snow
                    } 
                    else if (minedBlockID == 2) { 
                        ResourceManager::addResource(ResourceType::WOOD, 1);  // Dirt
                    }
                    
                    ResourceManager::printStockpile();
                }
            }
            leftMousePressed = true; 
        }
    } else {
        leftMousePressed = false;
    }

    // Right Mouse Button -> Place Block (Set to Stone / 3)
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!rightMousePressed) {
            RaycastResult res = castRay(cameraPos, cameraFront, world, 15.0f);
            if (res.hit) {
                int playerX = static_cast<int>(std::floor(cameraPos.x));
                int playerY = static_cast<int>(std::floor(cameraPos.y));
                int playerZ = static_cast<int>(std::floor(cameraPos.z));

                // Block self-placement collision check
                if (!(res.nx == playerX && res.nz == playerZ && (res.ny == playerY || res.ny == playerY - 1))) {
                    world.setBlock(res.nx, res.ny, res.nz, 3); 
                }
            }
            rightMousePressed = true;
        }
    } else {
        rightMousePressed = false;
    }
}