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
#include "MainThreadDispatcher.h"

Shader shader;
Camera camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
ThreadPool threadPool;
MainThreadDispatcher mtd;
Renderer renderer(camera, threadPool, mtd);
Controls controls;
World world(threadPool, mtd);
Player player(world);
int frameCount = 0;
double lastTime, currentTime;
double deltaTime = 0.0;
double lastFPSUpdate = 0.0;
bool playerPlaced = false;

void display(GLFWwindow* window, World& world) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.52f, 0.8f, 0.92f, 1.0);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
        
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
            printf("FPS: %d\n", frameCount);  
            frameCount = 0;
            lastFPSUpdate = currentTime;
        }
    }
    lastTime = currentTime;
}

void SpawnPlayer() {
    auto result = world.GetPositionAtXZ(0,0);
    if (result) {
        glm::ivec3 foundPosition = *result;
        player.SetPosition(glm::vec3(foundPosition.x + 0.5f, foundPosition.y + 1, foundPosition.z + 0.5f));
    }
    else {
        ErrorLogger::LogError("Failed to find a valid spawn");
    }
    playerPlaced = true;
}

void CheckChunkGenerationRequired() {
    ChunkPosition playerChunk = world.GetChunkPositionFromPlayerCoordinates(player.GetPosition());
    std::lock_guard<std::mutex> lock(world.chunkLoadQueueMutex);

    {
        for (int dx = -Config::CHUNK_LOAD_RADIUS; dx <= Config::CHUNK_LOAD_RADIUS; dx++) {
            for (int dz = -Config::CHUNK_LOAD_RADIUS; dz <= Config::CHUNK_LOAD_RADIUS; dz++) {
                ChunkPosition checkPosition = playerChunk + ChunkPosition(dx, dz);
                {
                    std::lock_guard<std::mutex> chunkLock(world.chunkMutex);
                    bool chunkExists = world.chunks.find(checkPosition) != world.chunks.end();
                    bool isGenerating = world.chunksBeingGenerated.contains(checkPosition);
                    if (chunkExists || isGenerating) continue;
                }
                world.chunkLoadQueue.push(checkPosition);
            }
        }
    }
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

    srand(time(0));
    Config::WOLRD_SEED = Config::DEBUG_MODE ? 1000 : (rand() % 1000);
    renderer.Initialize();
    glfwSetCursorPos(window, Config::SCREEN_WIDTH / 2, Config::SCREEN_HEIGHT / 2);
    controls.SetInitialMousePosition(Config::SCREEN_WIDTH / 2.0f, Config::SCREEN_HEIGHT / 2.0f);
    glfwSetCursorPosCallback(window, mouse);
    world.Generate(Config::SQRT_CHUNKS_TO_CREATE);
    lastTime, currentTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        UpdateDeltaTime();
        display(window, world);
        if (world.rendered) {
            if(!playerPlaced) SpawnPlayer();
            controls.ProcessKeyboard(window, deltaTime);
            player.UpdatePlayerMovement(deltaTime, controls.GetMovementInput(), camera.GetFront(), camera.GetRight());
            camera.UpdateFromPlayer(player, controls.GetMouseDelta());
        }
        CheckChunkGenerationRequired();
        world.ProcessChunkLoadQueue(1);
        mtd.Process();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}