#include <iostream>
#include <cmath>
#include <fstream>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "json.h"
#define PI 3.1415926535897932

//управление:
//WASD, LSHIFT, LCTRL - движение камеры
//QE, MOUSE WHEEL - вращение камеры
//ARROWS, R(L)MB - вращение объекта
//Enter - текстурирование
//RCTRL - модуляция при текстурировании
//RALT - изменение расчета отражений
//LALT - глобальный эмбиент

#define GL_MULTISAMPLE 0x809D

bool rotation[6] = {false, false, false, false, false, false};
bool yaw[2] = {false, false};
bool pitch = false;
bool yaw1 = false;
bool wasdsc[6] = {false, false, false, false, false, false};
bool texture = false;
bool modulate = false;
bool localViewer = true;
bool globalAmbient = false;
float rot = 0;
float speed = 0;
float delta = 0;
float prev = 0;
float cur = 0;
float tween = -1;
int coef = -1;
float emissionCoef = 0;

GLint WIDTH = 950;
GLint HEIGHT = 950;
float rotVCoef = 2;
float basicRot = 40;
float basicSpeed = 30;
float a1 = 10;
float b1 = 8;
float a2 = 4;
float b2 = 3;
int sp = 2; //число полных витков * 2
int n1 = 90; // число сегментов в витке: n1 + 10
int n2 = 90; // число сегментов в эллипсе: n2 + 10
float lo = 1.5; //удлинение спирали
float s = 20;
float spCut = 20;
float spExp = 10;
float linAtt = 0.01;
float twinDur = 7;
float shininess = 128;

GLfloat rModel[16] = {1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      0, 0, 0, 1};
GLfloat clearColor[] = {0, 0, 0, 1};
GLfloat frustum[] = {-0.05, 0.05, -0.05, 0.05, 0.1, 1000};

GLfloat global_ambientt[] = {0, 0.5, 0, 1};
GLfloat global_ambientf[] = {0, 0, 0, 1};

GLfloat light0_direction[] = {0, 0, 1, 0};
GLfloat light0_ambient[] = {1, 1, 1, 0};
GLfloat light0_diffuse[] = {1, 1, 0.9, 0};
GLfloat light0_specular[] = {0, 0, 0, 1};

GLfloat light1_direction[] = {0, 0, 50, 1};
GLfloat light1_ambient[] = {0, 0, 0, 1};
GLfloat light1_diffuse[] = {0, 0, 0, 1};
GLfloat light1_specular[] = {1, 1, 1, 1};
GLfloat light1_spot_dir[] = {0, 0, -1};

GLfloat mat_spec[] = {1, 1, 1, 1};
GLfloat mat_amb_dif1[] = {0.6, 0.4, 0, 1};
GLfloat mat_amb_dif2[] = {0, 1, 1, 1};
GLfloat mat_amb_dif3[] = {0.45, 0.45, 0.45, 1};
GLfloat mat_amb_dif[] = {0.6, 0.4, 0, 1};
GLfloat emission[] = {0, 0, 0, 1};

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

