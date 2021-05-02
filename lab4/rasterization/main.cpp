#include <cmath>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>

//управление:
//LMB - установить точку
//enter - окончание ввода
//backspace - очистка(отмена)
//tab - заливка
//ctrl - постфильтрация

GLint WIDTH = 720;
GLint HEIGHT = 720;
GLfloat *buffer = nullptr;
GLfloat *fillMap = nullptr;
GLint *pointNY = nullptr;
bool enterMode = true;
bool filled = false;
bool filtration = false;

const GLfloat ker[9] = {2, 2, 2,
                      2, 4, 2,
                      2, 2, 2};
const GLfloat divider = 20;

struct Point {
    int x;
    int y;
};

std::vector<Point> vertices;

void setPixel(int y, int x) {
    buffer[y * WIDTH * 3 + x * 3] = 1;
    buffer[y * WIDTH * 3 + x * 3 + 1] = 1;
    buffer[y * WIDTH * 3 + x * 3 + 2] = 1;
}

void brezenhem(int x1, int y1, int x2, int y2) {
    int x1l = x1;
    int y1t = y1;
    bool jump = false;
    bool first = true;
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;
    int error = deltaX - deltaY;
    setPixel(y2, x2);
    while(x1 != x2 || y1 != y2) {
        setPixel(y1, x1);
        int error2 = error * 2;
        if(error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
            if (x1 > x1l) {
                x1l = x1;
            }
        }
        if(error2 < deltaX) {
            error += deltaX;
            y1t += signY;
            jump = true;
        }
        if (jump) {
            if (!first) {
                fillMap[y1 * WIDTH + x1l] += 1;
                pointNY[y1] += 1;
            }
            x1l = x1;
            first = false;
            jump = false;
        }
        y1 = y1t;
    }
}

void fill() {
    bool inside;
    for (int i = 0; i < HEIGHT; i++) {
        inside = false;
        for (int j = 0; j < WIDTH; j++) {
            if (fillMap[i * WIDTH + j] == 1) {
                inside = !inside;
            }
            if (inside) {
                setPixel(i, j);
            }
        }
    }
}

void drawEdges() {
    for (int i = 0; i < vertices.size() - 1; i++) {
        brezenhem(vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y);
    }
    int i = vertices.size() - 1;
    brezenhem(vertices[i].x, vertices[i].y, vertices[0].x, vertices[0].y);
}

void adjustVertices() {
    for (int i = 1; i < vertices.size() - 1; i++) {
        if ((vertices[i].y - vertices[i - 1].y) * (vertices[i].y - vertices[i + 1].y) < 0) {
            fillMap[vertices[i].y * WIDTH + vertices[i].x] = 1;
        }
        if (pointNY[vertices[i].y] % 2 != 0) {
            fillMap[vertices[i].y * WIDTH + vertices[i].x] = 1;
            pointNY[vertices[i].y] += 1;
        }
    }
    int i = vertices.size() - 1;
    if ((vertices[i].y - vertices[0].y) * (vertices[i].y - vertices[i - 1].y) < 0) {
        fillMap[vertices[i].y * WIDTH + vertices[i].x] = 1;
    }
    if (pointNY[vertices[i].y] % 2 != 0) {
        fillMap[vertices[i].y * WIDTH + vertices[i].x] = 1;
        pointNY[vertices[i].y] += 1;
    }
    if ((vertices[0].y - vertices[i].y) * (vertices[0].y - vertices[1].y) < 0) {
        fillMap[vertices[0].y * WIDTH + vertices[0].x] = 1;
    }
    if (pointNY[vertices[0].y] % 2 != 0) {
        fillMap[vertices[0].y * WIDTH + vertices[0].x] = 1;
        pointNY[vertices[0].y] += 1;
    }
}

void initBuffer() {
    for (int i = 0; i < HEIGHT; i++) {
        pointNY[i] = 0;
        for (int j = 0; j < WIDTH; j++) {
            buffer[i * WIDTH * 3 + j * 3] = 0;
            buffer[i * WIDTH * 3 + j * 3 + 1] = 0;
            buffer[i * WIDTH * 3 + j * 3 + 2] = 0;
            fillMap[i * WIDTH +j] = 0;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT && enterMode) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        setPixel(int(HEIGHT - ypos), int(xpos));
        vertices.push_back(Point{int(xpos), int(HEIGHT - ypos)});
    }
}

void postfiltration() {
    glDrawBuffer(GL_BACK);
    glClear(GL_ACCUM_BUFFER_BIT);
    GLint offset[9][2] {{0, 2}, {1, 2}, {2, 2},
                        {0, 1}, {1, 1}, {2, 1},
                        {0, 0}, {1, 0}, {2, 0}};
    for (int i = 0; i < 9; i++) {
        glRasterPos2i(offset[i][0], offset[i][1]);
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &buffer[0]);
        glAccum(GL_ACCUM, ker[i] / divider);
    }
    glAccum(GL_RETURN, 1.0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
        if (vertices.size() > 2 && enterMode) {
            enterMode = false;
            drawEdges();
            adjustVertices();
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        vertices.clear();
        initBuffer();
        enterMode = true;
        filled = false;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_TAB && !filled && !enterMode) {
        fill();
        filled = true;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_LEFT_CONTROL && !enterMode) {
        filtration = !filtration;
    }
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
    float wk = float(width) / float(WIDTH);
    float hk = float(height) / float(HEIGHT);
    for (auto & vertice : vertices) {
        vertice.x = int(std::round(vertice.x * wk));
        vertice.y = int(std::round(vertice.y * hk));
    }
    delete []buffer;
    delete []fillMap;
    delete []pointNY;
    HEIGHT = height;
    WIDTH = width;
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -0.1, 0.1);
    buffer = new GLfloat[HEIGHT * WIDTH * 3];
    fillMap = new GLfloat[HEIGHT * WIDTH];
    pointNY = new GLint[HEIGHT];
    initBuffer();
    for (auto & vertice : vertices) {
        setPixel(vertice.y, vertice.x);
    }
    if (!enterMode) {
        drawEdges();
        adjustVertices();
    }
    if (filled) {
        fill();
    }
}

int main() {
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
    fillMap = new GLfloat[HEIGHT * WIDTH];
    pointNY = new GLint[HEIGHT];
    initBuffer();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -0.1, 0.1);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (filtration) {
            glViewport(-1, -1, WIDTH + 1, HEIGHT + 1);
            postfiltration();
        } else {
            glViewport(0, 0, WIDTH, HEIGHT);
            glRasterPos2i(0, 0);
            glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &buffer[0]);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    delete []buffer;
    delete []fillMap;
    delete []pointNY;
    return 0;
}