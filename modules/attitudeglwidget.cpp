#include "attitudeglwidget.h"
#include <GL/glu.h> // 需要在 .pro 中添加 LIBS += -lglu32

AttitudeGLWidget::AttitudeGLWidget(QWidget *parent) : QOpenGLWidget(parent) {}

void AttitudeGLWidget::updateAttitude(float roll, float pitch, float yaw) {
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;
    update(); // 触发重绘
}

void AttitudeGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    // 修复：使用 GL_DEPTH_TEST 而不是 QtPrivate::DepthTest
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void AttitudeGLWidget::resizeGL(int w, int h) {
    if (h == 0) h = 1; // 防止除以零
    glViewport(0, 0, w, h);

    // 修复：使用 GL_PROJECTION
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 设置透视投影
    gluPerspective(45.0, (double)w / (double)h, 0.1, 100.0);

    // 修复：使用 GL_MODELVIEW
    glMatrixMode(GL_MODELVIEW);
}

void AttitudeGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // 设置摄像机位置
    glTranslatef(0.0f, 0.0f, -6.0f); 

    // 根据欧拉角进行坐标转换
    // 注意：旋转顺序通常为 Y-X-Z 或根据你 IMU 的定义
    glRotatef(m_yaw,   0.0f, 1.0f, 0.0f);
    glRotatef(m_pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(m_roll,  0.0f, 0.0f, 1.0f);

    drawBallbotModel();
}

void AttitudeGLWidget::drawBallbotModel() {
    // 绘制一个彩色的长方体代表机器人主体
    glBegin(GL_QUADS);
        // 顶面 (绿色)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f, 1.5f, -1.0f); glVertex3f(-1.0f, 1.5f, -1.0f);
        glVertex3f(-1.0f, 1.5f,  1.0f); glVertex3f( 1.0f, 1.5f,  1.0f);
        // 底面 (蓝色)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f( 1.0f, -1.5f,  1.0f); glVertex3f(-1.0f, -1.5f,  1.0f);
        glVertex3f(-1.0f, -1.5f, -1.0f); glVertex3f( 1.0f, -1.5f, -1.0f);
        // 正面 (红色 - 代表前进方向)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f( 1.0f,  1.5f, 1.0f); glVertex3f(-1.0f,  1.5f, 1.0f);
        glVertex3f(-1.0f, -1.5f, 1.0f); glVertex3f( 1.0f, -1.5f, 1.0f);
        // 其他面 (灰色)
        glColor3f(0.5f, 0.5f, 0.5f);
        glVertex3f( 1.0f, -1.5f, -1.0f); glVertex3f(-1.0f, -1.5f, -1.0f);
        glVertex3f(-1.0f,  1.5f, -1.0f); glVertex3f( 1.0f,  1.5f, -1.0f);
        glVertex3f(-1.0f,  1.5f,  1.0f); glVertex3f(-1.0f,  1.5f, -1.0f);
        glVertex3f(-1.0f, -1.5f, -1.0f); glVertex3f(-1.0f, -1.5f,  1.0f);
        glVertex3f( 1.0f,  1.5f, -1.0f); glVertex3f( 1.0f,  1.5f,  1.0f);
        glVertex3f( 1.0f, -1.5f,  1.0f); glVertex3f( 1.0f, -1.5f, -1.0f);
    glEnd();

    // 绘制简易轮子/球体占位
    glColor3f(0.3f, 0.3f, 0.3f);
    // 这里可以添加更复杂的球形绘制逻辑
}
