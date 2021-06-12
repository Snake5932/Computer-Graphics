#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define PI 3.1415926535897932

//управление:
//WASD, LSHIFT, LCTRL - движение камеры
//QE, MOUSE WHEEL - вращение камеры
//ARROWS, R(L)MB - вращение объекта
//Enter - текстурирование
//RCTRL - модуляция при текстурировании
//RALT - изменение расчета отражений
//LALT - глобальный эмбиент

bool rotation[6] = {false, false, false, false, false, false};
bool yaw[2] = {false, false};
bool pitch = false;
bool yaw1 = false;
bool wasdsc[6] = {false, false, false, false, false, false};
bool updArr = false;
int cage = 1;
int cl = 0;
float rot = 0;
float speed = 0;
float delta = 0;
float prev = 0;
float cur = 0;

GLint WIDTH = 950;
GLint HEIGHT = 950;
float rotVCoef = 2;
float basicRot = 40;
float basicSpeed = 30;
float a1 = 10;
float b1 = 8;
float a2 = 4;
float b2 = 3;
int sp = 2; //число полных витков / 2
int n1 = 0; // число сегментов в витке: n1 + 10
int n2 = 0; // число сегментов в эллипсе: n2 + 10
float lo = 1.5; //удлинение спирали
float s = 20;
GLfloat clearColor[] = {0, 0, 0, 1};

