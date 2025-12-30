#include "waveformplot.h"

WaveformPlot::WaveformPlot(const QString &title, const QStringList &labels,
                           const QVector<QColor> &colors, QPair<double, double> yRange, QWidget *parent)
    : QWidget(parent), m_lineColors(colors), m_startTime(-1.0) {
    setupUi(title, labels);
    for (int i = 0; i < 3; ++i) {
        m_plot->addGraph();
        m_plot->graph(i)->setPen(QPen(m_lineColors[i], 1.5));
    }
    m_plot->yAxis->setRange(yRange.first, yRange.second);
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->axisRect()->setRangeDrag(Qt::Vertical);
    m_plot->axisRect()->setRangeZoom(Qt::Vertical);
    setupStyle();
}

void WaveformPlot::setupUi(const QString &title, const QStringList &labels) {
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0); l->setSpacing(0);

    m_lblTitle = new QLabel(title);
    m_lblTitle->setStyleSheet("font-size: 18px; color: #00AFFF; font-weight: bold; font-family: 'Segoe UI'; padding-left: 5px;");
    l->addWidget(m_lblTitle, 1);

    QWidget *leg = new QWidget(); QHBoxLayout *hl = new QHBoxLayout(leg); hl->setContentsMargins(5, 0, 5, 0);

    // 使用等宽字体，确保 '1', '8', '-', '+' 宽度完全一致
    QFont monoFont("Consolas", 12, QFont::Bold);
    // 如果系统没有 Consolas，备选方案
    if (!QFontInfo(monoFont).exactMatch()) {
        monoFont.setFamily("Monospace");
    }

    for(int i=0; i<3; ++i) {
        QLabel *c = new QLabel();
        c->setFixedSize(10, 3);
        c->setStyleSheet(QString("background:%1;").arg(m_lineColors[i].name()));

        // --- 核心修改：固定 QLabel 宽度并使用等宽字体 ---
        QLabel *v = new QLabel();
        v->setFixedWidth(110); // 根据标签文字长度（如 "Voltage: -00.0"）调整此数值
        v->setFont(monoFont);
        v->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        v->setStyleSheet(QString("color: %1;").arg(m_lineColors[i].name()));

        // 初始化文本
        v->setText(QString("%1:  0.0").arg(labels[i]));

        m_valLabels.append(v);
        hl->addWidget(c);
        hl->addWidget(v);
        hl->addSpacing(8);
    }
    hl->addStretch(); l->addWidget(leg, 1);
    m_plot = new QCustomPlot(); l->addWidget(m_plot, 8);
}

void WaveformPlot::setupStyle() {
    this->setStyleSheet("background-color: #000000;");
    m_plot->setBackground(Qt::black);
    m_plot->xAxis->setTickLabelFont(QFont("Segoe UI", 9, QFont::Bold));
    m_plot->yAxis->setTickLabelFont(QFont("Segoe UI", 11, QFont::Bold));
    m_plot->xAxis->setNumberFormat("f"); m_plot->xAxis->setNumberPrecision(1);
    m_plot->xAxis->ticker()->setTickCount(3);
    m_plot->axisRect()->setAutoMargins(QCP::msNone);
    m_plot->axisRect()->setMargins(QMargins(45, 5, 5, 22));

    QPen p(QColor(100, 100, 100)); m_plot->xAxis->setBasePen(p); m_plot->yAxis->setBasePen(p);
    m_plot->xAxis->setTickLabelColor(Qt::white); m_plot->yAxis->setTickLabelColor(Qt::white);
    m_plot->xAxis->grid()->setPen(QPen(QColor(50, 50, 50), 1, Qt::DotLine));
}

void WaveformPlot::addData(double ts, const QVector<double> &v) {
    if (m_isFrozen || v.size() < 3) return;
    if (m_startTime < 0) m_startTime = ts;
    double relTs = ts - m_startTime;

    for (int i = 0; i < 3; ++i) {
        m_plot->graph(i)->addData(relTs, v[i]);
        m_plot->graph(i)->data()->removeBefore(relTs - MAX_HISTORY_SEC);

        // --- 核心修改：格式化字符串，保证占位一致 ---
        // 使用 arg 的 fieldWidth 参数。例如 6 位：符号1位 + 数字2位 + 小数点1位 + 小数1位 = 5位，设为 6 更稳
        // 使用 QLatin1Char(' ') 在正数前补空格，或者用 QLatin1Char('+') 强制显示正号
        QString name = m_valLabels[i]->text().split(":").at(0);
        QString valStr = QString("%1").arg(v[i], 6, 'f', 1, QLatin1Char(' '));

        m_valLabels[i]->setText(QString("%1:%2").arg(name).arg(valStr));
    }
    m_plot->xAxis->setRange(relTs, 3.0, Qt::AlignRight);
    m_plot->replot(QCustomPlot::rpQueuedReplot);
}

void WaveformPlot::clearData() {
    m_startTime = -1.0;
    for (int i = 0; i < m_plot->graphCount(); ++i) {
        m_plot->graph(i)->data()->clear();
    }
    m_plot->replot();
}

void WaveformPlot::setYRange(double min, double max) {
    if (m_plot) {
        m_plot->yAxis->setRange(min, max);
        m_plot->replot();
    }
}
