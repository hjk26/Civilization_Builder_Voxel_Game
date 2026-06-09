#ifndef BLUEPRINT_MANAGER_H
#define BLUEPRINT_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "MachineGraph.h"

struct Blueprint {
    std::string machineName;
    MachineType productType;
    int requiredWood = 0;
    int requiredStone = 0;
};

class BlueprintManager {
public:
    static std::map<std::string, Blueprint> schematicRegistry;

    static void initializeRegistry();
    static bool manufactureMachine(std::string name, glm::ivec3 position, unsigned int factionId);
};

#endif // BLUEPRINT_MANAGER_H