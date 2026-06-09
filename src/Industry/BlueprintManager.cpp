#include "BlueprintManager.h"
#include "../Core/ResourceManager.h"
#include <iostream>

std::map<std::string, Blueprint> BlueprintManager::schematicRegistry;

void BlueprintManager::initializeRegistry() {
    schematicRegistry["AutoMiner"]   = { "Auto-Miner Engine", MachineType::AUTO_MINER, 20, 40 };
    schematicRegistry["BlastFurnace"] = { "Blast Furnace Array", MachineType::BLAST_FURNACE, 10, 50 };
    std::cout << "📜 Industrial Schematics Cached in Memory." << std::endl;
}

bool BlueprintManager::manufactureMachine(std::string name, glm::ivec3 position, unsigned int factionId) {
    if (schematicRegistry.find(name) == schematicRegistry.end()) return false;

    Blueprint bp = schematicRegistry[name];

    // Verify materials from Kingdom Stockpile (Assuming faction 1 for player-owned resources)
    if (ResourceManager::spendResource(ResourceType::STONE, bp.requiredStone)) {
        // Materials spent successfully! Register inside structural Machine Graph
        MachineGraph::registerMachine(bp.productType, position, factionId);
        return true;
    }

    std::cout << "❌ Construction Failed: Insufficient resources for " << bp.machineName << std::endl;
    return false;
}