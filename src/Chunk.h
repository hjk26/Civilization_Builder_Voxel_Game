#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include "Terrain.h"

// Structs for tracking chunks in 3D space
struct ChunkPos {
    int x;
    int y;
    int z;
    bool operator==(const ChunkPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct ChunkHash {
    std::size_t operator()(const ChunkPos& p) const {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1) ^ (std::hash<int>()(p.z) << 2);
    }
};

class Chunk;

class World {
public:
    std::unordered_map<ChunkPos, Chunk*, ChunkHash> chunks;
    World() {}
    ~World();

    unsigned char getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, unsigned char type);
};

class Chunk {
public:
    unsigned char blocks[16][16][16];
    std::vector<float> meshVertices;
    std::vector<float> waterVertices;
    unsigned int VAO, VBO;
    unsigned int waterVAO, waterVBO;
    int vertexCount = 0;
    int waterVertexCount = 0;
    int chunkX, chunkY, chunkZ;
    World* world;

    Chunk(int x, int y, int z, World* w);
    ~Chunk();

    void generateMesh();
    void updateGPU();
    void render(unsigned int modelLoc);
    void renderWater(unsigned int modelLoc);

private:
    void addFace(float x, float y, float z, std::string side, int blockType);
};

// Make the global terrain system accessible to the chunks
extern TerrainSystem terrainSystem;