void save() {
    nlohmann::json j;

    j["WIDTH"] = WIDTH;
    j["HEIGHT"] = HEIGHT;
    j["rotVCoef"] = rotVCoef;
    j["basicRot"] = basicRot;
    j["basicSpeed"] = basicSpeed;
    j["a1"] = a1;
    j["b1"] = b1;
    j["a2"] = a2;
    j["b2"] = b2;
    j["sp"] = sp;
    j["n1"] = n1;
    j["n2"] = n2;
    j["lo"] = lo;
    j["s"] = s;
    j["spCut"] = spCut;
    j["spExp"] = spExp;
    j["linAtt"] = linAtt;
    j["twinDur"] = twinDur;
    j["shininess"] = shininess;
    j["rModel"] = rModel;
    j["clearColor"] = clearColor;
    j["frustum"] = frustum;
    j["global_ambientt"] = global_ambientt;
    j["global_ambientf"] = global_ambientf;
    j["light0_direction"] = light0_direction;
    j["light0_ambient"] = light0_ambient;
    j["light0_diffuse"] = light0_diffuse;
    j["light0_specular"] = light0_specular;
    j["light1_direction"] = light1_direction;
    j["light1_ambient"] = light1_ambient;
    j["light1_diffuse"] = light1_diffuse;
    j["light1_specular"] = light1_specular;
    j["light1_spot_dir"] = light1_spot_dir;
    j["mat_spec"] = mat_spec;
    j["mat_amb_dif1"] = mat_amb_dif1;
    j["mat_amb_dif2"] = mat_amb_dif2;
    j["mat_amb_dif3"] = mat_amb_dif3;
    j["mat_amb_dif"] = mat_amb_dif;
    j["emission"] = emission;
    j["camera"] = {{"posX", camera.posX}, {"posY", camera.posY}, {"posZ", camera.posZ},
                   {"stdDirX", camera.stdDirX}, {"stdDirY", camera.stdDirY}, {"stdDirZ", camera.stdDirZ},
                   {"stdRX", camera.stdRX}, {"stdRY", camera.stdRY}, {"stdRZ", camera.stdRZ},
                   {"yaw", camera.yaw}, {"pitch", camera.pitch}};

    std::ofstream o(R"(C:\CLionProjects\realisticTube\config\curConfig.json)");
    o << j << std::endl;
    o.close();
}

void load() {
    nlohmann::json j;
    std::ifstream i(R"(C:\CLionProjects\realisticTube\config\curConfig.json)");
    i >> j;
    i.close();
    WIDTH = j["WIDTH"];
    HEIGHT = j["HEIGHT"];
    rotVCoef = j["rotVCoef"];
    basicRot = j["basicRot"];
    basicSpeed = j["basicSpeed"];
    a1 = j["a1"];
    b1 = j["b1"];
    a2 = j["a2"];
    b2 = j["b2"];
    sp = j["sp"];
    n1 = j["n1"];
    n2 = j["n2"];
    lo = j["lo"];
    s = j["s"];
    spCut = j["spCut"];
    spExp = j["spExp"];
    linAtt = j["linAtt"];
    twinDur = j["twinDur"];
    shininess = j["shininess"];
    for (int k = 0; k < 16; k++) {
        rModel[k] = j["rModel"][k];
    }
    for (int k = 0; k < 6; k++) {
        frustum[k] = j["frustum"][k];
    }
    for(int k = 0; k < 4; k++) {
        clearColor[k] = j["clearColor"][k];
        global_ambientt[k] = j["global_ambientt"][k];
        global_ambientf[k] = j["global_ambientf"][k];
        light0_direction[k] = j["light0_direction"][k];
        light0_ambient[k] = j["light0_ambient"][k];
        light0_diffuse[k] = j["light0_diffuse"][k];
        light0_specular[k] = j["light0_specular"][k];
        light1_direction[k] = j["light1_direction"][k];
        light1_ambient[k] = j["light1_ambient"][k];
        light1_diffuse[k] = j["light1_diffuse"][k];
        light1_specular[k] = j["light1_specular"][k];
        mat_spec[k] = j["mat_spec"][k];
        mat_amb_dif1[k] = j["mat_amb_dif1"][k];
        mat_amb_dif2[k] = j["mat_amb_dif2"][k];
        mat_amb_dif3[k] = j["mat_amb_dif3"][k];
        mat_amb_dif[k] = j["mat_amb_dif"][k];
        emission[k] = j["emission"][k];
    }
    for (int k = 0; k < 3; k++) {
        light1_spot_dir[k] = j["light1_spot_dir"][k];
    }
    camera.posX = j["camera"]["posX"];
    camera.posY = j["camera"]["posY"];
    camera.posZ = j["camera"]["posZ"];
    camera.stdDirX = j["camera"]["stdDirX"];
    camera.stdDirY = j["camera"]["stdDirY"];
    camera.stdDirZ = j["camera"]["stdDirZ"];
    camera.stdRX = j["camera"]["stdRX"];
    camera.stdRY = j["camera"]["stdRY"];
    camera.stdRZ = j["camera"]["stdRZ"];
    camera.yaw = j["camera"]["yaw"];
    camera.pitch = j["camera"]["pitch"];
}

