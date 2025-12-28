#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

class JoystickWidget : public QWidget {
    Q_OBJECT
public:
    explicit JoystickWidget(QWidget *parent = nullptr);

signals:
    // 发射归一化后的坐标 (范围 -1.0 到 1.0)
    void axisChanged(double x, double y);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateJoystick(const QPoint &pos);

    QPoint m_center;      // 底盘中心点
    QPoint m_knobPos;    // 摇杆头当前位置
    bool m_isPressed;    // 鼠标是否按下

    int m_padRadius;     // 底盘半径
    int m_knobRadius;    // 摇杆头半径
    int m_maxDistance;   // 摇杆最大偏移距离
};

#endif