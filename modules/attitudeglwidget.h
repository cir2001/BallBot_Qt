#ifndef ATTITUDEGLWIDGET_H
#define ATTITUDEGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>

class AttitudeGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit AttitudeGLWidget(QWidget *parent = nullptr);

public slots:
    // 外部调用接口：更新姿态数据
    void updateAttitude(float roll, float pitch, float yaw);

protected:
    // 必须实现的 OpenGL 三大接口
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    float m_roll = 0.0f;
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;

    void drawBallbotModel(); // 绘制机器人模型
};

#endif