void multMatrVec(const GLfloat m1[16], const GLfloat v[4], GLfloat vr[4]) {
    for (int i = 0; i < 4; i++) {
        vr[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            vr[j] += m1[i * 4 + j] * v[i];
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

void drawSystem() {
    glDisable(GL_TEXTURE_2D);
    glPointSize(1);
    GLfloat color1[] = {1, 0, 0, 1};
    GLfloat color2[] = {0, 0, 1, 1};
    GLfloat color3[] = {0, 1, 0, 1};
    GLfloat tempColor[] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tempColor);
    glBegin(GL_LINES);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color1);
    glVertex3f(0, 0, 0);
    glVertex3f(s, 0, 0);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color2);
    glVertex3f(0, 0, 0);
    glVertex3f(0, s, 0);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color3);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, s);
    glEnd();
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_dif);
    if (texture) {
        glEnable(GL_TEXTURE_2D);
    }
}

void calculateTube(GLfloat *spine) {
    for (int k = -sp; k < sp; k++) {
        float angle = 0;
        for (int i = 0; i < (n1 + 10); i++) {
            spine[((n1 + 10) * (k + sp) + i) * 3 + 0] = a1 * cosf(angle);
            spine[((n1 + 10) * (k + sp) + i) * 3 + 1] = b1 * sinf(angle);
            spine[((n1 + 10) * (k + sp) + i) * 3 + 2] = lo * angle + 2 * lo * PI * k;
            angle += float(2 * PI / (n1 + 10));
        }
    }
}

void calculateFrene(GLfloat **frene) {
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
            for (int j =0; j < 16; j++) {
                frene[(n1 + 10) * (k + sp) + i][j] = vF[j];
            }
            angle += float(2 * PI / (n1 + 10));
        }
    }
}

void calculateEllipse(GLfloat **ellipse) {
    float angle = 0;
    for (int i = 0; i < (n2 + 10); i++) {
        ellipse[i][0] = a2 * cosf(angle);
        ellipse[i][1] = b2 * sinf(angle);
        ellipse[i][2] = 0;
        angle += float(2 * PI / (n2 + 10));
    }
}

