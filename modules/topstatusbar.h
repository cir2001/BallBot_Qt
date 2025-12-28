#ifndef TOPSTATUSBAR_H
#define TOPSTATUSBAR_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QDateTime> // 必须引入

class TopStatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit TopStatusBar(QWidget *parent = nullptr);

    // 接收起始时间戳以实现同步
    void startSession(double startTime);
    void stopSession();
    void setPlaybackMode(bool isPlayback); // 切换回放/实时视觉状态

public slots:
    void updateConnection(bool connected, const QString &ip);
    void updateBattery(float voltage, int percentage);
    void updateNetworkStats(int hz, float lossRate);
    void updateSystemMode(const QString &mode);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateTimeDisplay();

private:
    QLabel *lblStatusLed, *lblIpInfo, *lblHz, *lblLoss, *lblMode, *lblVoltage;
    QLabel *lblTimerTitle, *lblTimerValue;
    QProgressBar *barBattery;
    QPushButton *btnMin, *btnMax, *btnClose;

    QTimer *m_refreshTimer;
    double m_baseTimestamp = -1.0; // 【新增】：同步基准时间
    QPoint m_dragPosition;

    void setupUi();
    void setupStyle();
};

#endif
