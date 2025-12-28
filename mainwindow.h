#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QThread>
#include <QSplitter>
#include <QTimer>
#include <QPushButton>
#include <QKeyEvent>
#include <QLabel>

// 导入所有自定义组件
#include "DataProcess/DataProcessor.h"
#include "DataProcess/DataProcessor.h"
#include "modules/topstatusbar.h"
#include "modules/controlpanel.h"
#include "modules/logpanel.h"
#include "modules/waveformplot.h"
#include "modules/mainplot.h"
#include "modules/joystickwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void stopPlayback(); // 退出回放模式的函数


protected:
    // 使用 override 关键字可以帮助编译器检查签名是否正确
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupLayout();
    void connectSignals();

    // --- 核心逻辑组件 ---
    DataProcessor *m_dataProcessor;
    QThread* m_workerThread;
    QTimer *m_replotTimer;

    // UI 组件
    TopStatusBar   *m_statusBar;
    ControlPanel   *m_controlPanel;
    LogPanel       *m_logPanel;
    MainPlot       *m_mainPlot;
    JoystickWidget *m_joystick;
    QPushButton    *m_btnFreeze;

    // 左侧波形
    WaveformPlot   *m_accelPlot;
    WaveformPlot   *m_gyroPlot;
    WaveformPlot   *m_motorGroupPlot;
    WaveformPlot   *m_attitudePlot;

    QTimer *m_heartbeatTimer; //

    //回放功能
    QTimer *m_playbackTimer = nullptr;
    QList<QStringList> m_playbackBuffer; // 存储 CSV 内容
    int m_playbackIndex = 0;
    void startPlayback(const QString &filePath); // 执行回放逻辑
    QPushButton *btnOpenLog; // 将按钮提升为成员变量，方便切换文字


};
#endif // MAINWINDOW_H