void calculatePolygons(GLfloat **ellipse, GLfloat **frene, GLfloat *polygons, GLfloat *normals, GLfloat *spine, GLfloat *textures) {
    GLfloat vn[3] = {0};
    float counter1 = 1;
    float step1 = float(4) / (n1 + 10);
    float counter = 0;
    float step = float(1) / (n2 + 10);
    for (int k = -sp; k < sp; k++) {
        for (int i = 0; i < (n1 + 10); i++) {
            for (int j = 0; j < (n2 + 10); j++) {
                GLfloat vr[4] = {0};
                GLfloat *m1 = frene[(n1 + 10) * (k + sp) + i];
                int mi1 = (n1 + 10) * (k + sp) + i;
                GLfloat *m2;
                int mi2;
                if (i == (n1 + 10) - 1) {
                    if (k < sp - 1) {
                        m2 = frene[(n1 + 10) * (k + 1 + sp)];
                        mi2 = (n1 + 10) * (k + 1 + sp);
                    } else {
                        m2 = m1;
                        mi2 = mi1;
                    }
                } else {
                    m2 = frene[(n1 + 10) * (k + sp) + i + 1];
                    mi2 = (n1 + 10) * (k + sp) + i + 1;
                }
                GLfloat v[4] = {ellipse[(j) % (n2 + 10)][0],
                                ellipse[(j) % (n2 + 10)][1],
                                ellipse[(j) % (n2 + 10)][2], 1};

                multMatrVec(m2, v, vr);
                textures[(n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 0] = counter;
                textures[(n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 1] = counter1 - step1;
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0] = vr[0];
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1] = vr[1];
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2] = vr[2];
                vn[0] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0] - spine[mi2 * 3 + 0];
                vn[1] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1] - spine[mi2 * 3 + 1];
                vn[2] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2] - spine[mi2 * 3 + 2];
                normalise(vn);
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0] = vn[0];
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1] = vn[1];
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2] = vn[2];
                multMatrVec(m1, v, vr);
                textures[(n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 2] = counter;
                textures[(n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 3] = counter1;
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3] = vr[0];
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4] = vr[1];
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5] = vr[2];
                vn[0] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3] - spine[mi1 * 3 + 0];
                vn[1] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4] - spine[mi1 * 3 + 1];
                vn[2] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5] - spine[mi1 * 3 + 2];
                normalise(vn);
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3] = vn[0];
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4] = vn[1];
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5] = vn[2];
                v[0] = ellipse[(j + 1) % (n2 + 10)][0];
                v[1] = ellipse[(j + 1) % (n2 + 10)][1];
                v[2] = ellipse[(j + 1) % (n2 + 10)][2];
                multMatrVec(m1, v, vr);
                textures[(n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 4] = counter + step;
                textures[(n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 5] = counter1;
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6] = vr[0];
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7] = vr[1];
                polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8] = vr[2];
                vn[0] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6] - spine[mi1 * 3 + 0];
                vn[1] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7] - spine[mi1 * 3 + 1];
                vn[2] = polygons[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8] - spine[mi1 * 3 + 2];
                normalise(vn);
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6] = vn[0];
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7] = vn[1];
                normals[(n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8] = vn[2];

                v[0] = ellipse[(j) % (n2 + 10)][0];
                v[1] = ellipse[(j) % (n2 + 10)][1];
                v[2] = ellipse[(j) % (n2 + 10)][2];
                int offset = 2 * 9 * sp * (n1 + 10) * (n2 + 10);
                int offset1 = 2 * 6 * sp * (n1 + 10) * (n2 + 10);
                multMatrVec(m2, v, vr);
                textures[offset1 + ((n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 0)] = counter;
                textures[offset1 + ((n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 1)] = counter1 - step1;
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0)] = vr[0];
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1)] = vr[1];
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2)] = vr[2];
                vn[0] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0)] - spine[mi2 * 3 + 0];
                vn[1] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1)] - spine[mi2 * 3 + 1];
                vn[2] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2)] - spine[mi2 * 3 + 2];
                normalise(vn);
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 0)] = vn[0];
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 1)] = vn[1];
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 2)] = vn[2];
                v[0] = ellipse[(j + 1) % (n2 + 10)][0];
                v[1] = ellipse[(j + 1) % (n2 + 10)][1];
                v[2] = ellipse[(j + 1) % (n2 + 10)][2];
                multMatrVec(m1, v, vr);
                textures[offset1 + ((n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 2)] = counter + step;
                textures[offset1 + ((n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 3)] = counter1;
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3)] = vr[0];
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4)] = vr[1];
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5)] = vr[2];
                vn[0] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3)] - spine[mi1 * 3 + 0];
                vn[1] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4)] - spine[mi1 * 3 + 1];
                vn[2] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5)] - spine[mi1 * 3 + 2];
                normalise(vn);
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 3)] = vn[0];
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 4)] = vn[1];
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 5)] = vn[2];
                multMatrVec(m2, v, vr);
                textures[offset1 + ((n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 4)] = counter + step;
                textures[offset1 + ((n2 + 10) * (n1 + 10) * 6 * (k + sp) + (n2 + 10) * 6 * i + j * 6 + 5)] = counter1 - step1;
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6)] = vr[0];
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7)] = vr[1];
                polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8)] = vr[2];
                vn[0] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6)] - spine[mi2 * 3 + 0];
                vn[1] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7)] - spine[mi2 * 3 + 1];
                vn[2] = polygons[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8)] - spine[mi2 * 3 + 2];
                normalise(vn);
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 6)] = vn[0];
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 7)] = vn[1];
                normals[offset + ((n2 + 10) * (n1 + 10) * 9 * (k + sp) + (n2 + 10) * 9 * i + j * 9 + 8)] = vn[2];
                counter += step;
            }
            counter1 -= step1;
            counter = 0;
            if (counter1 <= 0) {
                counter1 = 1;
            }
        }
    }
}

