#include "Minimap.h"
#include "InputHandler.h"
#include <vector>

unsigned int Minimap::quadVAO = 0;
unsigned int Minimap::quadVBO = 0;
unsigned int Minimap::mapTexture = 0;
unsigned int Minimap::shaderProgram = 0;

bool Minimap::isMaximized = false;
bool Minimap::mKeyWasPressed = false;

extern TerrainSystem terrainSystem;

void Minimap::initialize() {
    // 1. Create a larger 256x256 texture for high-detail tracking maps
    glGenTextures(1, &mapTexture);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    const char* vShader = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "out vec2 TexCoords;\n"
        "uniform vec2 scale;\n"
        "uniform vec2 offset;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos * scale + offset, 0.0, 1.0);\n"
        "   TexCoords = aTexCoords;\n"
        "}\0";

    const char* fShader = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D minimapTex;\n"
        "uniform bool maxMode;\n"
        "uniform float yaw;\n" // <-- Add player camera angle
        "\n"
        "void main() {\n"
        "   float dist = distance(TexCoords, vec2(0.5, 0.5));\n"
        "   \n"
        "   // --- PASS 1: DRAW THE ROTATING DIRECTIONAL ARROW ---\n"
        "   // Translate coordinates to center (0.0, 0.0) so we can spin around the center point\n"
        "   vec2 p = TexCoords - vec2(0.5, 0.5);\n"
        "   \n"
        "   // Convert player's looking Yaw angle to radians and adjust axis alignment\n"
        "   // We add 90 degrees (1.5708 rad) because our 0-yaw points along positive X\n"
        "   float angle = radians(yaw) + 1.5708;\n"
        "   float s = sin(angle);\n"
        "   float c = cos(angle);\n"
        "   \n"
        "   // Apply 2D Rotation Matrix transformations on pixel queries\n"
        "   vec2 rotatedP;\n"
        "   rotatedP.x = p.x * c - p.y * s;\n"
        "   rotatedP.y = p.x * s + p.y * c;\n"
        "   \n"
        "   // Scale check to make the arrow uniform relative to map modes\n"
        "   float arrowScale = maxMode ? 1.5 : 1.0;\n"
        "   vec2 arrowP = rotatedP * 25.0 / arrowScale;\n"
        "   \n"
        "   // Mathematical shape definition for a sharp navigation triangle/arrow pointer\n"
        "   // Base width bounds check and forward nose height clip bounds\n"
        "   if (arrowP.y > -0.6 && arrowP.y < 1.0 && abs(arrowP.x) < (1.0 - arrowP.y) * 0.5) {\n"
        "       // Check inner indentation cutout to give it an elegant arrowhead notch accent shape\n"
        "       if (!(arrowP.y < 0.0 && abs(arrowP.x) > (arrowP.y + 0.6) * 0.8)) {\n"
        "           FragColor = vec4(1.0, 0.2, 0.2, 1.0); // Bright Red Navigation Arrow Pointer\n"
        "           return;\n"
        "       }\n"
        "   }\n"
        "   \n"
        "   // --- PASS 2: DRAW STANDARD BACKGROUND MAP OVERLAYS ---\n"
        "   // Give the corner small map a beautiful circular frame, keep maximized map a square slate\n"
        "   if (!maxMode && dist > 0.5) {\n"
        "       discard;\n"
        "   }\n"
        "   FragColor = texture(minimapTex, TexCoords);\n"
        "}\0";

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vShader, NULL); glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fShader, NULL); glCompileShader(fs);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs); glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
}

void Minimap::checkInput(GLFWwindow* window) {
    bool mPressed = (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS);
    
    // Toggle once cleanly upon key release/initial strike down boundaries
    if (mPressed && !mKeyWasPressed) {
        isMaximized = !isMaximized;
        
        // Re-enable/Disable mouse looking so player can look at map without spinning camera around
        if (isMaximized) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    mKeyWasPressed = mPressed;
}

void Minimap::updateAndRender(glm::vec3 playerPos, int screenWidth, int screenHeight) {
    const int TEX_RES = 256;
    std::vector<unsigned char> pixelData(TEX_RES * TEX_RES * 3);

    // Dynamic Step Scaling:
    // Radar steps 1 block per pixel. Maximize steps 3 blocks per pixel to show a huge region!
    int step = isMaximized ? 3 : 1; 
    
    int startX = (int)playerPos.x - ((TEX_RES / 2) * step);
    int startZ = (int)playerPos.z - ((TEX_RES / 2) * step);

    for (int z = 0; z < TEX_RES; z++) {
        for (int x = 0; x < TEX_RES; x++) {
            int worldX = startX + (x * step);
            int worldZ = startZ + (z * step);

            VoxelTerrainData tData = terrainSystem.getTerrainData(worldX, worldZ);
            
            unsigned char r = 0, g = 0, b = 0;
            const int SEA_LEVEL = 44;

            if (tData.height <= SEA_LEVEL) {
                r = 25; g = 90; b = 190; // Ocean
            } else if (tData.height == SEA_LEVEL + 1 && tData.biome != BiomeType::SNOW_MOUNTAIN) {
                r = 225; g = 205; b = 150; // Coast
            } else {
                switch (tData.biome) {
                    case BiomeType::DESERT:        r = 230; g = 210; b = 160; break;
                    case BiomeType::SNOW_MOUNTAIN: r = 245; g = 245; b = 250; break;
                    case BiomeType::EXTREME_PEAKS: r = 130; g = 130; b = 135; break;
                    case BiomeType::FOREST:        r = 35;  g = 120; b = 40;  break;
                    default:                       r = 60;  g = 165; b = 65;  break;
                }
                
                float heightFactor = (tData.height - SEA_LEVEL) / 84.0f;
                r = (unsigned char)glm::clamp(r + (heightFactor * 35.0f), 0.0f, 255.0f);
                g = (unsigned char)glm::clamp(g + (heightFactor * 35.0f), 0.0f, 255.0f);
                b = (unsigned char)glm::clamp(b + (heightFactor * 35.0f), 0.0f, 255.0f);
            }

            int index = (z * TEX_RES + x) * 3;
            pixelData[index]     = r;
            pixelData[index + 1] = g;
            pixelData[index + 2] = b;
        }
    }

    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEX_RES, TEX_RES, GL_RGB, GL_UNSIGNED_BYTE, pixelData.data());

    glDisable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);

    float aspect = (float)screenWidth / (float)screenHeight;
    glm::vec2 scale;
    glm::vec2 offset;

    if (isMaximized) {
        // Center full screen layout config (Takes up 85% of screen height space gracefully)
        scale = glm::vec2(0.85f / aspect, 0.85f);
        offset = glm::vec2(0.0f, 0.0f);
    } else {
        // Traditional top-right radar circular configurations
        scale = glm::vec2(0.22f / aspect, 0.22f);
        offset = glm::vec2(0.73f, 0.73f);
    }

    glUniform2f(glGetUniformLocation(shaderProgram, "scale"), scale.x, scale.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "offset"), offset.x, offset.y);
    glUniform1i(glGetUniformLocation(shaderProgram, "maxMode"), isMaximized ? 1 : 0);

    // FIX: Grab live view rotation from your modular input system handler 
    // and feed it straight into the rotation shader matrix!
    // Ensure handler variables are accessible in local scope
    glUniform1f(glGetUniformLocation(shaderProgram, "yaw"), InputHandler::yaw);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void Minimap::cleanup() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &mapTexture);
    glDeleteProgram(shaderProgram);
}