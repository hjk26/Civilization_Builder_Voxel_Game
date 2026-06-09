#include "Chunk.h"
#include <cmath>

World::~World() {
    for (auto& pair : chunks) {
        delete pair.second;
    }
    chunks.clear();
}

unsigned char World::getBlock(int x, int y, int z) {
    if (y < 0 || y >= 128) return 0;
    int cx = x / 16; int cy = y / 16; int cz = z / 16;
    int bx = x % 16; int by = y % 16; int bz = z % 16;

    if (x < 0) { cx = (x + 1) / 16 - 1; bx = 15 + (x + 1) % 16; }
    if (z < 0) { cz = (z + 1) / 16 - 1; bz = 15 + (z + 1) % 16; }

    ChunkPos pos{cx, cy, cz};
    auto it = chunks.find(pos);
    if (it == chunks.end()) return 0;

    return it->second->blocks[bx][by][bz];
}

void World::setBlock(int x, int y, int z, unsigned char type) {
    if (y < 0 || y >= 128) return;
    int cx = x / 16; int cy = y / 16; int cz = z / 16;
    int bx = x % 16; int by = y % 16; int bz = z % 16;

    if (x < 0) { cx = (x + 1) / 16 - 1; bx = 15 + (x + 1) % 16; }
    if (z < 0) { cz = (z + 1) / 16 - 1; bz = 15 + (z + 1) % 16; }

    ChunkPos pos{cx, cy, cz};
    auto it = chunks.find(pos);

    auto getChunk = [&](int cx_, int cy_, int cz_) -> Chunk* {
        auto it_ = chunks.find({cx_, cy_, cz_});
        return (it_ == chunks.end()) ? nullptr : it_->second;
    };

    if (it != chunks.end()) {
        Chunk* chunk = it->second;
        chunk->blocks[bx][by][bz] = type;
        
        // Regenerate main mesh and force upload data to graphics card
        chunk->generateMesh();
        chunk->updateGPU(); 

        // CRITICAL FIX: Ensure all neighboring chunks run BOTH generateMesh AND updateGPU!
        if (bx == 0)   { Chunk* n = getChunk(cx - 1, cy, cz); if (n) { n->generateMesh(); n->updateGPU(); } }
        if (bx == 15)  { Chunk* n = getChunk(cx + 1, cy, cz); if (n) { n->generateMesh(); n->updateGPU(); } }
        if (by == 0)   { Chunk* n = getChunk(cx, cy - 1, cz); if (n) { n->generateMesh(); n->updateGPU(); } }
        if (by == 15)  { Chunk* n = getChunk(cx, cy + 1, cz); if (n) { n->generateMesh(); n->updateGPU(); } }
        if (bz == 0)   { Chunk* n = getChunk(cx, cy, cz - 1); if (n) { n->generateMesh(); n->updateGPU(); } }
        if (bz == 15)  { Chunk* n = getChunk(cx, cy, cz + 1); if (n) { n->generateMesh(); n->updateGPU(); } }
    }
}

Chunk::Chunk(int x, int y, int z, World* w) : chunkX(x), chunkY(y), chunkZ(z), world(w) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
        
    const int SEA_LEVEL = 44; // Anything below this absolute height receives water blocks

    for (int x_b = 0; x_b < 16; x_b++) {
        for (int z_b = 0; z_b < 16; z_b++) {
            int worldX = chunkX * 16 + x_b;
            int worldZ = chunkZ * 16 + z_b;
            int worldY_Base = chunkY * 16;
                
            VoxelTerrainData tData = terrainSystem.getTerrainData(worldX, worldZ);

            for (int y_b = 0; y_b < 16; y_b++) {
                int worldY = worldY_Base + y_b;

                // Cave Carving check
                if (terrainSystem.isCave(worldX, worldY, worldZ, tData.height)) {
                    // Even if it's a cave, if it's deep beneath sea level, fill it with water!
                    if (worldY <= SEA_LEVEL) {
                        blocks[x_b][y_b][z_b] = 6; // Water block
                    } else {
                        blocks[x_b][y_b][z_b] = 0; // Air
                    }
                    continue;
                }

                if (worldY > tData.height) {
                    // Check if empty sky space sits below Sea Level
                    if (worldY <= SEA_LEVEL) {
                        blocks[x_b][y_b][z_b] = 6; // Fill depressions with blue water
                    } else {
                        blocks[x_b][y_b][z_b] = 0; // Air
                    }
                } 
                else if (worldY == tData.height) {
                    // Check if beach shorelines should be sand instead of grass
                    if (tData.height <= SEA_LEVEL + 1 && tData.biome != BiomeType::SNOW_MOUNTAIN) {
                        blocks[x_b][y_b][z_b] = 4; // Beach Sand
                    } else {
                        switch (tData.biome) {
                            case BiomeType::DESERT:        blocks[x_b][y_b][z_b] = 4; break; // Sand
                            case BiomeType::SNOW_MOUNTAIN: blocks[x_b][y_b][z_b] = 5; break; // Snow
                            case BiomeType::EXTREME_PEAKS: blocks[x_b][y_b][z_b] = 3; break; // Raw Stone peaks
                            default:                       blocks[x_b][y_b][z_b] = 1; break; // Grass
                        }
                    }
                } 
                else if (worldY > tData.height - 4) {
                    if (tData.biome == BiomeType::DESERT) blocks[x_b][y_b][z_b] = 4;
                    else blocks[x_b][y_b][z_b] = 2; // Dirt
                } 
                else {
                    blocks[x_b][y_b][z_b] = 3; // Deep Stone
                }
            }
        }
    }
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
}

