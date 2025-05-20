#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Renderer.h"
#include "World.h"
#include "ErrorLogger.h"
#include "Controls.h"
#include "Config.h"
#include "Debugger.h"
#include "Player.h"

Shader shader;
Camera camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
Renderer renderer(camera);
Controls controls;
World world;
Player player;
int frameCount;
double lastTime, currentTime;
double deltaTime;
double lastFPSUpdate = 0.0;


void display(GLFWwindow* window, World& world) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.52f, 0.8f, 0.92f, 1.0);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height); // Optional, good practice if window resizes
        
    renderer.RenderWorld(world);
}

void mouse(GLFWwindow* window, double x, double y) {
    controls.ProcessMouse(x, y);
}

void UpdateDeltaTime() {
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    if (Config::SHOW_FPS) {
        frameCount++;
        if (currentTime - lastFPSUpdate >= 1.0) {
            //printf("FPS: %d\n", frameCount);  
            frameCount = 0;
            lastFPSUpdate = currentTime;
        }
    }
    lastTime = currentTime;
}

int main(int argc, char** argv) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, "Voxel Renderer", nullptr, nullptr);
    if (!window) {
        ErrorLogger::LogError("Failed to Create GLFW Window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    SetupDebugCallback();
    renderer.Initialize();
    glfwSetCursorPos(window, Config::SCREEN_WIDTH / 2, Config::SCREEN_HEIGHT / 2);
    controls.SetInitialMousePosition(Config::SCREEN_WIDTH / 2.0f, Config::SCREEN_HEIGHT / 2.0f);

    glfwSetCursorPosCallback(window, mouse);
    world.Generate(10,10);

    lastTime, currentTime = glfwGetTime();
    deltaTime = 0.0;
    frameCount = 0;


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        UpdateDeltaTime();
        controls.ProcessKeyboard(window, deltaTime);
        player.UpdatePlayerMovement(deltaTime, controls.GetMovementInput(), camera.GetFront(), camera.GetRight());
        camera.UpdateFromPlayer(player, controls.GetMouseDelta());
        display(window, world);
        glfwSwapBuffers(window);

    }
    glfwDestroyWindow(window);
    glfwTerminate();
}