glm::mat4 rModel = glm::mat4(1.0f);
glm::mat4 frustum = glm::perspective(glm::radians(60.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 1000.0f);
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

camera camera{0, 0, 50,
              0, 0, -1,
              1, 0, 0,
              -90, 0};

void multMatrVec(const glm::mat4 m1, const GLfloat v[4], GLfloat vr[4]) {
    for (int i = 0; i < 4; i++) {
        vr[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            vr[j] += m1[i][j] * v[i];
        }
    }
}

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

glm::mat4 lookAt(GLfloat Ax, GLfloat Ay, GLfloat Az, GLfloat Cx, GLfloat Cy, GLfloat Cz, GLfloat Ux, GLfloat Uy, GLfloat Uz) {
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
    glm::mat4 lookAtM = glm::mat4(1.0f);
    lookAtM = glm::rotate(lookAtM, glm::radians(float(k1 * 180 * acosf(dot(vy, vu)) / PI)), glm::vec3(1.0, 0.0, 0.0));
    lookAtM = glm::rotate(lookAtM, glm::radians(float(k2 * 180 * acosf(dot(vx, vr)) / PI)), glm::vec3(0.0, 1.0, 0.0));
    lookAtM = glm::translate(lookAtM, glm::vec3(-Cx, -Cy, -Cz));
    return lookAtM;
}

void updCamera() {
    yaw1 = false;
    if (yaw[0]) {
        camera.yaw -= rot * rotVCoef;
        if (camera.yaw < -180) {
            camera.yaw = 180;
        }
        yaw1 = true;
    }
    if (yaw[1]) {
        camera.yaw += rot * rotVCoef;
        if (camera.yaw > 180) {
            camera.yaw = -180;
        }
        yaw1 = true;
    }
    if (yaw1 || pitch) {
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
    }
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

void calculateFrene(glm::mat4 *frene) {
    for (int k = -sp; k < sp; k++) {
        float angle = 0;
        for (int i = 0; i < (n1 + 10); i++) {
            GLfloat vT[3] = {-a1 * sinf(angle), b1 * cosf(angle), lo};
            GLfloat vC2[3] = {-a1 * cosf(angle), -b1 * sinf(angle), 0};
            GLfloat vB[3] = {0};
            cross(vT, vC2, vB);
            normalise(vT);
            normalise(vB);
            GLfloat vN[3] = {0};
            cross(vB, vT, vN);
            GLfloat vF[16] = {vN[0], vN[1], vN[2], 0,
                              vB[0], vB[1], vB[2], 0,
                              vT[0], vT[1], vT[2], 0,
                              a1 * cosf(angle), b1 * sinf(angle), static_cast<GLfloat>(lo * angle + 2 * lo * PI * k), 1};
            for (int m = 0; m < 4; m++) {
                for (int j = 0; j < 4; j++) {
                    frene[(n1 + 10) * (k + sp) + i][m][j] = vF[m * 4 + j];
                }
            }
            angle += float(2 * PI / (n1 + 10));
        }
    }
}

void calculateVertices(glm::mat4 *frene, GLfloat *vertices) {
    for (int k = -sp; k < sp; k++) {
        float angle = 0;
        for (int i = 0; i < (n1 + 10); i++) {
            vertices[(n1 + 10) * (k + sp) * 3 + 3 * i] = a1 * cosf(angle);
            vertices[(n1 + 10) * (k + sp) * 3 + 3 * i + 1] = b1 * sinf(angle);
            vertices[(n1 + 10) * (k + sp) * 3 + 3 * i + 2] = lo * angle + 2 * lo * PI * k;
            angle += float(2 * PI / (n1 + 10));
        }
    }
    float angle = 0;
    vertices[2 * sp * 3 * (n1 + 10) + 3 * 0 + 0] = 0;
    vertices[2 * sp * 3 * (n1 + 10) + 3 * 0 + 1] = 0;
    vertices[2 * sp * 3 * (n1 + 10) + 3 * 0 + 2] = 0;
    for (int i = 1; i < (n2 + 11); i++) {
        vertices[2 * sp * 3 * (n1 + 10) + 3 * i + 0] = a2 * cosf(angle);
        vertices[2 * sp * 3 * (n1 + 10) + 3 * i + 1] = b2 * sinf(angle);
        vertices[2 * sp * 3 * (n1 + 10) + 3 * i + 2] = 0;
        angle += float(2 * PI / (n2 + 10));
    }
    vertices[2 * sp * 3 * (n1 + 10) + 3 * (n2 + 11) + 0] = vertices[2 * sp * 3 * (n1 + 10) + 3 * 1 + 0];
    vertices[2 * sp * 3 * (n1 + 10) + 3 * (n2 + 11) + 1] = vertices[2 * sp * 3 * (n1 + 10) + 3 * 1 + 1];
    vertices[2 * sp * 3 * (n1 + 10) + 3 * (n2 + 11) + 2] = vertices[2 * sp * 3 * (n1 + 10) + 3 * 1 + 2];
    for (int k = -sp; k < sp; k++) {
        for (int i = 0; i < (n1 + 10); i++) {
            for (int j = 0; j < (n2 + 10); j++) {
                GLfloat vr[4] = {0};
                glm::mat4 m1 = frene[(n1 + 10) * (k + sp) + i];
                glm::mat4 m2;
                if (i == (n1 + 10) - 1) {
                    if (k < sp - 1) {
                        m2 = frene[(n1 + 10) * (k + 1 + sp)];
                    } else {
                        m2 = m1;
                    }
                } else {
                    m2 = frene[(n1 + 10) * (k + sp) + i + 1];
                }
                GLfloat v[4] = {vertices[2 * sp * 3 * (n1 + 10) + 3 * (j + 1) + 0],
                                vertices[2 * sp * 3 * (n1 + 10) + 3 * (j + 1) + 1],
                                vertices[2 * sp * 3 * (n1 + 10) + 3 * (j + 1) + 2], 1};

                multMatrVec(m2, v, vr);
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0] = vr[0];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1] = vr[1];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2] = vr[2];
                multMatrVec(m1, v, vr);
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3] = vr[0];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4] = vr[1];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5] = vr[2];
                v[0] = vertices[2 * sp * 3 * (n1 + 10) + 3 * ((j + 1) % (n2 + 10) + 1) + 0];
                v[1] = vertices[2 * sp * 3 * (n1 + 10) + 3 * ((j + 1) % (n2 + 10) + 1) + 1];
                v[2] = vertices[2 * sp * 3 * (n1 + 10) + 3 * ((j + 1) % (n2 + 10) + 1) + 2];
                multMatrVec(m1, v, vr);
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6] = vr[0];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7] = vr[1];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + (n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8] = vr[2];

                v[0] = vertices[2 * sp * 3 * (n1 + 10) + 3 * (j % (n2 + 10) + 1) + 0];
                v[1] = vertices[2 * sp * 3 * (n1 + 10) + 3 * (j % (n2 + 10) + 1) + 1];
                v[2] = vertices[2 * sp * 3 * (n1 + 10) + 3 * (j % (n2 + 10) + 1) + 2];
                int offset = 2 * 9 * sp * (n1 + 10) * (n2 + 10);
                multMatrVec(m2, v, vr);
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0)] = vr[0];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1)] = vr[1];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2)] = vr[2];
                v[0] = vertices[2 * sp * 3 * (n1 + 10) + 3 * ((j + 1) % (n2 + 10) + 1) + 0];
                v[1] = vertices[2 * sp * 3 * (n1 + 10) + 3 * ((j + 1) % (n2 + 10) + 1) + 1];
                v[2] = vertices[2 * sp * 3 * (n1 + 10) + 3 * ((j + 1) % (n2 + 10) + 1) + 2];
                multMatrVec(m1, v, vr);
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3)] = vr[0];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4)] = vr[1];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5)] = vr[2];
                multMatrVec(m2, v, vr);
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6)] = vr[0];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7)] = vr[1];
                vertices[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8)] = vr[2];
            }
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    pitch = false;
    if (yoffset < 0) {
        camera.pitch += rot * rotVCoef;
        if (camera.pitch > 80) {
            camera.pitch = 80;
        }
        pitch = true;
    }
    if (yoffset > 0) {
        camera.pitch -= rot * rotVCoef;
        if (camera.pitch < -80) {
            camera.pitch = -80;
        }
        pitch = true;
    }
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
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
    if (key == GLFW_KEY_EQUAL && action == GLFW_RELEASE) {
        cl = 1;
        updArr = true;
    }
    if (key == GLFW_KEY_MINUS && action == GLFW_RELEASE) {
        cl = -1;
        updArr = true;
    }
}

