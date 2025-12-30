#ifndef MOTORSPEEDBAR_H
#define MOTORSPEEDBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include "plot/qcustomplot.h"

class MotorSpeedBar : public QWidget {
    Q_OBJECT
public:
    explicit MotorSpeedBar(QWidget *parent = nullptr);

    // 更新三个电机的 PWM 或转速值
    void setSpeeds(double m1, double m2, double m3);
    void setFrozen(bool frozen) { m_isFrozen = frozen; }

    void clear() { setSpeeds(0, 0, 0); }

private:
    QCustomPlot *m_plot;
    // 为三个电机分别创建柱状图对象，以便设置不同颜色
    QCPBars *m_bar1;
    QCPBars *m_bar2;
    QCPBars *m_bar3;

    bool m_isFrozen = false;
    const double MAX_VALUE = 25000.0; // 对应 PWM 最大值


};

#endif
