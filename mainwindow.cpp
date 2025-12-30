#include "mainwindow.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QVBoxLayout>
#include <QSplitter>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. 初始化后端
    m_dataProcessor = new DataProcessor();
    m_workerThread = new QThread(this);
    m_dataProcessor->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_dataProcessor, &DataProcessor::initWorker);
    m_workerThread->start();

    // 2. 初始化布局
    setupLayout();

    // 3. 连接信号
    connectSignals();

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setSingleShot(true); //
    // 当超过 2 秒没收到数据时，触发超时逻辑
    connect(m_heartbeatTimer, &QTimer::timeout, this, [=](){
        m_statusBar->updateConnection(false, "0.0.0.0"); // 变红并显示断开
    });

    // 4. 统一刷新定时器 (30FPS)
    m_replotTimer = new QTimer(this);
    connect(m_replotTimer, &QTimer::timeout, this, [=](){
        // 只有不处于冻结状态时才刷新
        if (!m_btnFreeze->isChecked()) {
            foreach(QCustomPlot* p, this->findChildren<QCustomPlot*>()) {
                p->replot(QCustomPlot::rpQueuedReplot);
            }
        }
    });
    m_replotTimer->start(33);

    // 设置无边框
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    this->setStyleSheet("background-color: #000000;"); // 全局黑背景

    // 设置全局按钮样式，自动处理选中和未选中状态
    this->setStyleSheet(
        "QPushButton#btnFreeze { "
        "   font-size: 18px; font-weight: bold; border-radius: 4px; border: 1px solid #333; "
        "   background-color: #1A1A1A; color: #00AFFF; "
        "}"
        "QPushButton#btnFreeze:checked { "
        "   background-color: #005500; color: white; border: 1px solid #00FF00; "
        "}"
        "QPushButton#btnFreeze:hover { background-color: #333; }"
        );

    // 适配 1080p
    this->resize(1920, 1080);
    this->setWindowTitle("Ballbot Research Station - Ver 2025.12");

}
void MainWindow::setupLayout() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ================== 1. 顶部状态栏 (占比 1) ==================
    m_statusBar = new TopStatusBar(this);
    rootLayout->addWidget(m_statusBar, 1);

    // ================== 2. 中间区域 (占比 6) ==================
    QSplitter *midHSplitter = new QSplitter(Qt::Horizontal);
    midHSplitter->setHandleWidth(2);
    midHSplitter->setStyleSheet("QSplitter::handle { background-color: #333; }");

    // --- 中左：姿态角 + 陀螺仪 (固定 360px) ---
    QWidget *midLeftArea = new QWidget();
    midLeftArea->setFixedWidth(400);
    QVBoxLayout *midLeftL = new QVBoxLayout(midLeftArea);
    midLeftL->setContentsMargins(5, 5, 5, 5);
    m_attitudePlot = new WaveformPlot("POSE EULER (deg)", {"Roll", "Pitch", "Yaw"},
                                      {Qt::cyan, Qt::magenta, Qt::yellow}, {-45, 45});
    m_gyroPlot = new WaveformPlot("GYROSCOPE (dps)", {"GyroX", "GyroY", "GyroZ"},
                                  {Qt::cyan, Qt::magenta, Qt::yellow}, {-500, 500});
    midLeftL->addWidget(m_attitudePlot, 5);
    midLeftL->addWidget(m_gyroPlot, 5);

    // --- 中中：MainPlot + Freeze按钮 (动态拉伸) ---
    QWidget *midCenterArea = new QWidget();
    midCenterArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *midCenterL = new QVBoxLayout(midCenterArea);
    midCenterL->setContentsMargins(0, 5, 0, 5);
    m_mainPlot = new MainPlot();

    QHBoxLayout *midBtnL = new QHBoxLayout();
    m_btnFreeze = new QPushButton("FREEZE ALL DATA (SPACE)");
    m_btnFreeze->setCheckable(true);
    m_btnFreeze->setFixedHeight(45);
    m_btnFreeze->setObjectName("btnFreeze");
    m_btnFreeze->setStyleSheet("font-size: 18px; font-weight: bold; background-color: #1A1A1A; color: #00AFFF; border: 1px solid #333;");

    btnOpenLog = new QPushButton("OPEN CSV LOG");
    btnOpenLog->setFixedHeight(45);
    btnOpenLog->setFixedWidth(180);
    // 橙色风格区分功能
    btnOpenLog->setObjectName("btnOpenLog");
    // 初始样式
    btnOpenLog->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFA500; background: #1A1A1A; border: 1px solid #444;");

    midCenterL->addWidget(m_mainPlot);
    midBtnL->addWidget(m_btnFreeze, 1);
    midBtnL->addWidget(btnOpenLog, 0);
    midCenterL->addLayout(midBtnL);

    connect(btnOpenLog, &QPushButton::clicked, this, [=](){
        if (m_playbackTimer && m_playbackTimer->isActive()) {
            // 如果正在回放，点击则是退出
            this->stopPlayback();
        } else {
            // 如果未在回放，点击则是打开文件
            QString path = QFileDialog::getOpenFileName(this, "选择回放数据", "", "CSV Files (*.csv)");
            if(!path.isEmpty()) this->startPlayback(path);
        }
    });

    // --- 中右：加速度 + 电机输出 (固定 360px) ---
    QWidget *midRightArea = new QWidget();
    midRightArea->setFixedWidth(400);
    QVBoxLayout *midRightL = new QVBoxLayout(midRightArea);
    midRightL->setContentsMargins(5, 5, 5, 5);
    m_accelPlot = new WaveformPlot("ACCELERATION (g)", {"AccX", "AccY", "AccZ"},
                                   {Qt::cyan, Qt::magenta, Qt::yellow}, {-2.0, 2.0});
    m_motorSpeedBar = new MotorSpeedBar();
    m_motorSpeedBar->setFixedHeight(300); // 设置一个合适的高度
    midRightL->addWidget(m_accelPlot, 5);
    midRightL->addWidget(m_motorSpeedBar, 5);

    midHSplitter->addWidget(midLeftArea);
    midHSplitter->addWidget(midCenterArea);
    midHSplitter->addWidget(midRightArea);

    // 设置伸缩因子，确保中间区域随窗口缩放
    midHSplitter->setStretchFactor(0, 0);
    midHSplitter->setStretchFactor(1, 1);
    midHSplitter->setStretchFactor(2, 0);

    rootLayout->addWidget(midHSplitter, 6);

    // ================== 3. 下部区域 (比例 3) ==================
    QSplitter *bottomHSplitter = new QSplitter(Qt::Horizontal);
    bottomHSplitter->setHandleWidth(2);
    bottomHSplitter->setStyleSheet("QSplitter::handle { background-color: #333; }");

    // --- 下左：LogPanel (固定 400px) ---
    m_logPanel = new LogPanel();
    m_logPanel->setFixedWidth(400); // 【修改点】：锁定 400px

    // --- 下中：ControlPanel (动态伸缩) ---
    m_controlPanel = new ControlPanel();
    m_controlPanel->setMinimumHeight(0); // 配合 QScrollArea 压缩

    // --- 下右：Joystick 容器 (固定 400px) ---
    QWidget *joyContainer = new QWidget();
    joyContainer->setFixedWidth(400); // 【修改点】：锁定 400px
    QVBoxLayout *joyL = new QVBoxLayout(joyContainer);
    joyL->setContentsMargins(0, 0, 0, 0); // 顶部边距设为 0
    joyL->setSpacing(0);                 // 标题和摇杆之间的间距设为极小

    m_joystick = new JoystickWidget();

    joyL->addWidget(m_joystick, 1);       // 摇杆占据所有剩余空间

    bottomHSplitter->addWidget(m_logPanel);
    bottomHSplitter->addWidget(m_controlPanel);
    bottomHSplitter->addWidget(joyContainer);

    // 设置下部伸缩因子：只有索引为 1 的 ControlPanel 伸缩
    bottomHSplitter->setStretchFactor(0, 0);
    bottomHSplitter->setStretchFactor(1, 1);
    bottomHSplitter->setStretchFactor(2, 0);

    rootLayout->addWidget(bottomHSplitter, 3);
}
void MainWindow::connectSignals() {
    // 1. 下行控制信号
    connect(m_controlPanel, &ControlPanel::pidUpdated, m_dataProcessor, &DataProcessor::sendPidPacket);
    connect(m_joystick, &JoystickWidget::axisChanged, this, [=](double x, double y){
        // 将摇杆转换为期望角度发送给机器人
        m_dataProcessor->sendPidPacket(2, x * 5.0f, y * 5.0f, 0);
    });

    // 2. 上行数据分发 (统一入口)
    connect(m_dataProcessor, &DataProcessor::dataParsed, this,
            [=](uint32_t ts_raw, float r, float p, float y, int16_t m1,int16_t m2,int16_t m3){
                if (m_btnFreeze->isChecked()) return;

                double timestamp = QDateTime::currentMSecsSinceEpoch() / 1000.0;
                m_statusBar->updateConnection(true, "192.168.0.104");
                m_statusBar->startSession(timestamp);
                m_heartbeatTimer->start(2000);

                // 发送完整数据给 MainPlot 和侧边辅助图
                m_attitudePlot->addData(timestamp, {double(r), double(p), double(y)});
                m_motorSpeedBar->setSpeeds(double(m1), double(m2),double(m3));

                QVector<double> poseData = {double(p), double(r), double(y)};
                QVector<double> motorData = {double(m1), double(m2), double(m3)};
                QVector<double> emptyData = {0,0,0};

                m_mainPlot->updateAllData(timestamp,
                                          emptyData, 0.0,    // Accel 保持不变或传空
                                          emptyData, 0.0,    // Gyro 保持不变或传空
                                          poseData,  0.0,    // Pose (对应首页 ATTITUDE)
                                          motorData, 0.0);   // Motor
            });

    // 3. 日志分发
    connect(m_dataProcessor, &DataProcessor::statusUpdated, m_logPanel, [=](const QString &msg){
        m_logPanel->appendLog(msg, 0); // 这里手动补齐第二个参数 0
    });

    // 4. 冻结逻辑
    connect(m_btnFreeze, &QPushButton::toggled, this, [=](bool frozen)
    {
        m_mainPlot->setFrozen(frozen);
        QList<WaveformPlot*> allPlots = this->findChildren<WaveformPlot*>();
        for (WaveformPlot* p : allPlots) p->setFrozen(frozen);

        m_motorSpeedBar->setFrozen(frozen);

        QString baseStyle = "font-size: 18px; font-weight: bold; border-radius: 4px; border: 1px solid #333; ";
        if (frozen) {
            m_btnFreeze->setText("RESUME REAL-TIME (SPACE)");
            m_btnFreeze->setStyleSheet(baseStyle + "background-color: #005500; color: white; border: 1px solid #00FF00;");
        } else {
            m_btnFreeze->setText("FREEZE ALL DATA (SPACE)");
            m_btnFreeze->setStyleSheet(baseStyle + "background-color: #1A1A1A; color: #00AFFF;");
        }
    });

    connect(m_dataProcessor, &DataProcessor::imuRawParsed, this,
            [=](int16_t gx, int16_t gy, int16_t gz, int16_t ax, int16_t ay, int16_t az){
                if (m_btnFreeze->isChecked()) return;

                double ts = QDateTime::currentMSecsSinceEpoch() / 1000.0;

                // 分发到三曲线组件
                m_accelPlot->addData(ts, {ax/16384.0, ay/16384.0, az/16384.0});
                m_gyroPlot->addData(ts, {gx/131.0, gy/131.0, gz/131.0});
            });

    connect(m_workerThread, &QThread::finished, m_dataProcessor, &QObject::deleteLater);//关闭界面后，解绑udp
}

