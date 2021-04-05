#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>
#define PI 3.14159265358979323846

//управление:
//WASD, LSHIFT, LCTRL - движение камеры
//QE, MOUSE WHEEL - вращение камеры
//ARROWS - вращение объекта
//R/LMB - вращение объекта
//RCTRL - отсечение граней
//MMB - каркасный режим

const GLint WIDTH = 950;
const GLint HEIGHT = 950;
bool rotation[6] = {false, false, false, false, false, false};
bool yaw[2] = {false, false};
bool wasdsc[6] = {false, false, false, false, false, false};
int cage = 1;
float rotVKoef = 2;
float rot = 0.5;
int speed = 1;
int mode = 0;
bool needUpd = false;

struct camera {
    GLfloat posX;
    GLfloat posY;
    GLfloat posZ;
    GLfloat stdDirX;
    GLfloat stdDirY;
    GLfloat stdDirZ;
    GLfloat stdRX;
    GLfloat stdRY;
    GLfloat stdRZ;
    GLfloat yaw;
    GLfloat pitch;
};

camera camera{82.5, 70, 32.5,
              -0.65, -0.4, -0.65,
              0.71, 0, -0.71,
              -135, -23};

void cross(const GLfloat v1[3], const GLfloat v2[3], GLfloat vr[3]) {
    vr[0] = v1[1] * v2[2] - v1[2] * v2[1];
    vr[1] = -1 * (v1[0] * v2[2] - v1[2] * v2[0]);
    vr[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

GLfloat vLength(const GLfloat v[3]) {
    GLfloat len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return len;
}

GLfloat dot(const GLfloat v1[3], const GLfloat v2[3]) {
    GLfloat angle = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
    angle = angle / (vLength(v1) * vLength(v2));
    return angle;
}

void normalise(GLfloat v[3]) {
    GLfloat len = vLength(v);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

void lookAt(GLfloat Ax, GLfloat Ay, GLfloat Az, GLfloat Cx, GLfloat Cy, GLfloat Cz, GLfloat Ux, GLfloat Uy, GLfloat Uz) {
    GLfloat vb[3] = {Cx - Ax, Cy - Ay, Cz - Az};
    GLfloat vu[3] = {Ux, Uy, Uz};
    GLfloat vr[3] = {0};
    cross(vu, vb, vr);
    cross(vb, vr, vu);
    GLfloat vx[3] = {1, 0, 0};
    GLfloat vy[3] = {0, 1, 0};

    GLfloat k1 = 0;
    GLfloat tx[3] = {0};
    cross(vy, vu, tx);
    if (dot(tx, vr) < 0) {
        k1 = 1;
    } else {
        k1 = -1;
    }
    GLfloat k2 = 0;
    GLfloat ty[3] = {0};
    cross(vx, vr, ty);
    if (dot(ty, vu) < 0) {
        k2 = 1;
    } else {
        k2 = -1;
    }

    glRotatef(k1 * 180 * acosf(dot(vy, vu)) / PI, 1, 0, 0);
    glRotatef(k2 * 180 * acosf(dot(vx, vr)) / PI, 0, 1, 0);
    glTranslatef(-Cx, -Cy, -Cz);
}

void updCamera() {
    if (yaw[0]) {
        camera.yaw -= rot * rotVKoef;
        if (camera.yaw < -180) {
            camera.yaw = 180;
        }
    }
    if (yaw[1]) {
        camera.yaw += rot * rotVKoef;
        if (camera.yaw > 180) {
            camera.yaw = -180;
        }
    }
    camera.stdDirX = cosf(PI * camera.yaw / 180) * cosf(PI * camera.pitch / 180);
    camera.stdDirY = sinf(PI * camera.pitch / 180);
    camera.stdDirZ = sinf(PI * camera.yaw / 180) * cosf(PI * camera.pitch / 180);
    GLfloat vd[3] = {camera.stdDirX, camera.stdDirY, camera.stdDirZ};
    normalise(vd);
    camera.stdDirX = vd[0];
    camera.stdDirY = vd[1];
    camera.stdDirZ = vd[2];
    GLfloat vu[3] = {0, 1, 0};
    GLfloat vr[3] = {0};
    cross(vd, vu, vr);
    normalise(vr);
    camera.stdRX = vr[0];
    camera.stdRY = vr[1];
    camera.stdRZ = vr[2];
    if (wasdsc[0]) {
        camera.posX += speed * camera.stdDirX;
        camera.posY += speed * camera.stdDirY;
        camera.posZ += speed * camera.stdDirZ;
    }
    if (wasdsc[1]) {
        camera.posX -= speed * camera.stdRX;
        camera.posY -= speed * camera.stdRY;
        camera.posZ -= speed * camera.stdRZ;
    }
    if (wasdsc[2]) {
        camera.posX -= speed * camera.stdDirX;
        camera.posY -= speed * camera.stdDirY;
        camera.posZ -= speed * camera.stdDirZ;
    }
    if (wasdsc[3]) {
        camera.posX += speed * camera.stdRX;
        camera.posY += speed * camera.stdRY;
        camera.posZ += speed * camera.stdRZ;
    }
    if (wasdsc[4]) {
        camera.posY += speed;
    }
    if (wasdsc[5]) {
        camera.posY -= speed;
    }
}

void drawSectors() {
    glPointSize(1.0);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex2f(0, -1);
    glVertex2f(0, 1);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex2f(-1, 0);
    glVertex2f(1, 0);
    glEnd();
}

void drawSystem(float s) {
    glPointSize(1.0);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(s, 0, 0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, s, 0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, s);
    glEnd();
}

void drawCube(float s) {
    glPointSize(1.0);
    glBegin(GL_POLYGON);
    glColor3f(0, 1, 0);
    glVertex3f(-s, s, s);
    glVertex3f(-s, -s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(s, s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex3f(-s, s, -s);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0, 1, 1);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, s);
    glVertex3f(-s, -s, s);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0, 0, 1);
    glVertex3f(s, s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 1);
    glVertex3f(-s, s, -s);
    glVertex3f(-s, s, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);
    glVertex3f(-s, s, -s);
    glEnd();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset < 0) {
        camera.pitch -= rot * rotVKoef;
        if (camera.pitch < -80) {
            camera.pitch = -80;
        }
    }
    if (yoffset > 0) {
        camera.pitch += rot * rotVKoef;
        if (camera.pitch > 80) {
            camera.pitch = 80;
        }
    }
    needUpd = true;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        rotation[0] = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        rotation[0] = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rotation[1] = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        rotation[1] = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        cage *= -1;
        if (cage < 0) {
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glEnable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    needUpd = true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_RIGHT_CONTROL && action == GLFW_PRESS) {
        mode = (mode + 1) % 2;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        rotation[2] = true;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        rotation[3] = true;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        rotation[4] = true;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        rotation[5] = true;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
        rotation[2] = false;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
        rotation[3] = false;
    }
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
        rotation[4] = false;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
        rotation[5] = false;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        yaw[0] = true;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        yaw[1] = true;
    }
    if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
        yaw[0] = false;
    }
    if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
        yaw[1] = false;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        wasdsc[0] = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        wasdsc[0] = false;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        wasdsc[1] = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        wasdsc[1] = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        wasdsc[2] = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        wasdsc[2] = false;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        wasdsc[3] = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        wasdsc[3] = false;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        wasdsc[4] = true;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
        wasdsc[4] = false;
    }
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
        wasdsc[5] = true;
    }
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
        wasdsc[5] = false;
    }
    needUpd = true;
}

