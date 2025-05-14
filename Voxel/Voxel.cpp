#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shader.h"
#include "Renderer.h"
#include "World.h"
#include "ErrorLogger.h"

Shader shader;
Renderer renderer;
World world;
GLuint vao = 0, vbo = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Chunk chunk = world.CreateChunk(0, 0);
    world.RenderChunk(renderer, chunk);
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Voxel Renderer");
    glewInit();
    glEnable(GL_DEPTH_TEST);  // Enable depth testing for proper z-ordering

    renderer.Initialize();

    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}