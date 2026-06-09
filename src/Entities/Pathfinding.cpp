#include "Pathfinding.h"
#include <algorithm>
#include <cmath>

// Helper to determine if a voxel space can physically fit a citizen
bool Pathfinding::isValidWalkableSpace(glm::ivec3 pos, World& world) {
    // A space is walkable if:
    // 1. The block at 'pos' is AIR (legs space)
    // 2. The block at 'pos + (0,1,0)' is AIR (head space)
    // 3. The block at 'pos - (0,1,0)' is SOLID (ground to stand on)
    
    bool legsAir     = (world.getBlock(pos.x, pos.y, pos.z) == 0);
    bool headAir     = (world.getBlock(pos.x, pos.y + 1, pos.z) == 0);
    bool groundSolid = (world.getBlock(pos.x, pos.y - 1, pos.z) != 0);

    return legsAir && headAir && groundSolid;
}

std::vector<glm::ivec3> Pathfinding::getNeighbors(glm::ivec3 pos, World& world) {
    std::vector<glm::ivec3> neighbors;

    // Check 4 horizontal directions: North, South, East, West
    glm::ivec3 directions[] = {
        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
    };

    for (const auto& dir : directions) {
        glm::ivec3 targetPos = pos + dir;

        // --- VOXEL MAGIC: Check flat walking, stepping up, and stepping down ---
        
        // 1. Flat Walk Check
        if (isValidWalkableSpace(targetPos, world)) {
            neighbors.push_back(targetPos);
        }
        // 2. Step Up Check (1 block higher)
        else if (isValidWalkableSpace(targetPos + glm::ivec3(0, 1, 0), world)) {
            neighbors.push_back(targetPos + glm::ivec3(0, 1, 0));
        }
        // 3. Step Down Check (1 block lower)
        else if (isValidWalkableSpace(targetPos + glm::ivec3(0, -1, 0), world)) {
            neighbors.push_back(targetPos + glm::ivec3(0, -1, 0));
        }
    }

    return neighbors;
}

float Pathfinding::getDistance(glm::ivec3 a, glm::ivec3 b) {
    // Standard 3D Euclidean distance heuristic
    return std::sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

std::vector<glm::vec3> Pathfinding::findPath(glm::vec3 start, glm::vec3 target, World& world) {
    glm::ivec3 startInt(std::floor(start.x), std::floor(start.y), std::floor(start.z));
    glm::ivec3 targetInt(std::floor(target.x), std::floor(target.y), std::floor(target.z));

    std::vector<PathNode> openList;
    std::vector<PathNode> closedList;

    PathNode startNode{ startInt, 0.0f, getDistance(startInt, targetInt), startInt };
    openList.push_back(startNode);

    bool pathFound = false;
    PathNode endNode;

    // Max loop iterations to prevent game freezing if target is unreachable
    int iterations = 0;
    const int MAX_ITERATIONS = 1000;

    while (!openList.empty() && iterations++ < MAX_ITERATIONS) {
        // Find node with lowest fCost in open list
        auto currentIt = openList.begin();
        for (auto it = openList.begin(); it != openList.end(); ++it) {
            if (it->fCost() < currentIt->fCost()) {
                currentIt = it;
            }
        }

        PathNode current = *currentIt;

        // If close enough to target, reconstruct path!
        if (getDistance(current.position, targetInt) <= 1.5f) {
            endNode = current;
            pathFound = true;
            break;
        }

        openList.erase(currentIt);
        closedList.push_back(current);

        // Evaluate neighbors
        std::vector<glm::ivec3> neighbors = getNeighbors(current.position, world);
        for (const auto& neighborPos : neighbors) {
            
            // Skip if neighbor is already evaluated
            auto inClosed = std::find_if(closedList.begin(), closedList.end(), 
                [&](const PathNode& n) { return n.position == neighborPos; });
            if (inClosed != closedList.end()) continue;

            float newGCost = current.gCost + getDistance(current.position, neighborPos);

            auto inOpen = std::find_if(openList.begin(), openList.end(), 
                [&](const PathNode& n) { return n.position == neighborPos; });

            if (inOpen == openList.end()) {
                // New node discovered
                PathNode neighborNode{ neighborPos, newGCost, getDistance(neighborPos, targetInt), current.position };
                openList.push_back(neighborNode);
            } else if (newGCost < inOpen->gCost) {
                // Found a better path to an existing open node
                inOpen->gCost = newGCost;
                inOpen->parentPos = current.position;
            }
        }
    }

    // Retrace path from end to start if successful
    std::vector<glm::vec3> finalPath;
    if (pathFound) {
        PathNode curr = endNode;
        while (!(curr.position == startInt)) {
            // Push actual float world positions centered inside the voxel column
            finalPath.push_back(glm::vec3(curr.position.x + 0.5f, curr.position.y, curr.position.z + 0.5f));
            
            // Find parent node in closed list to step backward
            auto parentIt = std::find_if(closedList.begin(), closedList.end(), 
                [&](const PathNode& n) { return n.position == curr.parentPos; });
            
            if (parentIt != closedList.end()) {
                curr = *parentIt;
            } else {
                break; // Break if parent tracking breaks
            }
        }
        std::reverse(finalPath.begin(), finalPath.end());
    }

    return finalPath;
}