#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Renderer.h"
#include "World.h"
#include "ErrorLogger.h"
#include "Controls.h"

Shader shader;
Camera camera(glm::vec3(0.0f, 7.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
Renderer renderer(camera);
Controls controls(camera);
World world;


void display(GLFWwindow* window, Chunk chunk) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height); // Optional, good practice if window resizes
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    world.RenderChunk(renderer, chunk);

}

void mouse(GLFWwindow* window, double x, double y) {
    controls.ProcessMouse(x, y);
}

int main(int argc, char** argv) {
    int screenWidth = 800, screenHeight = 600;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Voxel Renderer", nullptr, nullptr);
    if (!window) {
        ErrorLogger::LogError("Failed to Create GLFW Window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    renderer.Initialize();
    Chunk chunk = world.CreateChunk(0, 0);
    glfwSetCursorPos(window, screenWidth / 2, screenHeight / 2);
    controls.SetInitialMousePosition(screenWidth / 2.0, screenHeight / 2.0);

    glfwSetCursorPosCallback(window, mouse);
    while (!glfwWindowShouldClose(window)) {
        controls.UpdateDeltaTime();
        controls.ProcessKeyboard(window);
        display(window, chunk);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}