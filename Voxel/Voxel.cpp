#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shader.h"
#include "Renderer.h"
#include "World.h"
#include "ErrorLogger.h"
#include "Controls.h"

Shader shader;
Renderer renderer;
World world;
GLuint vao = 0, vbo = 0;
Controls controls;

void applyCamera(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glm::vec3 pos = controls.getCameraPosition();
    glm::vec3 front = controls.getCameraFront();
    glm::vec3 up = controls.getCameraUp();

    glm::vec3 center = pos + front;
    gluLookAt(pos.x, pos.y, pos.z, center.x, center.y, center.z, up.x, up.y, up.z);
}

void display() {
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    applyCamera(width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Chunk chunk = world.CreateChunk(0, 0);
    world.RenderChunk(renderer, chunk);
    glutSwapBuffers();
}

void idle() {
    controls.UpdateDeltaTime();
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    controls.ProcessKeyboard(key);
}

void mouse(int x, int y) {
    controls.ProcessMouse(x, y);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Voxel Renderer");
    glewInit();
    glEnable(GL_DEPTH_TEST);  // Enable depth testing for proper z-ordering
    renderer.Initialize();
    renderer.SetControls(&controls);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouse);
    glutMainLoop();
    return 0;
}