void calculateEllipses(GLfloat **ellipse, GLfloat **frene, GLfloat *ellipses, GLfloat *ellNormals, GLfloat *ellTextures) {
    GLfloat vr[4] = {0};
    GLfloat v[4] = {0, 0, 0, 0};
    v[2] = -1;
    multMatrVec(frene[0], v, vr);
    ellNormals[0] = vr[0];
    ellNormals[1] = vr[1];
    ellNormals[2] = vr[2];
    ellTextures[0] = 0.5;
    ellTextures[1] = 0.5;
    v[2] = 0;
    v[3] = 1;
    multMatrVec(frene[0], v, vr);
    ellipses[0] = vr[0];
    ellipses[1] = vr[1];
    ellipses[2] = vr[2];
    for (int i = 1; i < (n2 + 11); i++) {
        ellNormals[i * 3 + 0] = ellNormals[0];
        ellNormals[i * 3 + 1] = ellNormals[1];
        ellNormals[i * 3 + 2] = ellNormals[2];
        ellTextures[i * 2 + 0] = ((ellipse[n2 + 10 - i][0] / a2) + 1) / 2;
        ellTextures[i * 2 + 1] = ((ellipse[n2 + 10 - i][1] / b2) + 1) / 2;
        v[0] = ellipse[n2 + 10 - i][0];
        v[1] = ellipse[n2 + 10 - i][1];
        v[2] = ellipse[n2 + 10 - i][2];
        multMatrVec(frene[0], v, vr);
        ellipses[i * 3 + 0] = vr[0];
        ellipses[i * 3 + 1] = vr[1];
        ellipses[i * 3 + 2] = vr[2];
    }
    ellNormals[(n2 + 11) * 3 + 0] = ellNormals[3 + 0];
    ellNormals[(n2 + 11) * 3 + 1] = ellNormals[3 + 1];
    ellNormals[(n2 + 11) * 3 + 2] = ellNormals[3 + 2];
    ellTextures[(n2 + 11) * 2 + 0] = 0.5;
    ellTextures[(n2 + 11) * 2 + 1] = 0.5;
    ellipses[(n2 + 11) * 3 + 0] = ellipses[3 + 0];
    ellipses[(n2 + 11) * 3 + 1] = ellipses[3 + 1];
    ellipses[(n2 + 11) * 3 + 2] = ellipses[3 + 2];

    v[0] = 0;
    v[1] = 0;
    v[2] = 1;
    v[3] = 0;
    multMatrVec(frene[(n1 + 10) * (2 * sp - 1) + n1 + 9], v, vr);
    ellNormals[(n2 + 12) * 3 + 0] = vr[0];
    ellNormals[(n2 + 12) * 3 + 1] = vr[1];
    ellNormals[(n2 + 12) * 3 + 2] = vr[2];
    ellTextures[(n2 + 12) * 2 + 0] = 0.5;
    ellTextures[(n2 + 12) * 2 + 1] = 0.5;
    v[2] = 0;
    v[3] = 1;
    multMatrVec(frene[(n1 + 10) * (2 * sp - 1) + n1 + 9], v, vr);
    ellipses[(n2 + 12) * 3 + 0] = vr[0];
    ellipses[(n2 + 12) * 3 + 1] = vr[1];
    ellipses[(n2 + 12) * 3 + 2] = vr[2];
    for (int i = 1; i < (n2 + 11); i++) {
        ellNormals[(n2 + 12) * 3 + i * 3 + 0] = ellNormals[(n2 + 12) * 3 + 0];
        ellNormals[(n2 + 12) * 3 + i * 3 + 1] = ellNormals[(n2 + 12) * 3 + 1];
        ellNormals[(n2 + 12) * 3 + i * 3 + 2] = ellNormals[(n2 + 12) * 3 + 2];
        ellTextures[(n2 + 12) * 2 + i * 2 + 0] = ((ellipse[n2 + 10 - i][0] / a2) + 1) / 2;
        ellTextures[(n2 + 12) * 2 + i * 2 + 1] = ((ellipse[n2 + 10 - i][1] / b2) + 1) / 2;
        v[0] = ellipse[n2 + 10 - i][0];
        v[1] = ellipse[n2 + 10 - i][1];
        v[2] = ellipse[n2 + 10 - i][2];
        multMatrVec(frene[(n1 + 10) * (2 * sp - 1) + n1 + 9], v, vr);
        ellipses[(n2 + 12) * 3 + i * 3 + 0] = vr[0];
        ellipses[(n2 + 12) * 3 + i * 3 + 1] = vr[1];
        ellipses[(n2 + 12) * 3 + i * 3 + 2] = vr[2];
    }
    ellNormals[(n2 + 12) * 3 + (n2 + 11) * 3 + 0] = ellNormals[(n2 + 12) * 3 + 3 + 0];
    ellNormals[(n2 + 12) * 3 + (n2 + 11) * 3 + 1] = ellNormals[(n2 + 12) * 3 + 3 + 1];
    ellNormals[(n2 + 12) * 3 + (n2 + 11) * 3 + 2] = ellNormals[(n2 + 12) * 3 + 3 + 2];
    ellTextures[(n2 + 12) * 2 + (n2 + 11) * 2 + 0] = 0.5;
    ellTextures[(n2 + 12) * 2 + (n2 + 11) * 2 + 1] = 0.5;
    ellipses[(n2 + 12) * 3 + (n2 + 11) * 3 + 0] = ellipses[(n2 + 12) * 3 + 3 + 0];
    ellipses[(n2 + 12) * 3 + (n2 + 11) * 3 + 1] = ellipses[(n2 + 12) * 3 + 3 + 1];
    ellipses[(n2 + 12) * 3 + (n2 + 11) * 3 + 2] = ellipses[(n2 + 12) * 3 + 3 + 2];
}

