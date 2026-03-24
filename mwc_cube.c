/*! ШАБЛОН: Множество точек в единичном кубе [0, 1]^3 + сетка

Сборка (MSYS2):
    gcc -o points_cube mwc_points_cube.c -lfreeglut -lopengl32 -lglu32 -lm
*/

#include <GL/glut.h>
//#include <GLES/gl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
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
#define RANDC_MAX 32767 
static uint32_t randc(void){
    static unsigned long int next = 1;
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % (RAND_MAX + 1);
}
uint32_t newlib_rand () {
  static uint64_t x = 1;
  /* This multiplier was obtained from Knuth, D.E., "The Art of
     Computer Programming," Vol 2, Seminumerical Algorithms, Third
     Edition, Addison-Wesley, 1998, p. 106 (line 26) & p. 108 */
  x = x * 6364136223846793005LL + 1;
  return (uint32_t)(x >> 32) & RAND_MAX;
}
uint32_t lcg_parkmiller() {
    static uint32_t s = 1;
   uint64_t product = s * (uint64_t)48271u;
   uint32_t x = (product & 0x7fffffff) + (product >> 31);
   s = (x & 0x7fffffff) + (x >> 31);
   return s>>15;
}
// Генератор Лемера использует простой модуль 2^{32}-5:
uint32_t lcg_rand() {
    static uint32_t s = 1;
   uint64_t product = (uint64_t)s * 279470273u;
   product = (product & 0xffffffff) + 5 * (uint32_t) (product >> 32);
   product += 4;
   uint32_t x = (uint32_t)product + 5 * (uint32_t)(product >> 32);
   return s = x - 4;
}
#define ROTR(x,r) ((x)<<(32-r)^(x)>>(r))
static uint32_t mwc32x() {
    const  uint32_t A =  0xFF9B;//0xFF00;//0xFEA0;//0xFF75;//
    static uint32_t x = 1;
	x = A*(uint16_t)(x) + (x>>16);
    return x;//^x>>16;
}
#define MWC32_MAX (0x1.FEp-33)
static uint32_t mwc32x2() {
    const  uint32_t A = 0xFF9B;//0xFF00;//0xFEA0;//0xFF75;//
    static uint32_t x = (A<<15)-1;
	x = A*(uint16_t)(x) + (x>>16);
    return x;//^x>>16;
}

// ==================== ПАРАМЕТРЫ ====================
#define   NUM_POINTS  15000        // количество точек
float point_size = .75f;         // размер точек

// Массивы точек
float *px = NULL, *py = NULL, *pz = NULL;
float *qx = NULL, *qy = NULL, *qz = NULL;

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
int   auto_rotate = 1;     // 1 = включено, 0 = выключено
int   use_perspective = 1;
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
    if (qx) { free(qx); free(qy); free(qz); 
        qx = qy = qz = NULL;
    }

    px = (float*)malloc(NUM_POINTS * sizeof(float));
    py = (float*)malloc(NUM_POINTS * sizeof(float));
    pz = (float*)malloc(NUM_POINTS * sizeof(float));

    qx = (float*)malloc(NUM_POINTS * sizeof(float));
    qy = (float*)malloc(NUM_POINTS * sizeof(float));
    qz = (float*)malloc(NUM_POINTS * sizeof(float));


    for (int i = 0; i < NUM_POINTS; i++) {
#if 1
        px[i] = (mwc32x())*MWC32_MAX;
        py[i] = (mwc32x())*MWC32_MAX;
        pz[i] = (mwc32x())*MWC32_MAX;

        qx[i] = (mwc32x2())*MWC32_MAX;
        qy[i] = (mwc32x2())*MWC32_MAX;
        qz[i] = (mwc32x2())*MWC32_MAX;
#elif 1
        px[i] = (float)newlib_rand() / RAND_MAX;   // [0, 1]
        py[i] = (float)newlib_rand() / RAND_MAX;
        pz[i] = (float)newlib_rand() / RAND_MAX;
#elif 1
        px[i] = (float)lcg_parkmiller() *0x1p-16;   // [0, 1]
        py[i] = (float)lcg_parkmiller() *0x1p-16;
        pz[i] = (float)lcg_parkmiller() *0x1p-16;
#elif 0
        px[i] = (float)lcg_rand() *0x1p-32;   // [0, 1]
        py[i] = (float)lcg_rand() *0x1p-32;
        pz[i] = (float)lcg_rand() *0x1p-32;
#else
        px[i] = (float)randu() / RANDU_MAX;   // [0, 1]
        py[i] = (float)randu() / RANDU_MAX;
        pz[i] = (float)randu() / RANDU_MAX;
#endif
    }
}
uint32_t n_points = NUM_POINTS/4;
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

