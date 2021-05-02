#include <cmath>
#include <iostream>
#include <vector>
#include <stack>
#include <GLFW/glfw3.h>

//управление:
//LMB - установить точку
//RMB - точка затравки
//enter - окончание ввода
//backspace - очистка(отмена)
//tab - заливка
//ctrl - постфильтрация

GLint WIDTH = 720;
GLint HEIGHT = 720;
GLfloat *buffer = nullptr;
GLfloat *buffer2 = nullptr;
bool enterMode = true;
bool seedMode = true;
bool filled = false;
bool filtration = false;
bool needfiltration = true;

const GLfloat ker[9] = {2, 2, 2,
                        2, 4, 2,
                        2, 2, 2};
const GLfloat divider = 20;

struct Point {
    int x;
    int y;
};

std::vector<Point> vertices;
std::stack<Point> seeds;

void setPixel(int y, int x) {
    buffer[y * WIDTH * 3 + x * 3] = 1;
    buffer[y * WIDTH * 3 + x * 3 + 1] = 1;
    buffer[y * WIDTH * 3 + x * 3 + 2] = 1;
}

void brezenhem(int x1, int y1, int x2, int y2) {
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
        } else {
            if(error2 < deltaX) {
                error += deltaX;
                y1 += signY;
            }
        }
    }
}

void fill() {
    bool flag;
    while(!seeds.empty()) {
        Point p = seeds.top();
        setPixel(p.y, p.x);
        seeds.pop();
        int xMin = p.x;
        while (buffer[p.y * WIDTH * 3 + (xMin - 1) * 3] != 1) {
            xMin--;
            setPixel(p.y, xMin);
        }
        int xMax = p.x;
        while (buffer[p.y * WIDTH * 3 + (xMax + 1) * 3] != 1) {
            xMax++;
            setPixel(p.y, xMax);
        }
        flag = true;
        for (int x = xMin - 1; x < xMax + 2; x++) {
            if (buffer[(p.y - 1) * WIDTH * 3 + x * 3] != 1) {
                if (flag) {
                    seeds.push(Point{x, p.y - 1});
                    flag = false;
                }
            } else {
                flag = true;
            }
        }
        flag = true;
        for (int x = xMin - 1; x < xMax + 2; x++) {
            if (buffer[(p.y + 1) * WIDTH * 3 + x * 3] != 1) {
                if (flag) {
                    seeds.push(Point{x, p.y + 1});
                    flag = false;
                }
            } else {
                flag = true;
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

void initBuffer() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            buffer[i * WIDTH * 3 + j * 3] = 0;
            buffer[i * WIDTH * 3 + j * 3 + 1] = 0;
            buffer[i * WIDTH * 3 + j * 3 + 2] = 0;
            buffer2[i * WIDTH * 3 + j * 3] = 0;
            buffer2[i * WIDTH * 3 + j * 3 + 1] = 0;
            buffer2[i * WIDTH * 3 + j * 3 + 2] = 0;
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
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT && !enterMode && seedMode) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        setPixel(int(HEIGHT - ypos), int(xpos));
        seeds.push(Point{int(xpos), int(HEIGHT - ypos)});
        seedMode = false;
    }
}

void postfiltration() {
    GLint offset[9][2] {{-2, 2}, {0, 2}, {2, 2},
                        {-2, 0}, {0, 0}, {2, 0},
                        {-2, -2}, {0, -2}, {2, -2}};
    for (int i = 2; i < HEIGHT - 2; i++) {
        for (int j = 2; j < WIDTH - 2; j++) {
            for (int k = 0; k < 9; k++) {
                buffer2[i * WIDTH * 3 + j * 3] += (buffer[(i + offset[k][1]) * WIDTH * 3 + (j + offset[k][0]) * 3] * ker[k]);
                buffer2[i * WIDTH * 3 + j * 3 + 1] += (buffer[(i + offset[k][1]) * WIDTH * 3 + (j + offset[k][0]) * 3] * ker[k]);
                buffer2[i * WIDTH * 3 + j * 3 + 2] += (buffer[(i + offset[k][1]) * WIDTH * 3 + (j + offset[k][0]) * 3] * ker[k]);
            }
            buffer2[i * WIDTH * 3 + j * 3] /= divider;
            buffer2[i * WIDTH * 3 + j * 3 + 1] /= divider;
            buffer2[i * WIDTH * 3 + j * 3 + 2] /= divider;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
        if (vertices.size() > 2 && enterMode) {
            enterMode = false;
            drawEdges();
            needfiltration = true;
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        if (!seeds.empty()) {
            seeds.pop();
        }
        vertices.clear();
        initBuffer();
        enterMode = true;
        seedMode = true;
        filled = false;
        needfiltration = false;
        filtration = false;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_TAB && !filled && !enterMode && !seedMode) {
        fill();
        filled = true;
        needfiltration = true;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_LEFT_CONTROL && !enterMode) {
        filtration = !filtration;
    }
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
    delete []buffer;
    delete []buffer2;
    HEIGHT = height;
    WIDTH = width;
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -0.1, 0.1);
    buffer = new GLfloat[HEIGHT * WIDTH * 3];
    buffer2 = new GLfloat[HEIGHT * WIDTH * 3];
    initBuffer();
    if (!seeds.empty()) {
        seeds.pop();
    }
    vertices.clear();
    initBuffer();
    enterMode = true;
    seedMode = true;
    filled = false;
    needfiltration = false;
    filtration = false;
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
    buffer2 = new GLfloat[HEIGHT * WIDTH * 3];
    initBuffer();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -0.1, 0.1);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, WIDTH, HEIGHT);
        glRasterPos2i(0, 0);
        if (needfiltration) {
            postfiltration();
            needfiltration = false;
        }
        if (filtration) {
            glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &buffer2[0]);
        } else {
            glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &buffer[0]);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    delete []buffer;
    delete []buffer2;
    return 0;
}
