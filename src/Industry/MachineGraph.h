#ifndef MACHINE_GRAPH_H
#define MACHINE_GRAPH_H

#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>

enum class MachineType {
    AUTO_MINER,
    CONVEYOR_BELT,
    STEAM_CRUSHER,
    BLAST_FURNACE
};

struct MachineNode {
    unsigned int id;
    MachineType type;
    glm::ivec3 worldPosition;
    unsigned int owningFactionId;
    
    // Graph Edges: IDs of machines this node outputs resources to
    std::vector<unsigned int> outputConnections;
    
    float processingTimer = 0.0f;
    int internalInventory = 0;
};

class MachineGraph {
public:
    static std::map<unsigned int, MachineNode> networkNodes;
    static unsigned int nextMachineId;

    static unsigned int registerMachine(MachineType type, glm::ivec3 pos, unsigned int factionId);
    static void connectMachines(unsigned int sourceId, unsigned int targetId);
    static void updateNetwork(float deltaTime);
};

#endif // MACHINE_GRAPH_H