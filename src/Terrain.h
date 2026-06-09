#pragma once

#include <FastNoiseLite.h>
#include <glm/glm.hpp>
#include <cmath>

enum class BiomeType {
    DESERT,
    PLAINS,
    FOREST,
    SNOW_MOUNTAIN,
    EXTREME_PEAKS
};

struct VoxelTerrainData {
    int height;
    BiomeType biome;
    bool isRiver;
};

class TerrainSystem {
public:
    TerrainSystem();

    // Call this once at startup to set up noise seeds
    void initialize(int seed);

    // Returns structural height map info for X/Z coordinates
    VoxelTerrainData getTerrainData(int worldX, int worldZ);

    // Dynamic 3D noise check for caves (returns true if it's air)
    bool isCave(int worldX, int worldY, int worldZ, int surfaceHeight);

private:
    FastNoiseLite m_continentalNoise;
    FastNoiseLite m_peaksNoise;
    FastNoiseLite m_detailNoise;
    FastNoiseLite m_caveNoise;
    FastNoiseLite m_temperatureNoise;
    FastNoiseLite m_humidityNoise;

    BiomeType calculateBiome(float temp, float humidity, float elevation);
};