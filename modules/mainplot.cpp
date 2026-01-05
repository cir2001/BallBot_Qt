#include "mainplot.h"

MainPlot::MainPlot(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupStyle();
}

void MainPlot::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_topTabBar = new QTabBar();
    m_topTabBar->addTab("ATTITUDE");
    m_topTabBar->addTab("IMU ACCEL");
    m_topTabBar->addTab("IMU GYRO");
    m_topTabBar->addTab("MOTORS SPEED");
    m_topTabBar->setExpanding(true);

    m_controlStack = new QStackedWidget();
    m_plotStack = new QStackedWidget();

    createPage(0, "Pose",  {"Pitch", "Roll", "Yaw", "Angle Target"});
    createPage(1, "Accel", {"Acc X", "Acc Y", "Acc Z", "Acc Target"});
    createPage(2, "Gyro",  {"Gyro X", "Gyro Y", "Gyro Z", "Gyro Target"});
    createPage(3, "Motor", {"Motor 1", "Motor 2", "Motor 3", "Speed Target"});

    connect(m_topTabBar, &QTabBar::currentChanged, [this](int index){
        m_controlStack->setCurrentIndex(index);
        m_plotStack->setCurrentIndex(index);
        // 【新增】：切换 Tab 时立即重绘当前页面，确保量程视觉正确
        m_plots[index]->replot();
    });

    mainLayout->addWidget(m_topTabBar);
    mainLayout->addWidget(m_controlStack);
    mainLayout->addWidget(m_plotStack);

    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 8);
}

void MainPlot::createPage(int index, QString title, QStringList labels) {
    QColor colors[] = {Qt::cyan, Qt::magenta, Qt::yellow, Qt::white};

    QCustomPlot *plot = new QCustomPlot();
    for (int i = 0; i < 4; ++i) {
        plot->addGraph();
        if (i < 3) {
            plot->graph(i)->setPen(QPen(colors[i], 1.5));
        } else {
            plot->graph(i)->setPen(QPen(colors[i], 1, Qt::DashLine));
        }
    }

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->axisRect()->setRangeZoom(Qt::Vertical);
    plot->axisRect()->setRangeDrag(Qt::Vertical);

    m_plots.append(plot);
    m_plotStack->addWidget(plot);

    QWidget *ctrlContainer = new QWidget();
    QHBoxLayout *ctrlLayout = new QHBoxLayout(ctrlContainer);
    ctrlLayout->setContentsMargins(20, 0, 20, 0);
    ctrlLayout->setSpacing(40);

    for (int i = 0; i < 4; ++i) {
        QCheckBox *chk = new QCheckBox(labels[i]);
        chk->setChecked(true);
        QString colorHex = colors[i].name();
        chk->setStyleSheet(QString(
                               "QCheckBox::indicator:checked {"
                               "   background-color: %1;"
                               "   border: 1px solid %1;"
                               "}"
                               ).arg(colorHex));

        connect(chk, &QCheckBox::toggled, [=](bool checked){
            plot->graph(i)->setVisible(checked);
            plot->replot();
        });
        ctrlLayout->addWidget(chk);
    }
    ctrlLayout->addStretch();
    m_controlStack->addWidget(ctrlContainer);
}

