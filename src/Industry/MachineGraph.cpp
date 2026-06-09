#include "MachineGraph.h"
#include "../Core/ResourceManager.h"
#include "ChronicleDatabase.h"
#include <iostream>

std::map<unsigned int, MachineNode> MachineGraph::networkNodes;
unsigned int MachineGraph::nextMachineId = 1;

unsigned int MachineGraph::registerMachine(MachineType type, glm::ivec3 pos, unsigned int factionId) {
    MachineNode node;
    node.id = nextMachineId++;
    node.type = type;
    node.worldPosition = pos;
    node.owningFactionId = factionId;

    networkNodes[node.id] = node;
    std::cout << "⚙️ Machine Constructed: ID " << node.id << " at (" 
              << pos.x << "," << pos.y << "," << pos.z << ")" << std::endl;
    return node.id;
}

void MachineGraph::connectMachines(unsigned int sourceId, unsigned int targetId) {
    if (networkNodes.find(sourceId) != networkNodes.end() && networkNodes.find(targetId) != networkNodes.end()) {
        networkNodes[sourceId].outputConnections.push_back(targetId);
        std::cout << "🔗 Industrial Link Formed: Machine " << sourceId << " -> Machine " << targetId << std::endl;
    }
}

void MachineGraph::updateNetwork(float deltaTime) {
    for (auto& [id, node] : networkNodes) {
        node.processingTimer += deltaTime;

        // Simple Automation Behavior
        switch (node.type) {
            case MachineType::AUTO_MINER:
                if (node.processingTimer >= 5.0f) { // Every 5 seconds
                    node.processingTimer = 0.0f;
                    node.internalInventory++;
                    std::cout << "⛏️ Auto-Miner " << id << " extracted 1 Raw Ore chunk." << std::endl;
                    
                    // Push to downstream connections if available
                    if (!node.outputConnections.empty()) {
                        unsigned int targetId = node.outputConnections[0];
                        if (networkNodes.find(targetId) != networkNodes.end()) {
                            node.internalInventory--;
                            networkNodes[targetId].internalInventory++;
                            std::cout << "📦 Resource shifted down conveyor line to Machine " << targetId << std::endl;
                        }
                    }
                }
                break;

            case MachineType::BLAST_FURNACE:
                if (node.internalInventory > 0 && node.processingTimer >= 3.0f) {
                    node.processingTimer = 0.0f;
                    node.internalInventory--;
                    // Smelting turns ore into value!
                    ResourceManager::addResource(ResourceType::GOLD, 5); 
                    std::cout << "🔥 Blast Furnace " << id << " smelted ore into +5 Gold!" << std::endl;
                }
                break;
                ChronicleDatabase::logEvent(EventType::FINANCIAL, "Blast Furnace " + std::to_string(id) + " processed raw ore into marketplace Gold bullion.");

            default:
                break;
        }
    }
}