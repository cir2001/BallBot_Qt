#include "modules/topstatusbar.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent> // 【必须添加，解决 incomplete type 错误】
#include <QDebug>

TopStatusBar::TopStatusBar(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupStyle();

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &TopStatusBar::updateTimeDisplay);

    updateConnection(false, "0.0.0.0");
    updateBattery(0.0, 0);
}

void TopStatusBar::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 0, 0, 0);
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignVCenter);

    lblStatusLed = new QLabel("●");
    lblIpInfo = new QLabel("DISCONNECTED");
    lblMode = new QLabel("STANDBY");

    // 计时器组件
    lblTimerTitle = new QLabel("SESSION:");
    lblTimerValue = new QLabel("00:00:00");

    lblHz = new QLabel("Rate: 0 Hz");
    lblLoss = new QLabel("Loss: 0.0%");
    lblVoltage = new QLabel("0.0 V");
    barBattery = new QProgressBar();
    barBattery->setFixedSize(80, 10);
    barBattery->setTextVisible(false);

    btnMin = new QPushButton("—");
    btnMax = new QPushButton("□");
    btnClose = new QPushButton("✕");
    btnMin->setFixedSize(45, 45); btnMax->setFixedSize(45, 45); btnClose->setFixedSize(45, 45);

    mainLayout->addWidget(lblStatusLed);
    mainLayout->addWidget(lblIpInfo);
    mainLayout->addStretch();
    mainLayout->addWidget(lblMode);
    mainLayout->addStretch();
    mainLayout->addWidget(lblTimerTitle);
    mainLayout->addWidget(lblTimerValue);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(lblHz);
    mainLayout->addWidget(lblLoss);
    mainLayout->addWidget(lblVoltage);
    mainLayout->addWidget(barBattery);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(0);
    btnLayout->addWidget(btnMin); btnLayout->addWidget(btnMax); btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);

    connect(btnMin, &QPushButton::clicked, [this](){ window()->showMinimized(); });
    connect(btnMax, &QPushButton::clicked, [this](){
        if(window()->isMaximized()) window()->showNormal();
        else window()->showMaximized();
    });
    connect(btnClose, &QPushButton::clicked, [this](){ window()->close(); });
    this->setFixedHeight(55);
}

void TopStatusBar::setupStyle()
{
    this->setStyleSheet(
        "QWidget { background-color: #000000; color: #FFFFFF; font-family: 'Segoe UI'; font-size: 18px; font-weight: bold; }"
        "QLabel#lblMode { color: #00AFFF; font-size: 20px; }"
        "QLabel#timerValue { color: #00AFFF; min-width: 90px; }" // 科技蓝计时器
        "QPushButton { border: none; background: transparent; color: #DCDCDC; font-size: 26px; font-weight: normal; }"
        "QPushButton:hover { background-color: #333; }"
        "QPushButton#btnClose:hover { background-color: #E81123; }"
        "QProgressBar { border: 1px solid #444; background: #222; border-radius: 2px; }"
        "QProgressBar::chunk { background-color: #00FF44; }"
        );
    lblMode->setObjectName("lblMode");
    lblTimerValue->setObjectName("timerValue");
    btnClose->setObjectName("btnClose");
}

// 【核心修改】：接入外部起始时间实现同步
void TopStatusBar::startSession(double startTime) {
    if (m_baseTimestamp < 0) {
        m_baseTimestamp = startTime;
        m_refreshTimer->start(1000);
    }
}

void TopStatusBar::stopSession() {
    m_refreshTimer->stop();
    m_baseTimestamp = -1.0;
}

void TopStatusBar::updateTimeDisplay() {
    if (m_baseTimestamp < 0) return;

    // 使用与图表相同的 QDateTime 基准计算已过去的时间
    double currentTs = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    int totalSecs = static_cast<int>(currentTs - m_baseTimestamp);

    int s = totalSecs % 60;
    int m = (totalSecs / 60) % 60;
    int h = totalSecs / 3600;

    lblTimerValue->setText(QString("%1:%2:%3")
                               .arg(h, 2, 10, QChar('0'))
                               .arg(m, 2, 10, QChar('0'))
                               .arg(s, 2, 10, QChar('0')));
}

void TopStatusBar::updateConnection(bool conn, const QString &ip) {
    if(conn) {
        lblStatusLed->setStyleSheet("color: #00FFCC; font-size: 32px;");
        lblIpInfo->setText("CONNECTED: " + ip);
        lblIpInfo->setStyleSheet("color: #00AFFF;");
    } else {
        lblStatusLed->setStyleSheet("color: #FF3333; font-size: 32px;");
        lblIpInfo->setText("DISCONNECTED");
        lblIpInfo->setStyleSheet("color: #FF3333;");
        stopSession(); // 断开时停止计时
    }
}

void TopStatusBar::updateBattery(float v, int p) {
    lblVoltage->setText(QString("%1 V").arg(v, 0, 'f', 1));
    barBattery->setValue(p);
}

void TopStatusBar::updateNetworkStats(int hz, float loss) {
    lblHz->setText(QString("Rate: %1 Hz").arg(hz));
    lblLoss->setText(QString("Loss: %1%").arg(loss, 0, 'f', 1));
}

void TopStatusBar::updateSystemMode(const QString &mode) {
    lblMode->setText(mode.toUpper());
}

// 修正后的鼠标拖动逻辑
void TopStatusBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - window()->frameGeometry().topLeft();
        event->accept();
    }
}

void TopStatusBar::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        window()->move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void TopStatusBar::setPlaybackMode(bool isPlayback) {
    if (isPlayback) {
        lblMode->setText("OFFLINE PLAYBACK");
        lblMode->setStyleSheet("color: #FFA500; font-size: 20px; font-weight: bold;"); // 橙色
        lblStatusLed->setStyleSheet("color: #555555; font-size: 32px;"); // 灰色 LED 表示非实时
    } else {
        lblMode->setText("STANDBY"); // 或恢复当前真实的系统模式
        // 恢复实时模式的样式（科技蓝）
        lblMode->setStyleSheet("color: #00AFFF; font-size: 20px; font-weight: bold;");
        lblStatusLed->setStyleSheet("color: #FF3333; font-size: 32px;"); // 默认红色，直到收到新数据变绿
        lblIpInfo->setText("DISCONNECTED");
    }
}
