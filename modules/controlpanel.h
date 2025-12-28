#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = nullptr);

signals:
    // 信号：发送 PID 给主窗口进行转发
    void pidUpdated(int loopType, float p, float i, float d);
    // 信号：切换机器人模式
    void modeChanged(int modeIdx);
    // 信号：控制指令（速度 X, Y）
    void controlCmd(float vx, float vy);
    // 信号：执行校准逻辑
    void calibrateRequested(int sensorType);

private:
    void setupUi(QWidget *container);
    void setupStyle();
    
    // UI 内部组件封装
    QGroupBox* createPidGroup(const QString &name, int loopType);
    
    // 模式切换
    QComboBox *m_comboMode;
    QPushButton *m_btnStop;
};
#endif