void drawTube(GLfloat *spine, GLfloat *polygons, GLfloat *normals, GLfloat *textures, GLfloat *ellipses, GLfloat *ellNormals, GLfloat *ellTextures) {
    glVertexPointer(3, GL_FLOAT, 0, spine);
    glDrawArrays(GL_LINE_STRIP, 0, 2 * sp * (n1 + 10));

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, ellipses);
    glNormalPointer(GL_FLOAT, 0, ellNormals);
    glTexCoordPointer(2, GL_FLOAT, 0, ellTextures);
    glDrawArrays(GL_TRIANGLE_FAN, 0, (n2 + 12));
    glFrontFace(GL_CW);
    glDrawArrays(GL_TRIANGLE_FAN, (n2 + 12), (n2 + 12));
    glFrontFace(GL_CCW);

    glVertexPointer(3, GL_FLOAT, 0, polygons);
    glNormalPointer(GL_FLOAT, 0, normals);
    glTexCoordPointer(2, GL_FLOAT, 0, textures);
    glDrawArrays(GL_TRIANGLES, 0, 4 * 3 * sp * (n1 + 10) * (n2 + 10));
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
    if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE) {
        texture = !texture;
    }
    if (key == GLFW_KEY_RIGHT_CONTROL && action == GLFW_RELEASE) {
        modulate = !modulate;
        if (modulate) {
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        } else {
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        }
    }
    if (key == GLFW_KEY_RIGHT_ALT && action == GLFW_RELEASE) {
        localViewer = !localViewer;
        if (localViewer) {
            glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
        } else {
            glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
        }
    }
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        globalAmbient = !globalAmbient;
        if (globalAmbient) {
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambientt);
        } else {
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambientf);
        }
    }
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE) {
        if (tween < 0) {
            tween = 0;
        }
        if (tween > 1) {
            tween = 1;
        }
        coef *= -1;
    }
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

void freeN1(GLfloat *spine, GLfloat **frene) {
    delete[] spine;
    for (int i = 0; i < 2 * sp * (n1 + 10); i++) {
        delete[] frene[i];
    }
    delete[] frene;
}

void freeN2(GLfloat **ellipse) {
    for (int i = 0; i < (n2 + 10); i++) {
        delete[] ellipse[i];
    }
    delete[] ellipse;
}

void freeN3(GLfloat *polygons, GLfloat *normals, GLfloat *textures, GLfloat *ellipses, GLfloat *ellNormals, GLfloat *ellTextures) {
    delete[] polygons;
    delete[] normals;
    delete[] textures;
    delete[] ellipses;
    delete[] ellNormals;
    delete[] ellTextures;
}

