/*! ШАБЛОН: Множество точек в единичном кубе [0, 1]^3 + сетка

Сборка (MSYS2):
    gcc -o points_cube mwc_points_cube.c -lfreeglut -lopengl32 -lglu32 -lm
*/

#include <GL/glut.h>
#include <GLES/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846

// {\displaystyle V_{j+1}\equiv (65539V_{j}){\bmod {2}}^{31},}
// В качестве крайнего примера, на котором видна решетка используем RANDU
#define RANDU_MAX (1uL<<31)
static uint32_t randu(){
    static uint32_t v = 1;
    v = (65539u * v) & 0x7FFFFFFFu;
    return v;
}

static uint32_t mwc32x() {
    const  uint32_t A = 0xFF9B;//0xFF00;
    static uint32_t x = 1;
	x = A*(uint16_t)(x) + (x>>16);
    return x;
}
// ==================== ПАРАМЕТРЫ ====================
int   NUM_POINTS = 10000;        // количество точек
float point_size = 1.2f;         // размер точек

// Массивы точек
float *px = NULL, *py = NULL, *pz = NULL;

// ==================== КАМЕРА ====================
float angle = 0.0f;
float lx = 0.0f, lz = -1.0f;
float cam_x = 0.5f, cam_z = 3.5f;

// ==================== РИСОВАНИЕ СЕТКИ В КУБЕ [0,1]^3 ====================
void draw_grid(float step) {
    glColor3f(0.35f, 0.35f, 0.40f);
    glLineWidth(1.0f);
    //glEnable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);

    // Плоскости XY (фиксированный Z)
    for (float z = 0.0f; z <= 1.0f; z += step) {
        for (float x = 0.0f; x <= 1.0f; x += step) {
            glVertex3f(x, 0.0f, z); glVertex3f(x, 1.0f, z);   // вертикальные линии
        }
        for (float y = 0.0f; y <= 1.0f; y += step) {
            glVertex3f(0.0f, y, z); glVertex3f(1.0f, y, z);   // горизонтальные линии
        }
    }

    // Плоскости XZ (фиксированный Y)
    for (float y = 0.0f; y <= 1.0f; y += step) {
        for (float x = 0.0f; x <= 1.0f; x += step) {
            glVertex3f(x, y, 0.0f); glVertex3f(x, y, 1.0f);
        }
        for (float z = 0.0f; z <= 1.0f; z += step) {
            glVertex3f(0.0f, y, z); glVertex3f(1.0f, y, z);
        }
    }

    // Плоскости YZ (фиксированный X)
    for (float x = 0.0f; x <= 1.0f; x += step) {
        for (float y = 0.0f; y <= 1.0f; y += step) {
            glVertex3f(x, y, 0.0f); glVertex3f(x, y, 1.0f);
        }
        for (float z = 0.0f; z <= 1.0f; z += step) {
            glVertex3f(x, 0.0f, z); glVertex3f(x, 1.0f, z);
        }
    }

    glEnd();
}

// ==================== РИСОВАНИЕ ГРАНИЦ КУБА ====================
void draw_cube(void) {
    glColor3f(0.6f, 0.6f, 0.7f);
    glLineWidth(2.0f);
    //glEnable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);
    // 12 рёбер куба [0,1]^3
    // Нижняя грань
    glVertex3f(0,0,0); glVertex3f(1,0,0);
    glVertex3f(1,0,0); glVertex3f(1,1,0);
    glVertex3f(1,1,0); glVertex3f(0,1,0);
    glVertex3f(0,1,0); glVertex3f(0,0,0);

    // Верхняя грань
    glVertex3f(0,0,1); glVertex3f(1,0,1);
    glVertex3f(1,0,1); glVertex3f(1,1,1);
    glVertex3f(1,1,1); glVertex3f(0,1,1);
    glVertex3f(0,1,1); glVertex3f(0,0,1);

    // Вертикальные рёбра
    glVertex3f(0,0,0); glVertex3f(0,0,1);
    glVertex3f(1,0,0); glVertex3f(1,0,1);
    glVertex3f(1,1,0); glVertex3f(1,1,1);
    glVertex3f(0,1,0); glVertex3f(0,1,1);
    glEnd();
}
// ==================== КАМЕРА ====================
float angle_y = 0.0f;      // угол вращения вокруг Y (главное вращение)
float angle_x = 25.0f;     // небольшой наклон (можно менять мышкой)
float cam_dist = 3.2f;     // расстояние камеры от центра

