#include "motorspeedbar.h"

MotorSpeedBar::MotorSpeedBar(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_plot = new QCustomPlot();
    layout->addWidget(m_plot);

    // 基础样式配置
    m_plot->setBackground(Qt::black);
    m_plot->axisRect()->setBackground(Qt::black);
    m_plot->axisRect()->setMargins(QMargins(30, 10, 30, 30));

    // 配置 X 轴（数值轴）：左右对称
    m_plot->xAxis->setRange(-MAX_VALUE, MAX_VALUE);
    m_plot->xAxis->setBasePen(QPen(Qt::white));
    m_plot->xAxis->setTickLabelColor(Qt::white);
    m_plot->xAxis->setTickLabelFont(QFont("Segoe UI", 9));
    m_plot->xAxis->grid()->setZeroLinePen(QPen(Qt::red, 2));
    m_plot->xAxis->grid()->setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));

    // 配置 Y 轴（索引轴）：隐藏
    m_plot->yAxis->setRange(0.4, 3.6);
    m_plot->yAxis->setVisible(false);

    // --- 创建三个独立的柱状条 ---
    // 在 QCustomPlot 2.0 中，构造函数会自动将 plottable 添加到 plot 中
    m_bar1 = new QCPBars(m_plot->yAxis, m_plot->xAxis);
    m_bar1->setWidth(0.5);
    m_bar1->setPen(Qt::NoPen);
    m_bar1->setBrush(QColor(255, 50, 50));

    m_bar2 = new QCPBars(m_plot->yAxis, m_plot->xAxis);
    m_bar2->setWidth(0.5);
    m_bar2->setPen(Qt::NoPen);
    m_bar2->setBrush(QColor(50, 255, 50));

    m_bar3 = new QCPBars(m_plot->yAxis, m_plot->xAxis);
    m_bar3->setWidth(0.5);
    m_bar3->setPen(Qt::NoPen);
    m_bar3->setBrush(QColor(50, 100, 255));

    // 注意：这里删除了 m_plot->addPlottable(bar) 这一行，因为它不再需要且会导致报错
}

void MotorSpeedBar::setSpeeds(double m1, double m2, double m3) {
    if (m_isFrozen) return;

    // 更新数据
    m_bar1->data()->clear();
    m_bar1->addData(3, m1);

    m_bar2->data()->clear();
    m_bar2->addData(2, m2);

    m_bar3->data()->clear();
    m_bar3->addData(1, m3);

    m_plot->replot(QCustomPlot::rpQueuedReplot);
}
