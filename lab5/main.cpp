#include <cmath>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>

//управление:
//LMB - установить точку
//enter - окончание ввода
//backspace - очистка(отмена)

GLint WIDTH = 720;
GLint HEIGHT = 720;
GLfloat *buffer = nullptr;
bool enterMode = true;
bool innerMode = false;
bool clipMode = false;
bool innerClipMode = false;
bool united = false;
bool inClip = false;
int contourSize = 0;
int innerContourSize = 0;
int clipSize = 0;
int innerClipSize = 0;

struct Point {
    int x = 0;
    int y = 0;
    int type = 0; //вход/выход
    int conn1 = 0; //позиция в отсекаемом
    int conn2 = 0; //позиция в отсекателе
    int pos[2] = {0};
};

struct Color {
    int x = 0;
    int y = 0;
    int z = 0;
};

std::vector<Point> super;
std::vector<Point*> vertices[2][2];
std::vector<Point*> fullVertices[2][2];
std::vector<Point*> clipVertices[2];
std::vector<Point*> out[2][2];
std::vector<std::vector<Point*>> aux[2][2];

void setPixel(int y, int x, Color col) {
    buffer[y * WIDTH * 3 + x * 3] = col.x;
    buffer[y * WIDTH * 3 + x * 3 + 1] = col.y;
    buffer[y * WIDTH * 3 + x * 3 + 2] = col.z;
}

void setPoint(int y, int x, Color col) {
    buffer[y * WIDTH * 3 + x * 3] = col.x;
    buffer[y * WIDTH * 3 + x * 3 + 1] = col.y;
    buffer[y * WIDTH * 3 + x * 3 + 2] = col.z;
    buffer[(y + 1) * WIDTH * 3 + x * 3] = col.x;
    buffer[(y + 1) * WIDTH * 3 + x * 3 + 1] = col.y;
    buffer[(y + 1) * WIDTH * 3 + x * 3 + 2] = col.z;
    buffer[(y - 1) * WIDTH * 3 + x * 3] = col.x;
    buffer[(y - 1) * WIDTH * 3 + x * 3 + 1] = col.y;
    buffer[(y - 1) * WIDTH * 3 + x * 3 + 2] = col.z;
    buffer[y * WIDTH * 3 + (x + 1) * 3] = col.x;
    buffer[y * WIDTH * 3 + (x + 1) * 3 + 1] = col.y;
    buffer[y * WIDTH * 3 + (x + 1) * 3 + 2] = col.z;
    buffer[(y + 1) * WIDTH * 3 + (x + 1) * 3] = col.x;
    buffer[(y + 1) * WIDTH * 3 + (x + 1) * 3 + 1] = col.y;
    buffer[(y + 1) * WIDTH * 3 + (x + 1) * 3 + 2] = col.z;
    buffer[(y + 1) * WIDTH * 3 + (x - 1) * 3] = col.x;
    buffer[(y + 1) * WIDTH * 3 + (x - 1) * 3 + 1] = col.y;
    buffer[(y + 1) * WIDTH * 3 + (x - 1) * 3 + 2] = col.z;
    buffer[y * WIDTH * 3 + (x - 1) * 3] = col.x;
    buffer[y * WIDTH * 3 + (x - 1) * 3 + 1] = col.y;
    buffer[y * WIDTH * 3 + (x - 1) * 3 + 2] = col.z;
    buffer[(y - 1) * WIDTH * 3 + (x - 1) * 3] = col.x;
    buffer[(y - 1) * WIDTH * 3 + (x - 1) * 3 + 1] = col.y;
    buffer[(y - 1) * WIDTH * 3 + (x - 1) * 3 + 2] = col.z;
    buffer[(y - 1) * WIDTH * 3 + (x + 1) * 3] = col.x;
    buffer[(y - 1) * WIDTH * 3 + (x + 1) * 3 + 1] = col.y;
    buffer[(y - 1) * WIDTH * 3 + (x + 1) * 3 + 2] = col.z;
}