MainWindow::~MainWindow()
{
    if (m_workerThread->isRunning())
    {
        m_workerThread->quit();
        if (!m_workerThread->wait(1000))
        { // 给 1 秒优雅退出的时间
            m_workerThread->terminate();
            m_workerThread->wait();
        }
    }
}
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        // 切换按钮状态，会自动触发上面的 connect 逻辑
        m_btnFreeze->setChecked(!m_btnFreeze->isChecked());
    } else {
        QMainWindow::keyPressEvent(event);
    }
}
void MainWindow::startPlayback(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_logPanel->appendLog("无法打开文件: " + filePath, 1);
        return;
    }

    // 1. 解析 CSV 数据
    m_playbackBuffer.clear();
    QTextStream in(&file);
    QString header = in.readLine(); // 跳过表头
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            m_playbackBuffer.append(line.split(','));
        }
    }
    file.close();

    if (m_playbackBuffer.isEmpty()) {
        m_logPanel->appendLog("错误：CSV 文件内容为空", 1);
        return;
    }

    qDebug() << "成功加载数据行数:" << m_playbackBuffer.size();

    // 2. 初始化回放环境
    m_playbackIndex = 0;
    m_mainPlot->clearData();
    m_accelPlot->clearData();
    m_gyroPlot->clearData();
    m_attitudePlot->clearData();

    m_motorSpeedBar->clear();

    m_statusBar->setPlaybackMode(true);
    m_logPanel->appendLog("开始回放: " + filePath, 0);

    // 切换按钮状态
    btnOpenLog->setText("EXIT PLAYBACK");
    btnOpenLog->setStyleSheet("font-size: 16px; font-weight: bold; color: white; background: #800000; border: 1px solid #FF0000;");

    // 3. 配置定时器
    if (!m_playbackTimer) {
        m_playbackTimer = new QTimer(this);
    }
    m_playbackTimer->stop();
    m_playbackTimer->disconnect(); // 彻底清除之前的连接

    connect(m_playbackTimer, &QTimer::timeout, this, [=](){
        if (m_playbackIndex >= m_playbackBuffer.size()) {
            m_playbackTimer->stop();
            m_statusBar->setPlaybackMode(false);
            m_logPanel->appendLog("回放结束", 0);
            return;
        }

        // 获取当前行
        QStringList row = m_playbackBuffer[m_playbackIndex++];

        // 【关键防御】：检查 CSV 列数是否足够（至少需要 8 列才能访问 index 7）
        if (row.size() < 8) {
            if(m_playbackIndex == 1) qDebug() << "CSV 列数不足，当前列数:" << row.size();
            return;
        }

        // 模拟实时时间戳触发绘图逻辑
        double ts = QDateTime::currentMSecsSinceEpoch() / 1000.0;

        try {
            // 解析数据（根据你的 CSV 实际列索引调整）
            // 假设：1:Roll, 2:Pitch, 3:Yaw, 4:Ax, 5:Ay, 6:Az, 7:M1
            float r  = row[1].toFloat();
            float p  = row[2].toFloat();
            float y  = row[3].toFloat();
            float ax = row[4].toFloat();
            float ay = row[5].toFloat();
            float az = row[6].toFloat();
            int16_t m1_speed = static_cast<int16_t>(row[14].toInt());
            int16_t m2_speed = static_cast<int16_t>(row[15].toInt());
            int16_t m3_speed = static_cast<int16_t>(row[16].toInt());

            QVector<double> acc = {ax, ay, az};
            QVector<double> pose = {p, r, y};
            QVector<double> motor_speed = {double(m1_speed), double(m2_speed), double(m3_speed)};
            QVector<double> empty = {0,0,0};

            // 分发到 UI
            m_mainPlot->updateAllData(ts, acc, 0, empty, 0, pose, 0, motor_speed, 0);
            m_accelPlot->addData(ts, acc);
            m_attitudePlot->addData(ts, pose);
            m_motorSpeedBar->setSpeeds(motor_speed[0], motor_speed[1], motor_speed[2]);

        } catch (...) {
            qDebug() << "第" << m_playbackIndex << "行数据解析失败";
        }
    });

    m_playbackTimer->start(10); // 100Hz 频率回放
}
void MainWindow::stopPlayback() {
    if (m_playbackTimer) m_playbackTimer->stop();

    m_statusBar->setPlaybackMode(false);

    // 恢复按钮样式
    btnOpenLog->setText("OPEN CSV LOG");
    btnOpenLog->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFA500; background: #1A1A1A; border: 1px solid #444;");

    m_playbackBuffer.clear();
    m_playbackIndex = 0;
    m_logPanel->appendLog("已退出回放模式", 0);
}

