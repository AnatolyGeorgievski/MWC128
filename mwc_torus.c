/*! Визуализация 2D траекторий на Торе

Сборка под Windows/ MSYS2
    $ gcc -o test mwc_torus.c -lfreeglut -lopengl32 -lglu32 -lm
*/


#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846

// Параметры тора
float R = 3.0f;   // большой радиус
float r = 1.0f;   // малый радиус

// Для траектории
int NUM_POINTS = 2000;
float *x, *y, *z;

void init_trajectory() {
    x = (float*)malloc(NUM_POINTS * sizeof(float));
    y = (float*)malloc(NUM_POINTS * sizeof(float));
    z = (float*)malloc(NUM_POINTS * sizeof(float));

    for(int i = 0; i < NUM_POINTS; i++) {
        float t = i * 40.0f * PI / (NUM_POINTS - 1);   // параметр времени
        float theta = t;
        float phi   = 15.0f * t;                        // 5:1 намотка (можно менять)

        x[i] = (R + r * cosf(phi)) * cosf(theta);
        y[i] = (R + r * cosf(phi)) * sinf(theta);
        z[i] = r * sinf(phi);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Камера (примерный вид как в matplotlib)
    gluLookAt(0, -8, 5,   0,0,0,   0,0,1);

    glRotatef(30, 1,0,0);   // небольшой наклон
    glRotatef(glutGet(GLUT_ELAPSED_TIME)*0.02f, 0,1,0);  // вращение

    // Сам тор (wireframe для простоты)
    glColor3f(0.5f, 0.5f, 0.6f);
    glLineWidth(0.50f);
    glutWireTorus(r, R, 24, 48);   // встроенная функция GLUT!

    // Траектория (красная линия)
    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < NUM_POINTS; i++) {
        glVertex3f(x[i], y[i], z[i]);
    }
    glEnd();

    // Точки начала и конца (для наглядности)
    glPointSize(8.0f);
    glBegin(GL_POINTS);
        glColor3f(0,1,0);   glVertex3f(x[0], y[0], z[0]);           // старт
        glColor3f(0,0,1);   glVertex3f(x[NUM_POINTS-1], y[NUM_POINTS-1], z[NUM_POINTS-1]); // конец
    glEnd();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w/h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // ~60 fps
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    glutCreateWindow("MWC-Torus");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);

    init_trajectory();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    printf("Используй мышь + клавиши для управления окном.\n");
    glutMainLoop();

    free(x); free(y); free(z);
    return 0;
}