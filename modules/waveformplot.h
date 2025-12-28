#ifndef WAVEFORMPLOT_H
#define WAVEFORMPLOT_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include "plot/qcustomplot.h"

class WaveformPlot : public QWidget {
    Q_OBJECT
public:
    explicit WaveformPlot(const QString &title, const QStringList &labels,
                          const QVector<QColor> &colors, QPair<double, double> yRange, QWidget *parent = nullptr);

    void setYRange(double min, double max);

    void addData(double timestamp, const QVector<double> &values);
    void setFrozen(bool frozen) { m_isFrozen = frozen; }
    void clearData();

private:
    QCustomPlot *m_plot;
    QLabel *m_lblTitle;
    QVector<QLabel*> m_valLabels;
    QVector<QColor> m_lineColors;

    bool m_isFrozen = false;
    double m_startTime = -1.0;
    const double MAX_HISTORY_SEC = 600.0; // 【新增】：仅保留最近 10 分钟数据

    void setupUi(const QString &title, const QStringList &labels);
    void setupStyle();

};
#endif