// ==================== РИСОВАНИЕ ТРЁХ ОСЕЙ ====================
void draw_axes(void) {
    glLineWidth(3.0f);
    //glEnable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);
    // Ось X (красная) от 0 до 1
    glColor3f(1.0f, 0.2f, 0.2f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);

    // Ось Y (зелёная)
    glColor3f(0.2f, 1.0f, 0.2f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    // Ось Z (синяя)
    glColor3f(0.3f, 0.6f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);

    glEnd();

    // Маленькие подписи осей (опционально)
    glColor3f(1.0f, 0.2f, 0.2f);
    glRasterPos3f(1.05f, 0.0f, 0.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'X');

    glColor3f(0.2f, 1.0f, 0.2f);
    glRasterPos3f(0.0f, 1.05f, 0.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Y');

    glColor3f(0.3f, 0.6f, 1.0f);
    glRasterPos3f(0.0f, 0.0f, 1.05f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Z');
}
// ==================== ИНИЦИАЛИЗАЦИЯ ТОЧЕК ====================
void init_points(void) {
    if (px) { free(px); free(py); free(pz); }

    px = (float*)malloc(NUM_POINTS * sizeof(float));
    py = (float*)malloc(NUM_POINTS * sizeof(float));
    pz = (float*)malloc(NUM_POINTS * sizeof(float));

    srand((unsigned int)time(NULL));

    for (int i = 0; i < NUM_POINTS; i++) {
#if 0
        px[i] = mwc32x()*0x1.fep-33;
        py[i] = mwc32x()*0x1.fep-33;
        pz[i] = mwc32x()*0x1.fep-33;
#else
        px[i] = (float)randu() / RANDU_MAX;   // [0, 1]
        py[i] = (float)randu() / RANDU_MAX;
        pz[i] = (float)randu() / RANDU_MAX;
#endif
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(cam_x, 1.2f, cam_z,
              cam_x + lx, 1.0f, cam_z + lz,
              0.0f, 1.0f, 0.0f);

    glRotatef(25.0f, 1.0f, 0.0f, 0.0f);                    // лёгкий наклон
    glRotatef(glutGet(GLUT_ELAPSED_TIME) * 0.015f, 0.0f, 1.0f, 0.0f); // авто-вращение

    draw_grid(0.2f);      // сетка с шагом 0.2
    //draw_cube();          // границы куба
    draw_axes();

    // Точки
    glPointSize(point_size);
    glColor3f(1.0f, 0.45f, 0.15f);     // ярко-оранжевый
    glEnable(GL_POINT_SMOOTH);

    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_POINTS; i++) {
        glVertex3f(px[i], py[i], pz[i]);
    }
    glEnd();

    glDisable(GL_POINT_SMOOTH);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
void processNormalKeys(unsigned char key, int x, int y) {
    if (key == 27) exit(0);
}
void processSpecialKeys(int key, int x, int y) {
    float fraction = 0.15f;
    switch (key) {
        case GLUT_KEY_LEFT:  angle -= 0.03f; break;
        case GLUT_KEY_RIGHT: angle += 0.03f; break;
        case GLUT_KEY_UP:    cam_x += lx * fraction; cam_z += lz * fraction; break;
        case GLUT_KEY_DOWN:  cam_x -= lx * fraction; cam_z -= lz * fraction; break;
    }
    lx = sinf(angle);
    lz =-cosf(angle);
}

float deltaAngle = 0.0f;
int xOrigin = -1;

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_UP) {
            angle += deltaAngle;
            xOrigin = -1;
        } else {
            xOrigin = x;
        }
    }
}

void mouseMove(int x, int y) {
    if (xOrigin >= 0) {
        deltaAngle = (x - xOrigin) * 0.002f;
        lx = sinf(angle + deltaAngle);
        lz = -cosf(angle + deltaAngle);
    }
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(960, 762);
    glutCreateWindow("MWC-Lattice test");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(processSpecialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    glutTimerFunc(0, timer, 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.03f, 0.03f, 0.08f, 1.0f);

    init_points();

    printf("Точки в кубе [0,1]^3\n");
    printf("Управление: мышь + стрелки + ESC\n");
    glutMainLoop();

    free(px); free(py); free(pz);
    return 0;
}
