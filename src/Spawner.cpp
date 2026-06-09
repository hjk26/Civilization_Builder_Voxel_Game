#include "Spawner.h"
#include <cmath>

glm::vec3 Spawner::findSafeSpawn(int targetX, int targetZ, World& world) {
    int surfaceY = 64; // Default starting guess in case columns fail

    // Start looking from maximum project height parameters downward
    for (int y = 127; y >= 0; y--) {
        unsigned char block = world.getBlock(targetX, y, targetZ);
        
        // If we hit solid ground or sand blocks (not air or water fluid)
        if (block != 0 && block != 6) {
            surfaceY = y;
            break;
        }
        // If we hit water, let the player spawn floating cleanly right on the sea surface
        if (block == 6) {
            surfaceY = y + 2; 
            break;
        }
    }

    // Return the calculated vector adjusting for player camera height placement (2.5 blocks)
    return glm::vec3(targetX + 0.5f, (float)surfaceY + 3.0f, targetZ + 0.5f);
}