void Chunk::generateMesh() {
    meshVertices.clear();
    waterVertices.clear();
    // waterVertices.clear(); // If you use a separate water mesh later

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                int currentBlock = blocks[x][y][z];
                
                // 1. If it's air, skip it entirely!
                if (currentBlock == 0) continue;
                
                // 2. Calculate the exact global world position for this block
                int wx = chunkX * 16 + x;
                int wy = chunkY * 16 + y;
                int wz = chunkZ * 16 + z;

                // 3. Simple neighbor check logic
                // Draw the face ONLY if the neighbor is Air (0) 
                // OR if a solid block is touching Water (6)
                auto shouldDrawFace = [&](int nx, int ny, int nz) {
                    // Safety check: If out of world height bounds, handle safely
                    if (ny < 0 || ny >= 128) return false; 
                    
                    int neighborType = world->getBlock(nx, ny, nz);
                    
                    if (neighborType == 0) return true; // Neighbor is air -> draw face
                    if (currentBlock != 6 && neighborType == 6) return true; // Land touching water -> draw face
                    
                    return false; // Neighbor is solid -> hide face
                };

                // 4. Pass LOCAL coordinates (x,y,z) to addFace, NOT world coordinates!
                if (shouldDrawFace(wx, wy + 1, wz)) addFace(x, y, z, "TOP", currentBlock);
                if (shouldDrawFace(wx, wy - 1, wz)) addFace(x, y, z, "BOTTOM", currentBlock);
                if (shouldDrawFace(wx - 1, wy, wz)) addFace(x, y, z, "LEFT", currentBlock);
                if (shouldDrawFace(wx + 1, wy, wz)) addFace(x, y, z, "RIGHT", currentBlock);
                if (shouldDrawFace(wx, wy, wz + 1)) addFace(x, y, z, "FRONT", currentBlock);
                if (shouldDrawFace(wx, wy, wz - 1)) addFace(x, y, z, "BACK", currentBlock);
            }
        }
    }
    updateGPU();
}

