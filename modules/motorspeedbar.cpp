#include "motorspeedbar.h"

MotorSpeedBar::MotorSpeedBar(QWidget *parent) : QWidget(parent) {
    // 1. 整体布局：使用网格布局实现对齐 [灯] [文字] [图表]
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(8);

    QStringList names = {"M1", "M2", "M3"};
    QColor motorColors[] = {QColor(255, 50, 50), QColor(50, 255, 50), QColor(50, 100, 255)};

    for(int i = 0; i < 3; ++i) {
        // 状态指示灯 (圆点)
        m_statusLamps[i] = new QLabel();
        m_statusLamps[i]->setFixedSize(12, 12);
        m_statusLamps[i]->setStyleSheet("background-color: #444; border-radius: 6px; border: 1px solid #666;");

        // 电机名称标签
        m_nameLabels[i] = new QLabel(names[i]);
        m_nameLabels[i]->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold; font-family: 'Segoe UI';")
                                           .arg(motorColors[i].name()));
        m_nameLabels[i]->setFixedWidth(30);

        layout->addWidget(m_statusLamps[i], i, 0, Qt::AlignCenter);
        layout->addWidget(m_nameLabels[i], i, 1, Qt::AlignCenter);
    }

    // 2. 配置 QCustomPlot
    m_plot = new QCustomPlot();
    m_plot->setBackground(Qt::black);

    // 【核心空间优化】：禁用自动边距，手动预留 X 轴刻度空间
    m_plot->axisRect()->setAutoMargins(QCP::msNone);
    // 参数：左10, 上10, 右15, 下35 (下侧35px用于容纳X轴刻度数字)
    m_plot->axisRect()->setMargins(QMargins(10, 10, 15, 35));

    // X 轴配置 (数值轴)
    m_plot->xAxis->setRange(-MAX_VALUE, MAX_VALUE);
    m_plot->xAxis->setVisible(true);
    m_plot->xAxis->setBasePen(QPen(Qt::white, 1));
    m_plot->xAxis->setTickLabelColor(Qt::white);
    m_plot->xAxis->setTickLabelFont(QFont("Segoe UI", 9, QFont::Bold));
    m_plot->xAxis->grid()->setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));
    m_plot->xAxis->grid()->setZeroLinePen(QPen(Qt::red, 1, Qt::SolidLine)); // 0位线

    // Y 轴配置 (位置轴)
    m_plot->yAxis->setRange(0.4, 3.6);
    m_plot->yAxis->setVisible(false);

    // 3. 创建水平柱状条
    // M1 (位于 Y=3)
    m_bar1 = new QCPBars(m_plot->yAxis, m_plot->xAxis);
    m_bar1->setBrush(motorColors[0]);
    m_bar1->setPen(Qt::NoPen);
    m_bar1->setWidth(0.5);

    // M2 (位于 Y=2)
    m_bar2 = new QCPBars(m_plot->yAxis, m_plot->xAxis);
    m_bar2->setBrush(motorColors[1]);
    m_bar2->setPen(Qt::NoPen);
    m_bar2->setWidth(0.5);

    // M3 (位于 Y=1)
    m_bar3 = new QCPBars(m_plot->yAxis, m_plot->xAxis);
    m_bar3->setBrush(motorColors[2]);
    m_bar3->setPen(Qt::NoPen);
    m_bar3->setWidth(0.5);

    // 将绘图区跨 3 行放入网格第 3 列
    layout->addWidget(m_plot, 0, 2, 3, 1);
    layout->setColumnStretch(2, 1); // 让图表占据剩余所有宽度

    this->setStyleSheet("background-color: #000000;");
}

void MotorSpeedBar::setSpeeds(double m1, double m2, double m3, uint16_t s1, uint16_t s2, uint16_t s3) {
    if (m_isFrozen) return;

    // 更新柱状图数值
    m_bar1->data()->clear(); m_bar1->addData(3, m1);
    m_bar2->data()->clear(); m_bar2->addData(2, m2);
    m_bar3->data()->clear(); m_bar3->addData(1, m3);

    uint16_t status[] = {s1, s2, s3};
    for(int i = 0; i < 3; ++i) {
        if(status[i] > 0) {
            // 正常：亮绿色。通过设置浅色边框模拟发光感
            m_statusLamps[i]->setStyleSheet(
                "background-color: #00FF44; "
                "border-radius: 6px; "
                "border: 2px solid #88FFBB;" // 浅绿色边框模拟外发光
                );
        } else {
            // 异常：亮红色
            m_statusLamps[i]->setStyleSheet(
                "background-color: #FF3333; "
                "border-radius: 6px; "
                "border: 2px solid #FF9999;"
                );
        }
    }

    m_plot->replot(QCustomPlot::rpQueuedReplot);
}

void MotorSpeedBar::clear() {
    m_bar1->data()->clear();
    m_bar2->data()->clear();
    m_bar3->data()->clear();
    for(int i = 0; i < 3; ++i) {
        m_statusLamps[i]->setStyleSheet("background-color: #444; border-radius: 6px; border: 1px solid #666;");
    }
    m_plot->replot();
}
