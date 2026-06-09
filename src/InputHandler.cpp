#include "InputHandler.h"
#include <cmath>

// Define static members
glm::vec3 InputHandler::cameraPos = glm::vec3(0.0f, 80.0f, 0.0f);
glm::vec3 InputHandler::cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 InputHandler::cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float InputHandler::yaw = -90.0f;
float InputHandler::pitch = 0.0f;
bool InputHandler::firstMouse = true;
float InputHandler::lastX = 400.0f;
float InputHandler::lastY = 300.0f;
float InputHandler::verticalVelocity = 0.0f;
bool InputHandler::isGrounded = false;

const float GRAVITY = -30.0f;
const float PLAYER_EYE_HEIGHT = 2.5f;

void InputHandler::initialize(float startX, float startY, float startZ) {
    cameraPos = glm::vec3(startX, startY, startZ);
}

bool InputHandler::isSolidBlock(float x, float y, float z, World& world) {
    unsigned char block = world.getBlock((int)std::floor(x), (int)std::floor(y), (int)std::floor(z));
    // Air (0) and Water (6) are traversable, anything else blocks the player
    return (block != 0 && block != 6);
}

void InputHandler::processKeyboard(GLFWwindow* window, World& world, float deltaTime) {
    // Swimming drag coefficient check
    unsigned char currentBlock = world.getBlock((int)std::floor(cameraPos.x), (int)std::floor(cameraPos.y - 1.5f), (int)std::floor(cameraPos.z));
    bool isSwimming = (currentBlock == 6);

    float cameraSpeed = (isSwimming ? 4.0f : 10.0f) * deltaTime;

    // Stable horizontal movement vector using looking yaw angle
    glm::vec3 frontXZ;
    frontXZ.x = std::cos(glm::radians(yaw));
    frontXZ.y = 0.0f;
    frontXZ.z = std::sin(glm::radians(yaw));
    frontXZ = glm::normalize(frontXZ);

    glm::vec3 rightXZ = glm::normalize(glm::cross(frontXZ, cameraUp));
    glm::vec3 nextPos = cameraPos;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) nextPos += frontXZ * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) nextPos -= frontXZ * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) nextPos -= rightXZ * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) nextPos += rightXZ * cameraSpeed;

    // Horizontal Collision Checks
    float stepHeight = 1.0f;
    bool canMoveX = !isSolidBlock(nextPos.x, cameraPos.y, cameraPos.z, world) && 
                    !isSolidBlock(nextPos.x, cameraPos.y - 1.5f, cameraPos.z, world);
    if (!canMoveX) {
        if (!isSolidBlock(nextPos.x, cameraPos.y + stepHeight, cameraPos.z, world)) cameraPos.y += stepHeight;
        else nextPos.x = cameraPos.x;
    }
    cameraPos.x = nextPos.x;

    bool canMoveZ = !isSolidBlock(cameraPos.x, cameraPos.y, nextPos.z, world) && 
                    !isSolidBlock(cameraPos.x, cameraPos.y - 1.5f, nextPos.z, world);
    if (!canMoveZ) {
        if (!isSolidBlock(cameraPos.x, cameraPos.y + stepHeight, nextPos.z, world)) cameraPos.y += stepHeight;
        else nextPos.z = cameraPos.z;
    }
    cameraPos.z = nextPos.z;

    // Gravity and Jumping handling
    verticalVelocity += GRAVITY * deltaTime;
    if (isSwimming && verticalVelocity < -3.0f) verticalVelocity = -3.0f; // Terminal velocity resistance in water

    float moveY = verticalVelocity * deltaTime;
    float footY = cameraPos.y - PLAYER_EYE_HEIGHT;

    if (verticalVelocity < 0) { // Falling
        if (isSolidBlock(cameraPos.x, footY + moveY, cameraPos.z, world)) {
            verticalVelocity = 0;
            isGrounded = true;
            cameraPos.y = std::floor(footY + moveY) + 1.0f + PLAYER_EYE_HEIGHT; // Snap to surface
        } else {
            cameraPos.y += moveY;
            isGrounded = false;
        }
    } else { // Rising or Floating
        cameraPos.y += moveY;
        isGrounded = false;
    }

    // Spacebar control
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (isSwimming) {
            verticalVelocity = 4.0f; // Swim up smoothly
        } else if (isGrounded) {
            verticalVelocity = 10.0f; // Solid ground Jump
            isGrounded = false;
        }
    }

    if (cameraPos.y < -50.0f) cameraPos = glm::vec3(8.0f, 70.0f, 8.0f); // Protection void fallback
}

void InputHandler::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos; lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos; lastY = ypos;

    float sensitivity = 0.1f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front.y = std::sin(glm::radians(pitch));
    front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    InputHandler::cameraFront = glm::normalize(front);
}