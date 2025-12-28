#include "joystickwidget.h"

JoystickWidget::JoystickWidget(QWidget *parent) 
    : QWidget(parent), m_isPressed(false) 
{
    setMinimumSize(200, 200);
    // 开启鼠标追踪，即使不按住也能获取坐标（可选）
    setMouseTracking(true);
    
    m_knobRadius = 30; // 摇杆头大小
}

void JoystickWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    m_center = QPoint(width() / 2, height() / 2);
    m_knobPos = m_center;
    m_padRadius = qMin(width(), height()) / 2 - 20;
    m_maxDistance = m_padRadius - 10; // 留出一点余量
}

void JoystickWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 绘制底盘背景
    painter.setBrush(QColor(15, 15, 15));
    QPen padPen(QColor(0, 175, 255, 180), 2);
    painter.setPen(padPen);
    // 修复点：强制转换为 int 或使用 QPointF
    painter.drawEllipse(m_center, (int)m_padRadius, (int)m_padRadius);

    // 2. 绘制内部装饰环
    painter.setPen(QPen(QColor(0, 175, 255, 60), 1));
    painter.drawEllipse(m_center, (int)(m_padRadius * 0.7), (int)(m_padRadius * 0.7));
    painter.drawEllipse(m_center, (int)(m_padRadius * 0.4), (int)(m_padRadius * 0.4));

    // 3. 绘制坐标参考系
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    painter.drawLine(m_center.x() - m_padRadius, m_center.y(), m_center.x() + m_padRadius, m_center.y());
    painter.drawLine(m_center.x(), m_center.y() - m_padRadius, m_center.x(), m_center.y() + m_padRadius);

    // 4. 绘制摇杆头 (Knob)
    if (m_isPressed) {
        QRadialGradient knobGrad(m_knobPos, m_knobRadius);
        knobGrad.setColorAt(0, QColor(0, 175, 255));
        knobGrad.setColorAt(1, QColor(0, 80, 150));
        painter.setBrush(knobGrad);
        painter.setPen(QPen(Qt::white, 2));
    } else {
        painter.setBrush(QColor(40, 40, 40));
        painter.setPen(QPen(QColor(0, 175, 255), 2));
    }
    // 修复点：强制转换为 int
    painter.drawEllipse(m_knobPos, (int)m_knobRadius, (int)m_knobRadius);

    // 5. 数值提示 (保持 18px Bold 风格)
    if (m_isPressed) {
        double nx = (double)(m_knobPos.x() - m_center.x()) / m_maxDistance;
        double ny = -(double)(m_knobPos.y() - m_center.y()) / m_maxDistance;
        painter.setPen(QColor(0, 175, 255));
        painter.setFont(QFont("Segoe UI", 10, QFont::Bold)); // 摇杆内数值稍小，避免拥挤
        painter.drawText(rect(), Qt::AlignBottom | Qt::AlignHCenter,
                         QString("X: %1 | Y: %2").arg(nx, 0, 'f', 2).arg(ny, 0, 'f', 2));
    }
}

void JoystickWidget::mousePressEvent(QMouseEvent *event) {
    // 检查点击位置是否在底盘内
    double dist = QVector2D(event->pos() - m_center).length();
    if (dist < m_padRadius) {
        m_isPressed = true;
        updateJoystick(event->pos());
    }
}

void JoystickWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPressed) {
        updateJoystick(event->pos());
    }
}

void JoystickWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    m_isPressed = false;
    // 摇杆自动归位
    m_knobPos = m_center;
    update();
    emit axisChanged(0, 0); // 发送归零信号
}

void JoystickWidget::updateJoystick(const QPoint &pos) {
    QVector2D direction(pos - m_center);
    float distance = direction.length();

    // 核心逻辑：限程处理 (Vector Clamping)
    if (distance > m_maxDistance) {
        direction.normalize();
        direction *= m_maxDistance;
        m_knobPos = m_center + direction.toPoint();
        distance = m_maxDistance;
    } else {
        m_knobPos = pos;
    }

    // 归一化计算：映射到 -1.0 到 1.0
    double nx = (double)(m_knobPos.x() - m_center.x()) / m_maxDistance;
    double ny = -(double)(m_knobPos.y() - m_center.y()) / m_maxDistance; // Y轴取反符合笛卡尔坐标系

    update();
    emit axisChanged(nx, ny);
}
