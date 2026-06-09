#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <unordered_map>

enum class ResourceType {
    WOOD,
    STONE,
    FOOD,
    WATER,
    GOLD,
    METALLURGY_TIER
};

class ResourceManager {
public:
    // Core Stockpile Variables
    static int woodCount;
    static int stoneCount;
    static int foodCount;
    static int waterCount;
    static int goldCount;
    static int metallurgyTier; // 1 = Stone/Bronze, 2 = Iron, 3 = Improvised

    // Thread-safe modification functions
    static void addResource(ResourceType type, int amount);
    static bool spendResource(ResourceType type, int amount);
    static int getResourceCount(ResourceType type);
    
    // Debug helper to print status to the UI/Console later
    static void printStockpile();
};

#endif // RESOURCE_MANAGER_H