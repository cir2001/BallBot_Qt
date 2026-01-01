#ifndef MOTORSPEEDBAR_H
#define MOTORSPEEDBAR_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVector>
#include "plot/qcustomplot.h"

class MotorSpeedBar : public QWidget {
    Q_OBJECT
public:
    explicit MotorSpeedBar(QWidget *parent = nullptr);

    // 更新接口：包含 3 个电机的转速和状态位
    void setSpeeds(double m1, double m2, double m3, uint16_t s1 = 0, uint16_t s2 = 0, uint16_t s3 = 0);
    void setFrozen(bool frozen) { m_isFrozen = frozen; }
    void clear();

private:
    QCustomPlot *m_plot;
    QCPBars *m_bar1, *m_bar2, *m_bar3;

    // 指示灯与标签
    QLabel *m_statusLamps[3];
    QLabel *m_nameLabels[3];

    bool m_isFrozen = false;
    const double MAX_VALUE = 2400.0; // 对应转速/PWM 最大量程

    void setupStyle();
};

#endif