void rotate() {
    if (rotation[0]) {
        glRotatef(rot, 0, 0, 1);
    }
    if (rotation[1]) {
        glRotatef(-rot, 0, 0, 1);
    }
    if (rotation[2]) {
        glRotatef(rot, 0, 1, 0);
    }
    if (rotation[3]) {
        glRotatef(-rot, 0, 1, 0);
    }
    if (rotation[4]) {
        glRotatef(-rot, 1, 0, 0);
    }
    if (rotation[5]) {
        glRotatef(rot, 1, 0, 0);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glMatrixMode(GL_PROJECTION);
    //матрица ортгональной проекции
    glOrtho(-25, 25, -25, 25, 0.1, 100);
    GLfloat ortho[16];
    glGetFloatv(GL_PROJECTION_MATRIX, ortho);
    //матрица перспективной проекции
    glLoadIdentity();
    glFrustum(-0.05, 0.05, -0.05, 0.05, 0.1, 100);
    GLfloat persp[16];
    glGetFloatv(GL_PROJECTION_MATRIX, persp);

    glMatrixMode(GL_MODELVIEW);
    //матрица вида орто1
    lookAt(50, -50, 0, 50, -50, -50, 0, 1, 0);
    GLfloat view1[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view1);
    //матрица вида орто2
    glLoadIdentity();
    lookAt(-50, 50, 0, -100, 50, 0, 0, 1, 0);
    GLfloat view2[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view2);
    //матрица вида орто3
    glLoadIdentity();
    lookAt(-50, -50, 0, -50, -100, 0, 0, 0, 1);
    GLfloat view3[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view3);
    //матрица модели 1
    glLoadIdentity();
    glTranslatef(50, -50, 0);
    GLfloat model1[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, model1);
    //матрица модели 2
    glLoadIdentity();
    glTranslatef(-50, 50, 0);
    GLfloat model2[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, model2);
    //матрица модели 3
    glLoadIdentity();
    glTranslatef(-50, -50, 0);
    GLfloat model3[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, model3);
    //матрицы модели
    glLoadIdentity();
    glTranslatef(50, 50, 0);
    GLfloat model[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, model);

    glLoadIdentity();
    GLfloat rModel[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, rModel);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (mode == 1) {
            glFrontFace(GL_CW);
        } else {
            glFrontFace(GL_CCW);
        }
        //основной куб
        glViewport(screenWidth / 2, screenHeight / 2, screenWidth / 2, screenHeight / 2);
        rotate();
        glMultMatrixf(rModel);
        glGetFloatv(GL_MODELVIEW_MATRIX, rModel);
        glLoadIdentity();
        lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
                camera.posX, camera.posY, camera.posZ, 0, 1, 0);
        //glLoadMatrixf(view);
        glMultMatrixf(model);
        drawSystem(20);
        glMultMatrixf(rModel);
        drawCube(10);
        //проекция на Oyz
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(ortho);
        glMatrixMode(GL_MODELVIEW);
        glViewport(0, screenHeight / 2, screenWidth / 2, screenHeight / 2);
        glLoadMatrixf(view2);
        glMultMatrixf(model2);
        glMultMatrixf(rModel);
        drawCube(10);
        //проекция на Oxy
        glViewport(screenWidth / 2, 0, screenWidth / 2, screenHeight / 2);
        glLoadMatrixf(view1);
        glMultMatrixf(model1);
        glMultMatrixf(rModel);
        drawCube(10);
        //проекция на Oxz
        glViewport(0, 0, screenWidth / 2, screenHeight / 2);
        glLoadMatrixf(view3);
        glMultMatrixf(model3);
        glMultMatrixf(rModel);
        drawCube(10);
        //разделение на зоны
        glViewport(0, 0, screenWidth, screenHeight);
        glLoadIdentity ();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity ();
        drawSectors();
        glLoadMatrixf(persp);
        glMatrixMode(GL_MODELVIEW);

        glfwSwapBuffers(window);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwPollEvents();
        if (needUpd) {
            updCamera();
            needUpd = false;
        }
    }
    glfwTerminate();
    return 0;
}