void MainPlot::setupStyle() {
    this->setStyleSheet("background-color: #000000;");

    QString baseStyle =
        "QTabBar::tab { background: #1A1A1A; color: #888; padding: 10px 40px; border: 1px solid #333; font-size: 18px; font-weight: bold; }"
        "QTabBar::tab:selected { background: #333; color: #00AFFF; border-bottom: 2px solid #00AFFF; }"
        "QCheckBox { background: transparent; color: white; font-size: 16px; font-weight: bold; spacing: 10px; }"
        "QCheckBox::indicator { width: 30px; height: 5px; border-radius: 2px; }"
        "QCheckBox::indicator:unchecked { background-color: #2A2A2A; border: 1px solid #444; }";

    this->setStyleSheet(baseStyle);

    QFont axisFont("Segoe UI", 12, QFont::Bold);

    // 【核心修改】：遍历每个绘图窗口并根据索引设置专属量程
    for(int i = 0; i < m_plots.size(); ++i) {
        QCustomPlot* plot = m_plots[i];
        plot->setBackground(QBrush(QColor(0, 0, 0)));
        plot->axisRect()->setBackground(QBrush(QColor(0, 0, 0)));

        plot->xAxis->setTickLabelFont(axisFont);
        plot->yAxis->setTickLabelFont(axisFont);
        plot->xAxis->setLabelFont(axisFont);
        plot->yAxis->setLabelFont(axisFont);

        QPen axisPen(QColor(100, 100, 100));
        plot->xAxis->setBasePen(axisPen);
        plot->yAxis->setBasePen(axisPen);
        plot->xAxis->setTickLabelColor(Qt::white);
        plot->yAxis->setTickLabelColor(Qt::white);

        plot->xAxis->setNumberFormat("f");
        plot->xAxis->setNumberPrecision(1);

        plot->axisRect()->setAutoMargins(QCP::msNone);
        plot->axisRect()->setMargins(QMargins(65, 10, 20, 40));

        plot->xAxis->grid()->setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));
        plot->yAxis->grid()->setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));

        // --- 根据 Tab 类型初始化 Y 轴范围 ---
        switch(i) {
        case 0: plot->yAxis->setRange(-45.0, 45.0);   break; // 首页：姿态角 (deg)
        case 1: plot->yAxis->setRange(-2.0, 2.0);    break; // 加速度 (g)
        case 2: plot->yAxis->setRange(-500.0, 500.0); break; // 陀螺仪 (dps)
        case 3: plot->yAxis->setRange(-25000, 25000);   break; // 电机输出 (PWM)
        }

        plot->replot();
    }
}

void MainPlot::updateAllData(double ts,
                             const QVector<double>& acc, double accTar,
                             const QVector<double>& gyro, double gyroTar,
                             const QVector<double>& angle, double angleTar,
                             const QVector<double>& motor, double motorTar)
{
    if (m_isFrozen) return;

    if (m_startTime < 0) m_startTime = ts;
    double relTs = ts - m_startTime;

    auto pushData = [&](int idx, const QVector<double>& data, double tar) {
        if (data.size() >= 3) {
            for(int i=0; i<3; ++i) {
                m_plots[idx]->graph(i)->addData(relTs, data[i]);
                m_plots[idx]->graph(i)->data()->removeBefore(relTs - 600.0);
            }
            m_plots[idx]->graph(3)->addData(relTs, tar);
            m_plots[idx]->graph(3)->data()->removeBefore(relTs - 600.0);

            m_plots[idx]->xAxis->setRange(relTs, 8.0, Qt::AlignRight);

            // 注意：如果你希望量程完全固定，请注释掉下面这行 rescale()
            // m_plots[idx]->yAxis->rescale();

            m_plots[idx]->replot(QCustomPlot::rpQueuedReplot);
        }
    };

    pushData(0, angle, angleTar); // 首页显示姿态角
    pushData(1, acc, accTar);     // 第二页显示加速度
    pushData(2, gyro, gyroTar);    // 第三页显示陀螺仪
    pushData(3, motor, motorTar);  // 第四页显示电机
}

void MainPlot::setFrozen(bool frozen) {
    m_isFrozen = frozen;
    for (auto plot : m_plots) {
        if (m_isFrozen) {
            plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
            plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
            plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
            plot->setBackground(QBrush(QColor(30, 30, 35)));
        } else {
            plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            plot->axisRect()->setRangeZoom(Qt::Vertical);
            plot->axisRect()->setRangeDrag(Qt::Vertical);
            plot->setBackground(QBrush(QColor(0, 0, 0)));
        }
        plot->replot();
    }
}

void MainPlot::clearData() {
    m_startTime = -1.0; // 【核心重置】：清除数据时同时重置起始时间
    for (auto plot : m_plots) {
        for (int i = 0; i < plot->graphCount(); ++i) {
            plot->graph(i)->data()->clear();
        }
        plot->replot();
    }
}