float len(int x1, int y1, int x2, int y2) {
    return sqrtf(float((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
}

void insertion(int pos, Point* p, int t, int s) {
    int end = clipVertices[s].size();
    int res = clipVertices[s].size();
    for (int i = pos; i < end; i++) {
        if (len(clipVertices[s][pos - 1]->x, clipVertices[s][pos - 1]->y, p->x, p->y) <
            len(clipVertices[s][pos - 1]->x, clipVertices[s][pos - 1]->y, clipVertices[s][i]->x, clipVertices[s][i]->y)) {
            res = i;
            break;
        }
    }
    p->conn2 = res;
    auto it = clipVertices[s].begin();
    clipVertices[s].insert(it + res, p);
    for (int i = res + 1; i < clipVertices[s].size(); i++) {
        clipVertices[s][i]->conn2 += 1;
    }
    if (t == 0) {
        clipVertices[s][clipVertices[s].size() - 1]->conn2 = 0;
    }
}

void insertion2(int ind, Point* p, int k, int s) {
    int end = aux[s][k][ind].size();
    int res = aux[s][k][ind].size();
    for (int i = 0; i < end; i++) {
        if (len(vertices[s][k][ind]->x, vertices[s][k][ind]->y, p->x, p->y) <
            len(vertices[s][k][ind]->x, vertices[s][k][ind]->y, aux[s][k][ind][i]->x, aux[s][k][ind][i]->y)) {
            res = i;
            break;
        }
    }
    auto it = aux[s][k][ind].begin();
    aux[s][k][ind].insert(it + res, p);
}

int cross(int x1, int y1, int x2, int y2) {
    return x1 * y2 - x2 * y1;
}

int dotProd(int x1, int y1, int x2, int y2) {
    return x1 * x2 + y1 * y2;
}

void intersection(int x1, int y1, int x2, int y2, int pos, int t, int s) {
    for (int k = 0; k < 2; k++) {
        if (!vertices[s][k].empty()) {
            for (int i = 0; i < vertices[s][k].size() - 1; i++) {
                int cr1 = cross(x2 - x1, y2 - y1, vertices[s][k][i]->x - x1, vertices[s][k][i]->y - y1);
                int cr2 = cross(x2 - x1, y2 - y1, vertices[s][k][i + 1]->x - x1, vertices[s][k][i + 1]->y - y1);
                int c1 = cr1 < 0 ? -1 : 1;
                int c2 = cr2 < 0 ? -1 : 1;
                int cr3 = cross(vertices[s][k][i + 1]->x - vertices[s][k][i]->x, vertices[s][k][i + 1]->y - vertices[s][k][i]->y, x1 - vertices[s][k][i]->x, y1 - vertices[s][k][i]->y);
                int cr4 = cross(vertices[s][k][i + 1]->x - vertices[s][k][i]->x, vertices[s][k][i + 1]->y - vertices[s][k][i]->y, x2 - vertices[s][k][i]->x, y2 - vertices[s][k][i]->y);
                int c3 = cr3 < 0 ? -1 : 1;
                int c4 = cr4 < 0 ? -1 : 1;
                if (c1 * c2 < 0 && c3 * c4 < 0) {
                    int x = int(vertices[s][k][i]->x + (vertices[s][k][i + 1]->x - vertices[s][k][i]->x) * abs(cr1) / abs(cr2 - cr1));
                    int y = int(vertices[s][k][i]->y + (vertices[s][k][i + 1]->y - vertices[s][k][i]->y) * abs(cr1) / abs(cr2 - cr1));
                    if (dotProd(x2 - x1, y2 - y1, (vertices[s][k][i + 1]->y - vertices[s][k][i]->y), -1 * (vertices[s][k][i + 1]->x - vertices[s][k][i]->x)) < 0) {
                        setPoint(y, x, Color{1, 0, 0});
                        Point p{x, y, 2, 0, 0, 0};
                        super.push_back(p);
                        insertion(pos, &(super[super.size() - 1]), t, s);
                        insertion2(i, &(super[super.size() - 1]), k, s);
                    } else {
                        setPoint(y, x, Color{1, 1, 0});
                        Point p{x, y, 1, 0, 0, 0};
                        super.push_back(p);
                        insertion(pos, &(super[super.size() - 1]), t, s);
                        insertion2(i, &(super[super.size() - 1]), k, s);
                    }
                }
            }
            int cr1 = cross(x2 - x1, y2 - y1, vertices[s][k][vertices[s][k].size() - 1]->x - x1, vertices[s][k][vertices[s][k].size() - 1]->y - y1);
            int cr2 = cross(x2 - x1, y2 - y1, vertices[s][k][0]->x - x1, vertices[s][k][0]->y - y1);
            int c1 = cr1 < 0 ? -1 : 1;
            int c2 = cr2 < 0 ? -1 : 1;
            int cr3 = cross(vertices[s][k][0]->x - vertices[s][k][vertices[s][k].size() - 1]->x, vertices[s][k][0]->y - vertices[s][k][vertices[s][k].size() - 1]->y, x1 - vertices[s][k][vertices[s][k].size() - 1]->x, y1 - vertices[s][k][vertices[s][k].size() - 1]->y);
            int cr4 = cross(vertices[s][k][0]->x - vertices[s][k][vertices[s][k].size() - 1]->x, vertices[s][k][0]->y - vertices[s][k][vertices[s][k].size() - 1]->y, x2 - vertices[s][k][vertices[s][k].size() - 1]->x, y2 - vertices[s][k][vertices[s][k].size() - 1]->y);
            int c3 = cr3 < 0 ? -1 : 1;
            int c4 = cr4 < 0 ? -1 : 1;
            if (c1 * c2 < 0 && c3 * c4 < 0) {
                int x = int(vertices[s][k][vertices[s][k].size() - 1]->x + (vertices[s][k][0]->x - vertices[s][k][vertices[s][k].size() - 1]->x) * abs(cr1) / abs(cr2 - cr1));
                int y = int(vertices[s][k][vertices[s][k].size() - 1]->y + (vertices[s][k][0]->y - vertices[s][k][vertices[s][k].size() - 1]->y) * abs(cr1) / abs(cr2 - cr1));
                if (dotProd(x2 - x1, y2 - y1, (vertices[s][k][0]->y - vertices[s][k][vertices[s][k].size() - 1]->y), -1 * (vertices[s][k][0]->x - vertices[s][k][vertices[s][k].size() - 1]->x)) < 0) {
                    setPoint(y, x, Color{1, 0, 0});
                    Point p{x, y, 2, 0, 0, 0};
                    super.push_back(p);
                    insertion(pos, &(super[super.size() - 1]), t, s);
                    insertion2(vertices[s][k].size() - 1, &(super[super.size() - 1]), k, s);
                } else {
                    setPoint(y, x, Color{1, 1, 0});
                    Point p{x, y, 1, 0, 0, 0};
                    super.push_back(p);
                    insertion(pos, &(super[super.size() - 1]), t, s);
                    insertion2(vertices[s][k].size() - 1, &(super[super.size() - 1]), k, s);
                }
            }
        }
    }
}

void brezenhem(int x1, int y1, int x2, int y2, Color col) {
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;
    int error = deltaX - deltaY;
    setPixel(y2, x2, col);
    while(x1 != x2 || y1 != y2) {
        setPixel(y1, x1, col);
        int error2 = error * 2;
        if(error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if(error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

bool intersect(int x1, int y1, int x2, int y2, int lx1, int ly1, int lx2, int ly2) {
    int cr1 = cross(x2 - x1, y2 - y1, lx1 - x1, ly1 - y1);
    int cr2 = cross(x2 - x1, y2 - y1, lx2 - x1, ly2 - y1);
    int c1 = cr1 < 0 ? -1 : 1;
    int c2 = cr2 < 0 ? -1 : 1;
    int cr3 = cross(lx2 - lx1, ly2 - ly1, x1 - lx1, y1 - ly1);
    int cr4 = cross(lx2 - lx1, ly2 - ly1, x2 - lx1, y2 - ly1);
    int c3 = cr3 < 0 ? -1 : 1;
    int c4 = cr4 < 0 ? -1 : 1;
    return c1 * c2 < 0 && c3 * c4 < 0;
}

void noIntersectionDraw() {
    int count = 0;
    if (clipVertices[0].size() == clipSize) {
        count = 0;
        for (int i = 0; i < vertices[0][0].size() - 1; i++) {
            if (intersect(0, 0, clipVertices[0][0]->x, clipVertices[0][0]->y,
                          vertices[0][0][i]->x, vertices[0][0][i]->y, vertices[0][0][i + 1]->x, vertices[0][0][i + 1]->y)) {
                count += 1;
            }
        }
        if (intersect(0, 0, clipVertices[0][0]->x, clipVertices[0][0]->y,
                      vertices[0][0][vertices[0][0].size() - 1]->x, vertices[0][0][vertices[0][0].size() - 1]->y, vertices[0][0][0]->x, vertices[0][0][0]->y)) {
            count += 1;
        }
        if (!vertices[0][1].empty()) {
            for (int i = 0; i < vertices[0][1].size() - 1; i++) {
                if (intersect(0, 0, clipVertices[0][0]->x, clipVertices[0][0]->y,
                              vertices[0][1][i]->x, vertices[0][1][i]->y, vertices[0][1][i + 1]->x, vertices[0][1][i + 1]->y)) {
                    count += 1;
                }
            }
            if (intersect(0, 0, clipVertices[0][0]->x, clipVertices[0][0]->y,
                          vertices[0][1][vertices[0][1].size() - 1]->x, vertices[0][1][vertices[0][1].size() - 1]->y, vertices[0][1][0]->x, vertices[0][1][0]->y)) {
                count += 1;
            }
        }
        if (count % 2 == 0) {
            int s = clipVertices[0].size() - 1;
            for (int i = 0; i < s; i++) {
                brezenhem(clipVertices[0][i]->x, clipVertices[0][i]->y, clipVertices[0][i + 1]->x, clipVertices[0][i + 1]->y, Color{0, 1, 1});
            }
            brezenhem(clipVertices[0][s]->x, clipVertices[0][s]->y, clipVertices[0][0]->x, clipVertices[0][0]->y, Color{0, 1, 1});
        }
    }
    if (clipVertices[1].size() == innerClipSize && !clipVertices[1].empty()) {
        count = 0;
        for (int i = 0; i < vertices[1][0].size() - 1; i++) {
            if (intersect(0, 0, clipVertices[1][0]->x, clipVertices[1][0]->y,
                          vertices[1][0][i]->x, vertices[1][0][i]->y, vertices[1][0][i + 1]->x, vertices[1][0][i + 1]->y)) {
                count += 1;
            }
        }
        if (intersect(0, 0, clipVertices[1][0]->x, clipVertices[1][0]->y,
                      vertices[1][0][vertices[1][0].size() - 1]->x, vertices[1][0][vertices[1][0].size() - 1]->y, vertices[1][0][0]->x, vertices[1][0][0]->y)) {
            count += 1;
        }
        if (!vertices[1][1].empty()) {
            for (int i = 0; i < vertices[1][1].size() - 1; i++) {
                if (intersect(0, 0, clipVertices[1][0]->x, clipVertices[1][0]->y,
                              vertices[1][1][i]->x, vertices[1][1][i]->y, vertices[1][1][i + 1]->x, vertices[1][1][i + 1]->y)) {
                    count += 1;
                }
            }
            if (intersect(0, 0, clipVertices[1][0]->x, clipVertices[1][0]->y,
                          vertices[0][1][vertices[0][1].size() - 1]->x, vertices[0][1][vertices[0][1].size() - 1]->y, vertices[0][1][0]->x, vertices[0][1][0]->y)) {
                count += 1;
            }
        }
        if (count % 2 == 0) {
            int s = clipVertices[1].size() - 1;
            for (int i = 0; i < s; i++) {
                brezenhem(clipVertices[1][i]->x, clipVertices[1][i]->y, clipVertices[1][i + 1]->x, clipVertices[1][i + 1]->y, Color{0, 1, 1});
            }
            brezenhem(clipVertices[1][s]->x, clipVertices[1][s]->y, clipVertices[1][0]->x, clipVertices[1][0]->y, Color{0, 1, 1});
        }
    }
    if (fullVertices[0][0].size() == contourSize && fullVertices[1][0].size() == contourSize) {
        count = 0;
        for (int i = 0; i < clipVertices[0].size() - 1; i++) {
            if (intersect(0, 0, fullVertices[0][0][0]->x, fullVertices[0][0][0]->y,
                          clipVertices[0][i]->x, clipVertices[0][i]->y, clipVertices[0][i + 1]->x, clipVertices[0][i + 1]->y)) {
                count += 1;
            }
        }
        if (intersect(0, 0, fullVertices[0][0][0]->x, fullVertices[0][0][0]->y,
                      clipVertices[0][clipVertices[0].size() - 1]->x, clipVertices[0][clipVertices[0].size() - 1]->y, clipVertices[0][0]->x, clipVertices[0][0]->y)) {
            count += 1;
        }
        if (!clipVertices[1].empty()) {
            for (int i = 0; i < clipVertices[1].size() - 1; i++) {
                if (intersect(0, 0, fullVertices[0][0][0]->x, fullVertices[0][0][0]->y,
                              clipVertices[1][i]->x, clipVertices[1][i]->y, clipVertices[1][i + 1]->x, clipVertices[1][i + 1]->y)) {
                    count += 1;
                }
            }
            if (intersect(0, 0, fullVertices[0][0][0]->x, fullVertices[0][0][0]->y,
                          clipVertices[1][clipVertices[1].size() - 1]->x, clipVertices[1][clipVertices[1].size() - 1]->y, clipVertices[1][0]->x, clipVertices[1][0]->y)) {
                count += 1;
            }
        }
        if (count % 2 == 0) {
            int s = fullVertices[0][0].size() - 1;
            for (int i = 0; i < s; i++) {
                brezenhem(fullVertices[0][0][i]->x, fullVertices[0][0][i]->y, fullVertices[0][0][i + 1]->x, fullVertices[0][0][i + 1]->y, Color{1, 1, 1});
            }
            brezenhem(fullVertices[0][0][s]->x, fullVertices[0][0][s]->y, fullVertices[0][0][0]->x, fullVertices[0][0][0]->y, Color{1, 1, 1});
        }
    }
    if (fullVertices[0][1].size() == innerContourSize && fullVertices[1][1].size() == innerContourSize && !fullVertices[0][1].empty() && !fullVertices[1][1].empty()) {
        count = 0;
        for (int i = 0; i < clipVertices[0].size() - 1; i++) {
            if (intersect(0, 0, fullVertices[0][1][0]->x, fullVertices[0][1][0]->y,
                          clipVertices[0][i]->x, clipVertices[0][i]->y, clipVertices[0][i + 1]->x, clipVertices[0][i + 1]->y)) {
                count += 1;
            }
        }
        if (intersect(0, 0, fullVertices[0][1][0]->x, fullVertices[0][1][0]->y,
                      clipVertices[0][clipVertices[0].size() - 1]->x, clipVertices[0][clipVertices[0].size() - 1]->y, clipVertices[0][0]->x, clipVertices[0][0]->y)) {
            count += 1;
        }
        if (!clipVertices[1].empty()) {
            for (int i = 0; i < clipVertices[1].size() - 1; i++) {
                if (intersect(0, 0, fullVertices[0][1][0]->x, fullVertices[0][1][0]->y,
                              clipVertices[1][i]->x, clipVertices[1][i]->y, clipVertices[1][i + 1]->x, clipVertices[1][i + 1]->y)) {
                    count += 1;
                }
            }
            if (intersect(0, 0, fullVertices[0][1][0]->x, fullVertices[0][1][0]->y,
                          clipVertices[1][clipVertices[1].size() - 1]->x, clipVertices[1][clipVertices[1].size() - 1]->y, clipVertices[1][0]->x, clipVertices[1][0]->y)) {
                count += 1;
            }
        }
        if (count % 2 == 0) {
            int s = fullVertices[0][1].size() - 1;
            for (int i = 0; i < s; i++) {
                brezenhem(fullVertices[0][1][i]->x, fullVertices[0][1][i]->y, fullVertices[0][1][i + 1]->x, fullVertices[0][1][i + 1]->y, Color{1, 1, 1});
            }
            brezenhem(fullVertices[0][1][s]->x, fullVertices[0][1][s]->y, fullVertices[0][1][0]->x, fullVertices[0][1][0]->y, Color{1, 1, 1});
        }
    }
}

void initBuffer() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            buffer[i * WIDTH * 3 + j * 3] = 0;
            buffer[i * WIDTH * 3 + j * 3 + 1] = 0;
            buffer[i * WIDTH * 3 + j * 3 + 2] = 0;
        }
    }
}

void unite(int k, int s) {
    for (int i = 0; i < out[s][k].size(); i++) {
        inClip = false;
        Point *initP = out[s][k][i];
        Point *curP = initP;
        Point *nextP;
        if (curP->conn1 + 1 == fullVertices[s][k].size()) {
            nextP = fullVertices[s][k][0];
        } else {
            nextP = fullVertices[s][k][curP->conn1 + 1];
        }
        while(initP != nextP) {
            if (inClip) {
                brezenhem(curP->x, curP->y, nextP->x, nextP->y, Color{0, 1, 1});
            } else {
                brezenhem(curP->x, curP->y, nextP->x, nextP->y, Color{1, 1, 1});
            }
            if (nextP->type != 0) {
                inClip = !inClip;
                if (inClip) {
                    curP = nextP;
                    if (curP->conn2 + 1 == clipVertices[s].size()) {
                        nextP = clipVertices[s][0];
                    } else {
                        nextP = clipVertices[s][curP->conn2 + 1];
                    }
                } else {
                    curP = nextP;
                    if (curP->conn1 + 1 == fullVertices[s][k].size()) {
                        nextP = fullVertices[s][k][0];
                    } else {
                        nextP = fullVertices[s][k][curP->conn1 + 1];
                    }
                }
            } else {
                if (inClip) {
                    curP = nextP;
                    if (curP->pos[s] + 1 == clipVertices[s].size()) {
                        nextP = clipVertices[s][0];
                    } else {
                        nextP = clipVertices[s][curP->pos[s] + 1];
                    }
                } else {
                    curP = nextP;
                    if (curP->pos[s] + 1 == fullVertices[s][k].size()) {
                        nextP = fullVertices[s][k][0];
                    } else {
                        nextP = fullVertices[s][k][curP->pos[s] + 1];
                    }
                }
            }
        }
        if (inClip) {
            brezenhem(curP->x, curP->y, nextP->x, nextP->y, Color{0, 1, 1});
        } else {
            brezenhem(curP->x, curP->y, nextP->x, nextP->y, Color{1, 1, 1});
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT && (enterMode || clipMode || innerMode || innerClipMode)) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Color col;
        if (clipMode) {
            col.x = 0;
            col.y = 1;
            col.z = 1;
            setPoint(int(HEIGHT - ypos), int(xpos), col);
            super.push_back(Point{int(xpos), int(HEIGHT - ypos), 0, 0, 0, 0});
            clipVertices[0].push_back(&(super[super.size() - 1]));
            clipSize += 1;
            int s = clipVertices[0].size();
            if (s > 1) {
                brezenhem(clipVertices[0][s - 2]->x, clipVertices[0][s - 2]->y,
                          clipVertices[0][s - 1]->x, clipVertices[0][s - 1]->y, col);
                intersection(clipVertices[0][s - 2]->x, clipVertices[0][s - 2]->y,
                             clipVertices[0][s - 1]->x, clipVertices[0][s - 1]->y, s - 1, 0, 0);
            }
        }
        if (innerClipMode) {
            col.x = 0;
            col.y = 1;
            col.z = 1;
            setPoint(int(HEIGHT - ypos), int(xpos), col);
            super.push_back(Point{int(xpos), int(HEIGHT - ypos), 0, 0, 0, 0});
            clipVertices[1].push_back(&(super[super.size() - 1]));
            innerClipSize += 1;
            int s = clipVertices[1].size();
            if (s > 1) {
                brezenhem(clipVertices[1][s - 2]->x, clipVertices[1][s - 2]->y,
                          clipVertices[1][s - 1]->x, clipVertices[1][s - 1]->y, col);
                intersection(clipVertices[1][s - 2]->x, clipVertices[1][s - 2]->y,
                             clipVertices[1][s - 1]->x, clipVertices[1][s - 1]->y, s - 1, 0, 1);
            }
        }
        if (innerMode) {
            col.x = 1;
            col.y = 1;
            col.z = 1;
            setPoint(int(HEIGHT - ypos), int(xpos), col);
            super.push_back(Point{int(xpos), int(HEIGHT - ypos),0, 0, 0, 0});
            innerContourSize += 1;
            for (int i = 0; i < 2; i++) {
                vertices[i][1].push_back(&(super[super.size() - 1]));
                std::vector<Point*> p;
                aux[i][1].push_back(p);
                int s = vertices[i][1].size();
                if (s > 1) {
                    brezenhem(vertices[i][1][s - 2]->x, vertices[i][1][s - 2]->y,
                              vertices[i][1][s - 1]->x, vertices[i][1][s - 1]->y, col);
                }
            }
        }
        if (enterMode) {
            col.x = 1;
            col.y = 1;
            col.z = 1;
            setPoint(int(HEIGHT - ypos), int(xpos), col);
            super.push_back(Point{int(xpos), int(HEIGHT - ypos),0, 0, 0, 0});
            contourSize += 1;
            for (int i = 0; i < 2; i++) {
                vertices[i][0].push_back(&(super[super.size() - 1]));
                std::vector<Point*> p;
                aux[i][0].push_back(p);
                int s = vertices[i][0].size();
                if (s > 1) {
                    brezenhem(vertices[i][0][s - 2]->x, vertices[i][0][s - 2]->y,
                              vertices[i][0][s - 1]->x, vertices[i][0][s - 1]->y, col);
                }
            }
        }
    }
}

void clipActions(int s) {
    for (int k = 0; k < 2; k++) {
        if (!vertices[s][k].empty()) {
            for (int i = 0; i < aux[s][k].size(); i++) {
                fullVertices[s][k].push_back(vertices[s][k][i]);
                for (int j = 0; j < aux[s][k][i].size(); j++) {
                    fullVertices[s][k].push_back(aux[s][k][i][j]);
                    fullVertices[s][k][fullVertices[s][k].size() - 1]->conn1 = fullVertices[s][k].size() - 1;
                    clipVertices[s][fullVertices[s][k][fullVertices[s][k].size() - 1]->conn2]->conn1 =
                            fullVertices[s][k].size() - 1;
                }
            }

            for (int i = 0; i < fullVertices[s][k].size(); i++) {
                if (fullVertices[s][k][i]->type == 1) {
                    out[s][k].push_back(fullVertices[s][k][i]);
                }
                if (fullVertices[s][k][i]->type == 0) {
                    fullVertices[s][k][i]->pos[s] = i;
                }
            }
        }
    }
    for (int i = 0; i < clipVertices[s].size(); i++) {
        if (clipVertices[s][i]->type == 0) {
            clipVertices[s][i]->pos[s] = i;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
        if (!enterMode && !clipMode && !innerMode && !innerClipMode && !united) {
            united = true;
            initBuffer();
            unite(0, 0);
            if (!vertices[0][1].empty()) {
                unite(1, 0);
            }
            if (!clipVertices[1].empty()) {
                unite(0, 1);
                if (!vertices[1][1].empty()) {
                    unite(1, 1);
                }
            }
            noIntersectionDraw();
        }
        if (vertices[0][0].size() > 2 && enterMode) {
            enterMode = false;
            innerMode = true;
            int s = vertices[0][0].size();
            brezenhem(vertices[0][0][s - 1]->x, vertices[0][0][s - 1]->y, vertices[0][0][0]->x,
                      vertices[0][0][0]->y, Color{1, 1, 1});
        } else {
            if ((vertices[0][1].size() > 2 || vertices[0][1].empty()) && innerMode) {
                innerMode = false;
                clipMode = true;
                if (!vertices[0][1].empty()) {
                    int s = vertices[0][1].size();
                    brezenhem(vertices[0][1][s - 1]->x, vertices[0][1][s - 1]->y, vertices[0][1][0]->x,
                              vertices[0][1][0]->y, Color{1, 1, 1});
                }
            }
        }
        if (clipVertices[0].size() > 2 && clipMode) {
            clipMode = false;
            innerClipMode = true;
            int s = clipVertices[0].size();
            brezenhem(clipVertices[0][s - 1]->x, clipVertices[0][s - 1]->y, clipVertices[0][0]->x,
                      clipVertices[0][0]->y, Color{0, 1, 1});
            intersection(clipVertices[0][s - 1]->x, clipVertices[0][s - 1]->y, clipVertices[0][0]->x,
                         clipVertices[0][0]->y, s, 1, 0);
            clipActions(0);
        } else {
            if ((clipVertices[1].size() > 2 || clipVertices[1].empty())  && innerClipMode) {
                innerClipMode = false;
                int s = clipVertices[1].size();
                if (s != 0) {
                    brezenhem(clipVertices[1][s - 1]->x, clipVertices[1][s - 1]->y, clipVertices[1][0]->x,
                              clipVertices[1][0]->y, Color{0, 1, 1});
                    intersection(clipVertices[1][s - 1]->x, clipVertices[1][s - 1]->y, clipVertices[1][0]->x,
                                 clipVertices[1][0]->y, s, 1, 1);
                    clipActions(1);
                } else {
                    clipActions(1);
                }
            }
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        for (int k = 0; k < 2; k++) {
            for (int s = 0; s < 2; s++) {
                vertices[k][s].clear();
                fullVertices[k][s].clear();
                for (auto & i : aux[k][s]) {
                    i.clear();
                }
                aux[k][s].clear();
                out[k][s].clear();
            }
            clipVertices[k].clear();
        }
        super.clear();
        initBuffer();
        enterMode = true;
        clipMode = false;
        united = false;
        inClip = false;
        innerMode = false;
        innerClipMode = false;
        clipSize = 0;
        innerClipSize = 0;
        contourSize = 0;
        innerContourSize = 0;
    }
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
    delete []buffer;
    HEIGHT = height;
    WIDTH = width;
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -0.1, 0.1);
    buffer = new GLfloat[HEIGHT * WIDTH * 3];
    initBuffer();
    for (int k = 0; k < 2; k++) {
        for (int s = 0; s < 2; s++) {
            vertices[k][s].clear();
            fullVertices[k][s].clear();
            for (auto & i : aux[k][s]) {
                i.clear();
            }
            aux[k][s].clear();
            out[k][s].clear();
        }
        clipVertices[k].clear();
    }
    super.clear();
    initBuffer();
    enterMode = true;
    clipMode = false;
    united = false;
    inClip = false;
    innerMode = false;
    innerClipMode = false;
    clipSize = 0;
    innerClipSize = 0;
    contourSize = 0;
    innerContourSize = 0;
}

int main() {
    super.reserve(500);
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
    glfwGetFramebufferSize(window, &WIDTH, &HEIGHT);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

    buffer = new GLfloat[HEIGHT * WIDTH * 3];
    initBuffer();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -0.1, 0.1);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, WIDTH, HEIGHT);
        glRasterPos2i(0, 0);
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &buffer[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    delete []buffer;
    return 0;
}