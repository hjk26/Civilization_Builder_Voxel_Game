#include "ZoneManager.h"
#include <cstdlib>
#include <iostream>

std::vector<Zone> ZoneManager::activeZones;
unsigned int ZoneManager::nextZoneId = 1;

void ZoneManager::createZone(std::string name, ZoneType type, glm::ivec3 minB, glm::ivec3 maxB) {
    Zone newZone;
    newZone.id = nextZoneId++;
    newZone.name = name;
    newZone.type = type;
    
    // Ensure min bounds are actually smaller than max bounds
    newZone.minBounds = glm::min(minB, maxB);
    newZone.maxBounds = glm::max(minB, maxB);

    activeZones.push_back(newZone);
    std::cout << "🗺️ Zone Created: " << name << " [" << (maxB.x - minB.x + 1) << "x" << (maxB.z - minB.z + 1) << " Voxel Area]" << std::endl;
}

int ZoneManager::findZoneAt(glm::ivec3 pos) {
    for (const auto& zone : activeZones) {
        if (zone.contains(pos)) return zone.id;
    }
    return -1; // Unzoned wilderness
}

glm::ivec3 ZoneManager::getRandomVoxelInZone(unsigned int zoneId) {
    for (const auto& zone : activeZones) {
        if (zone.id == zoneId) {
            int rx = zone.minBounds.x + (std::rand() % (zone.maxBounds.x - zone.minBounds.x + 1));
            int ry = zone.minBounds.y + (std::rand() % (zone.maxBounds.y - zone.minBounds.y + 1));
            int rz = zone.minBounds.z + (std::rand() % (zone.maxBounds.z - zone.minBounds.z + 1));
            return glm::ivec3(rx, ry, rz);
        }
    }
    return glm::ivec3(0);
}

glm::ivec3 ZoneManager::getNearestZoneCenter(glm::vec3 currentPos, ZoneType type, unsigned int factionId) {
    glm::ivec3 absoluteCenter(-1);
    float minDistance = 99999.0f;

    for (const auto& zone : activeZones) {
        if (zone.type == type && zone.owningFactionId == factionId) { // Check ownership!
            glm::ivec3 center = (zone.minBounds + zone.maxBounds) / 2;
            float dist = glm::distance(currentPos, glm::vec3(center));
            if (dist < minDistance) {
                minDistance = dist;
                absoluteCenter = center;
            }
        }
    }
    return absoluteCenter;
}