int main() {
    glfwInit();
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

    //load();

    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(R"(C:\CLionProjects\realisticTube\textures\m_m_iron02.bmp)", &width, &height, &nrChannels, 0);
    if (data) {
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LIGHTING);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambientf);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glEnable(GL_NORMALIZE);
    glDisable(GL_NORMALIZE);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spCut);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spExp);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, linAtt);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    //glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5]);
    GLfloat persp[16];
    glGetFloatv(GL_PROJECTION_MATRIX, persp);

    glMatrixMode(GL_MODELVIEW);

    auto *spine = new GLfloat[2 * 3 * sp * (n1 + 10)];
    calculateTube(spine);
    auto **frene = new GLfloat*[2 * sp * (n1 + 10)];
    for (int i = 0; i < 2 * sp * (n1 + 10); i++) {
        frene[i] = new GLfloat[16];
    }
    calculateFrene(frene);
    auto **ellipse = new GLfloat*[(n2 + 10)];
    for (int i = 0; i < (n2 + 10); i++) {
        ellipse[i] = new GLfloat[3];
    }
    calculateEllipse(ellipse);
    auto *polygons = new GLfloat[4 * 9 * sp * (n1 + 10) * (n2 + 10)];
    auto *normals = new GLfloat[4 * 9 * sp * (n1 + 10) * (n2 + 10)];
    auto *textures = new GLfloat[4 * 6 * sp * (n1 + 10) * (n2 + 10)];
    calculatePolygons(ellipse, frene, polygons, normals, spine, textures);
    auto *ellipses = new GLfloat[2 * 3 * (n2 + 12)];
    auto *ellNormals = new GLfloat[2 * 3 * (n2 + 12)];
    auto *ellTextures = new GLfloat[2 * 2 * (n2 + 12)];
    calculateEllipses(ellipse, frene, ellipses, ellNormals, ellTextures);
    glEnableClientState(GL_VERTEX_ARRAY);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(persp);
    glMatrixMode(GL_MODELVIEW);
    lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
           camera.posX, camera.posY, camera.posZ, 0, 1, 0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_direction);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_spot_dir);

    float fps[1000];
    int count = 0;
    while (!glfwWindowShouldClose(window)) {
        cur = float(glfwGetTime());
        delta = cur - prev;
        prev = cur;
        speed = basicSpeed * delta;
        rot = basicRot * delta;
        if (count < 1000) {
            fps[count] = 1 / delta;
            count += 1;
        } else {
            if (count == 1000) {
                count += 2;
                std::cout << "over";
            }
        }
        if (tween >= 0 && tween <= 1) {
            emissionCoef = tween * (1 - tween);
            mat_amb_dif[0] = pow(1 - tween, 2) * mat_amb_dif1[0] + 2 * tween * (1 - tween) * mat_amb_dif2[0] + pow(tween, 2) * mat_amb_dif3[0];
            emission[0] = mat_amb_dif[0] * emissionCoef;
            mat_amb_dif[1] = pow(1 - tween, 2) * mat_amb_dif1[1] + 2 * tween * (1 - tween) * mat_amb_dif2[1] + pow(tween, 2) * mat_amb_dif3[1];
            emission[1] = mat_amb_dif[1] * emissionCoef;
            mat_amb_dif[2] = pow(1 - tween, 2) * mat_amb_dif1[2] + 2 * tween * (1 - tween) * mat_amb_dif2[2] + pow(tween, 2) * mat_amb_dif3[2];
            emission[2] = mat_amb_dif[2] * emissionCoef;
            tween += coef * delta / twinDur;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glViewport(0, 0, screenWidth, screenHeight);
        rotate();
        glMultMatrixf(rModel);
        glGetFloatv(GL_MODELVIEW_MATRIX, rModel);
        glLoadIdentity();
        lookAt(camera.posX + camera.stdDirX, camera.posY + camera.stdDirY, camera.posZ + camera.stdDirZ,
               camera.posX, camera.posY, camera.posZ, 0, 1, 0);
        drawSystem();
        glMultMatrixf(rModel);
        drawTube(spine, polygons, normals, textures, ellipses, ellNormals, ellTextures);
        glfwSwapBuffers(window);
        glfwPollEvents();
        updCamera();
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glfwTerminate();
    float accum = 0;
    for (int i = 0; i < 1000; i++) {
        accum += fps[i];
    }
    std::cout << "\n" << accum / 1000;
    freeN1(spine, frene);
    freeN2(ellipse);
    freeN3(polygons, normals, textures, ellipses, ellNormals, ellTextures);
    //save();
    return 0;
}