void rotate() {
    glm::mat4 rotModel{1.0f};
    if (rotation[0]) {
        rModel = glm::rotate(rotModel, glm::radians(rot), glm::vec3(0.0, 0.0, 1.0)) * rModel;
    }
    if (rotation[1]) {
        rModel = glm::rotate(rotModel, glm::radians(-rot), glm::vec3(0.0, 0.0, 1.0)) * rModel;
    }
    if (rotation[2]) {
        rModel = glm::rotate(rotModel, glm::radians(rot), glm::vec3(0.0, 1.0, 0.0)) * rModel;
    }
    if (rotation[3]) {
        rModel = glm::rotate(rotModel, glm::radians(-rot), glm::vec3(0.0, 1.0, 0.0)) * rModel;
    }
    if (rotation[4]) {
        rModel = glm::rotate(rotModel, glm::radians(-rot), glm::vec3(1.0, 0.0, 0.0)) * rModel;
    }
    if (rotation[5]) {
        rModel = glm::rotate(rotModel, glm::radians(rot), glm::vec3(1.0, 0.0, 0.0)) * rModel;
    }
}

void vertexPrep(unsigned int VBO, unsigned int VAO1, unsigned int VAO2, unsigned int VAO3, GLfloat *vertices) {
    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + 4 * 9 * sp * (n1 + 10) * (n2 + 10)) * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + 4 * 9 * sp * (n1 + 10) * (n2 + 10)) * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)((2 * sp * (n1 + 10) * 3) * sizeof(float)));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(VAO3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + 4 * 9 * sp * (n1 + 10) * (n2 + 10)) * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)((2 * sp * (n1 + 10) * 3 + (n2 + 12) * 3) * sizeof(float)));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    auto *frene = new glm::mat4[2 * sp * (n1 + 10)];
    calculateFrene(frene);
    auto *vertices = new GLfloat[2 * sp * 3 * (n1 + 10) + (n2 + 12) * 3 + 4 * 9 * sp * (n1 + 10) * (n2 + 10)];
    calculateVertices(frene, vertices);

    std::string vShaderCode;
    std::string fShaderCode;
    std::ifstream vShader(R"(C:\CLionProjects\shaders\shaders\shader.vs)");
    std::ifstream fShader(R"(C:\CLionProjects\shaders\shaders\shader.frag)");
    std::stringstream vSStream, fSStream;
    vSStream << vShader.rdbuf();
    fSStream << fShader.rdbuf();
    vShader.close();
    fShader.close();
    vShaderCode = vSStream.str();
    fShaderCode = fSStream.str();
    const char* vertexShaderCode = vShaderCode.c_str();
    const char* fragmentShaderCode = fShaderCode.c_str();

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);

    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO, VAO1, VAO2, VAO3;
    glGenVertexArrays(1, &VAO1);
    glGenVertexArrays(1, &VAO2);
    glGenVertexArrays(1, &VAO3);
    glGenBuffers(1, &VBO);

    vertexPrep(VBO, VAO1, VAO2, VAO3, vertices);

    while (!glfwWindowShouldClose(window)) {
        cur = float(glfwGetTime());
        delta = cur - prev;
        prev = cur;
        speed = basicSpeed * delta;
        rot = basicRot * delta;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);
        rotate();
        glm::mat4 lookAtM = lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
               camera.posX, camera.posY, camera.posZ, 0, 1, 0);
        glUseProgram(shaderProgram);
        int rMod = glGetUniformLocation(shaderProgram, "rModel");
        glUniformMatrix4fv(rMod, 1, GL_FALSE, glm::value_ptr(rModel));
        int proj = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(frustum));
        int la = glGetUniformLocation(shaderProgram, "lookAt");
        glUniformMatrix4fv(la, 1, GL_FALSE, glm::value_ptr(lookAtM));
        int mo = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(mo, 1, GL_FALSE, glm::value_ptr(glm::mat4{1.0f}));
        glBindVertexArray(VAO1);
        glDrawArrays(GL_LINE_STRIP, 0, 2 * sp * (n1 + 10));
        glBindVertexArray(VAO3);
        glDrawArrays(GL_TRIANGLES, 0, 4 * 3 * sp * (n1 + 10) * (n2 + 10));
        glDisable(GL_CULL_FACE);
        glBindVertexArray(VAO2);
        for (int i = 0; i < 2 * sp * (n1 + 10); i++) {
            glUniformMatrix4fv(mo, 1, GL_FALSE, glm::value_ptr(frene[i]));
            glDrawArrays(GL_TRIANGLE_FAN, 0, (n2 + 12));
        }
        if (cage > 0) {
            glEnable(GL_CULL_FACE);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        updCamera();
        if (updArr) {
            delete[] vertices;
            if (cl == -1) {
                n2 = (n2 + 10) % 50;
                cl = 0;
            }
            if (cl == 1) {
                delete[] frene;
                n1 = (n1 + 10) % 50;
                frene = new glm::mat4[2 * sp * (n1 + 10)];
                calculateFrene(frene);
                cl = 0;
            }
            vertices = new GLfloat[2 * sp * (n1 + 10) * 3 + (n2 + 12) * 3 + 4 * 9 * sp * (n1 + 10) * (n2 + 10)];
            calculateVertices(frene, vertices);
            vertexPrep(VBO, VAO1, VAO2, VAO3, vertices);
            updArr = false;
        }
    }
    glDeleteVertexArrays(1, &VAO1);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteVertexArrays(1, &VAO3);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    delete[] frene;
    delete[] vertices;
    return 0;
}