// === Авто-вращение (плавное включение/выключение) ===
    static float last_elapsed = 0.0f;
    float curr_elapsed = (float)glutGet(GLUT_ELAPSED_TIME);
    if (auto_rotate) {
        angle_y += (curr_elapsed - last_elapsed) * 0.015f;
    }
    last_elapsed = curr_elapsed;
// === Орбитальная камера вокруг центра куба ===
    float cx = 0.5f, cy = 0.5f, cz = 0.5f;
    float theta = angle_y * PI / 180.0f;
    float phi   = angle_x * PI / 180.0f;

    float camx = cx + cam_dist * cosf(phi) * sinf(theta);
    float camy = cy + cam_dist * sinf(phi);
    float camz = cz + cam_dist * cosf(phi) * cosf(theta);

    gluLookAt(camx, camy, camz,
              cx, cy, cz,
              0.0f, 1.0f, 0.0f);

// === ПРОЕКЦИЯ ===
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (h == 0) h = 1;

    float aspect = (float)w / (float)h;

    if (use_perspective) {
        gluPerspective(30.0, aspect, 0.1, 100.0);
    } else {
        // Пропорциональная ортогональная проекция
        float ortho_size = cam_dist * 0.25f;   // подберите коэффициент под ваш вкус (0.6–0.9)

        if (aspect >= 1.0f) {
            // широкое окно — растягиваем по X
            glOrtho(-ortho_size * aspect, ortho_size * aspect,
                    -ortho_size,          ortho_size,
                    -100.0, 100.0);
        } else {
            // высокое окно — растягиваем по Y
            glOrtho(-ortho_size,          ortho_size,
                    -ortho_size / aspect, ortho_size / aspect,
                    -100.0, 100.0);
        }
    }

    glMatrixMode(GL_MODELVIEW);

    draw_grid(0.2f);      // сетка с шагом 0.2
    //draw_cube();          // границы куба
    draw_axes();

    // Точки
    glPointSize(point_size);
    glColor3f(1.0f, 0.45f, 0.15f);     // ярко-оранжевый
    glEnable(GL_POINT_SMOOTH);
    n_points += 8;//
    if (n_points> NUM_POINTS) n_points=0;
    glBegin(GL_POINTS);
    for (int i = 0; i < n_points; i++) {
        glVertex3f(px[i], py[i], pz[i]);
    }
    glEnd();
    if (qx){
        glColor3f(0.15f, 1.0f, 0.45f);
        glBegin(GL_POINTS);
        for (int i = 0; i < n_points; i++) {
            glVertex3f(qx[i], qy[i], qz[i]);
        }
        glEnd();
    }

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
    if (key == ' ')
        auto_rotate = !auto_rotate;
    if (key == 'p')
        use_perspective = !use_perspective;
}
void processSpecialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  angle_y -= 5.0f; break;   // вращение влево
        case GLUT_KEY_RIGHT: angle_y += 5.0f; break;   // вращение вправо
        case GLUT_KEY_UP:    
            cam_dist -= 0.2f;        // перемещение по Z (приближение)
            if (cam_dist < 0.5f) cam_dist = 0.5f;
            break;
        case GLUT_KEY_DOWN:  
            cam_dist += 0.2f;        // перемещение по Z (удаление)
            break;
    }
}
// Мышь — орбитальное управление (гораздо удобнее старого)
int last_mouse_x = -1;
int last_mouse_y = -1;

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            last_mouse_x = x;
            last_mouse_y = y;
        } else {
            last_mouse_x = -1;
            last_mouse_y = -1;
        }
    }
}

void mouseMove(int x, int y) {
    if (last_mouse_x != -1) {
        int dx = x - last_mouse_x;
        int dy = y - last_mouse_y;

        angle_y += (float)dx * 0.3f;      // yaw
        angle_x -= (float)dy * 0.3f;      // pitch (естественное направление)

        // защита от переворота камеры
        if (angle_x > 85.0f)  angle_x = 85.0f;
        if (angle_x < -85.0f) angle_x = -85.0f;

        last_mouse_x = x;
        last_mouse_y = y;
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
    free(qx); free(qy); free(qz);
    return 0;
}
