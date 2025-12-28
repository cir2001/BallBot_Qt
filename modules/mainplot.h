#ifndef MAINPLOT_H
#define MAINPLOT_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabBar>
#include <QStackedWidget>
#include <QVector>
#include "plot/qcustomplot.h"

class MainPlot : public QWidget {
    Q_OBJECT
public:
    explicit MainPlot(QWidget *parent = nullptr);

    // 适配 Ballbot 传感器数据结构
    void updateAllData(double ts,
                       const QVector<double>& acc, double accTar,
                       const QVector<double>& gyro, double gyroTar,
                       const QVector<double>& angle, double angleTar,
                       const QVector<double>& motor, double motorTar);

    void clearData();
    void setFrozen(bool frozen);

private:
    void setupUi();
    void setupStyle();
    void createPage(int index, QString title, QStringList labels);

    QTabBar *m_topTabBar;
    QStackedWidget *m_controlStack;
    QStackedWidget *m_plotStack;

    QList<QCustomPlot*> m_plots;
    bool m_isFrozen = false;
    double m_startTime = -1.0; // 【新增】：记录首个数据点的时间戳
};

#endif