void Chunk::addFace(float x, float y, float z, std::string side, int blockType) {
    float sideLight = 0.6f;
    if (side == "TOP") sideLight = 1.0f;
    else if (side == "BOTTOM") sideLight = 0.4f;
    else if (side == "FRONT" || side == "BACK") sideLight = 0.8f;

    float r = 0.6f, g = 0.6f, bl = 0.6f;
    float alpha = 1.0f; // Default fully opaque

    if (blockType == 1)      { r = 0.2f;  g = 0.75f; bl = 0.2f;  }  
    else if (blockType == 2) { r = 0.5f;  g = 0.3f;  bl = 0.15f; } 
    else if (blockType == 3) { r = 0.55f; g = 0.55f; bl = 0.55f; } 
    else if (blockType == 4) { r = 0.9f;  g = 0.82f; bl = 0.62f; } 
    else if (blockType == 5) { r = 0.95f; g = 0.95f; bl = 0.95f; } 
    else if (blockType == 6) { r = 0.1f;  g = 0.4f;  bl = 0.85f; alpha = 0.5f; } 

    // Determine target vector array based on block type
    std::vector<float>& targetBuffer = (blockType == 6) ? waterVertices : meshVertices;

    // Resetting AO factors to completely flat white (1.0f) to fix the invisible black face glitch
    float aoF[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    // Helper lambda to cleanly construct attributes
    auto pushV = [&](float vx, float vy, float vz, float vertexAO) {
        targetBuffer.push_back(vx);
        targetBuffer.push_back(vy);
        targetBuffer.push_back(vz);

        float finalB = sideLight * vertexAO;
        targetBuffer.push_back(r * finalB);
        targetBuffer.push_back(g * finalB);
        targetBuffer.push_back(bl * finalB);
        targetBuffer.push_back(alpha); 
    };

    // Winding order coordinates optimized for standard counter-clockwise backface culling
    // Standard CCW Winding Order for solid cube faces
    if (side == "TOP") {
        // Triangle 1
        pushV(x-0.5f, y+0.5f, z+0.5f, aoF[0]);
        pushV(x+0.5f, y+0.5f, z+0.5f, aoF[1]);
        pushV(x+0.5f, y+0.5f, z-0.5f, aoF[2]);
        // Triangle 2
        pushV(x+0.5f, y+0.5f, z-0.5f, aoF[2]);
        pushV(x-0.5f, y+0.5f, z-0.5f, aoF[3]);
        pushV(x-0.5f, y+0.5f, z+0.5f, aoF[0]);
    } else if (side == "BOTTOM") {
        // Triangle 1
        pushV(x-0.5f, y-0.5f, z-0.5f, aoF[0]);
        pushV(x+0.5f, y-0.5f, z-0.5f, aoF[1]);
        pushV(x+0.5f, y-0.5f, z+0.5f, aoF[2]);
        // Triangle 2
        pushV(x+0.5f, y-0.5f, z+0.5f, aoF[2]);
        pushV(x-0.5f, y-0.5f, z+0.5f, aoF[3]);
        pushV(x-0.5f, y-0.5f, z-0.5f, aoF[0]);
    } else if (side == "LEFT") {
        // Triangle 1
        pushV(x-0.5f, y-0.5f, z-0.5f, aoF[2]);
        pushV(x-0.5f, y-0.5f, z+0.5f, aoF[3]);
        pushV(x-0.5f, y+0.5f, z+0.5f, aoF[0]);
        // Triangle 2
        pushV(x-0.5f, y+0.5f, z+0.5f, aoF[0]);
        pushV(x-0.5f, y+0.5f, z-0.5f, aoF[1]);
        pushV(x-0.5f, y-0.5f, z-0.5f, aoF[2]);
    } else if (side == "RIGHT") {
        // Triangle 1
        pushV(x+0.5f, y-0.5f, z+0.5f, aoF[2]);
        pushV(x+0.5f, y-0.5f, z-0.5f, aoF[3]);
        pushV(x+0.5f, y+0.5f, z-0.5f, aoF[0]);
        // Triangle 2
        pushV(x+0.5f, y+0.5f, z-0.5f, aoF[0]);
        pushV(x+0.5f, y+0.5f, z+0.5f, aoF[1]);
        pushV(x+0.5f, y-0.5f, z+0.5f, aoF[2]);
    } else if (side == "FRONT") {
        // Triangle 1
        pushV(x-0.5f, y-0.5f, z+0.5f, aoF[0]);
        pushV(x+0.5f, y-0.5f, z+0.5f, aoF[1]);
        pushV(x+0.5f, y+0.5f, z+0.5f, aoF[2]);
        // Triangle 2
        pushV(x+0.5f, y+0.5f, z+0.5f, aoF[2]);
        pushV(x-0.5f, y+0.5f, z+0.5f, aoF[3]);
        pushV(x-0.5f, y-0.5f, z+0.5f, aoF[0]);
    } else if (side == "BACK") {
        // Triangle 1
        pushV(x+0.5f, y-0.5f, z-0.5f, aoF[0]);
        pushV(x-0.5f, y-0.5f, z-0.5f, aoF[1]);
        pushV(x-0.5f, y+0.5f, z-0.5f, aoF[2]);
        // Triangle 2
        pushV(x-0.5f, y+0.5f, z-0.5f, aoF[2]);
        pushV(x+0.5f, y+0.5f, z-0.5f, aoF[3]);
        pushV(x+0.5f, y-0.5f, z-0.5f, aoF[0]);
    }
}

void Chunk::updateGPU() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(float), meshVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float))); // Read 4 components (RGBA)
    glEnableVertexAttribArray(1);
    vertexCount = meshVertices.size() / 7;

    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(float), waterVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float))); // Read 4 components (RGBA)
    glEnableVertexAttribArray(1);
    waterVertexCount = waterVertices.size() / 7;
}

void Chunk::render(unsigned int modelLoc) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkX * 16, chunkY * 16, chunkZ * 16));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    // Step 1: Render Opaque Solid Blocks
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

// Separate function to render water after solids
void Chunk::renderWater(unsigned int modelLoc) {
    if (waterVertexCount == 0) return;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkX * 16, chunkY * 16, chunkZ * 16));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    // Step 2: Render Transparent Fluids
    glBindVertexArray(waterVAO);
    glDrawArrays(GL_TRIANGLES, 0, waterVertexCount);
}