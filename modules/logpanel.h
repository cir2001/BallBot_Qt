#ifndef LOGPANEL_H
#define LOGPANEL_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget *parent = nullptr);

public slots:
    // 核心接口：添加不同等级的日志
    void appendLog(const QString &msg, int level = 0); // 0:Info, 1:Warning, 2:Error
    void clearLog();

private:
    void setupUi();
    void setupStyle();

    QPlainTextEdit *m_textDisplay;
    QCheckBox *m_checkAutoScroll;
    QPushButton *m_btnClear;
};

#endif