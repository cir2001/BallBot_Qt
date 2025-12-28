#include "logpanel.h"
#include <QLabel>
#include <QScrollBar>

LogPanel::LogPanel(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupStyle();
}

void LogPanel::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 0, 5, 5); // 顶部 0 边距，增加显示面积
    mainLayout->setSpacing(5);

    // 顶部工具栏
    QHBoxLayout *toolLayout = new QHBoxLayout();

    // 1. 标题：18px Bold, #00AFFF
    QLabel *lblTitle = new QLabel("SYSTEM LOG");
    lblTitle->setObjectName("logTitle");

    m_checkAutoScroll = new QCheckBox("Auto Scroll");
    m_checkAutoScroll->setChecked(true);

    m_btnClear = new QPushButton("Clear Log");

    toolLayout->addWidget(lblTitle);
    toolLayout->addStretch();
    toolLayout->addWidget(m_checkAutoScroll);
    toolLayout->addWidget(m_btnClear);
    mainLayout->addLayout(toolLayout);

    // 2. 日志显示区
    m_textDisplay = new QPlainTextEdit();
    m_textDisplay->setReadOnly(true);
    // 使用 Consolas 确保数据对齐，字号设为 11pt 易于阅读
    m_textDisplay->setFont(QFont("Consolas", 11));
    m_textDisplay->setMaximumBlockCount(1000); // 性能优化
    mainLayout->addWidget(m_textDisplay);

    // 信号连接
    connect(m_btnClear, &QPushButton::clicked, this, &LogPanel::clearLog);
}

void LogPanel::setupStyle() {
    this->setStyleSheet(
        "QWidget { background-color: #000000; color: #DCDCDC; font-family: 'Segoe UI'; }"

        /* 标题风格对齐 TopStatusBar */
        "QLabel#logTitle { "
        "   color: #00AFFF; "
        "   font-size: 18px; "
        "   font-weight: bold; "
        "}"

        /* 文本框：科技蓝边框 + 深黑背景 */
        "QPlainTextEdit { "
        "   background-color: #050505; "
        "   border: 1px solid #333; "
        "   border-radius: 4px; "
        "   color: #00AFFF; " // 默认 Info 日志使用科技蓝
        "}"

        /* 按钮与复选框：16px Bold */
        "QPushButton { "
        "   background-color: #1A1A1A; "
        "   color: #FFFFFF; "
        "   border: 1px solid #444; "
        "   padding: 4px 12px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   border-radius: 2px; "
        "}"
        "QPushButton:hover { background-color: #333; border: 1px solid #00AFFF; }"

        "QCheckBox { color: #FFFFFF; font-weight: bold; spacing: 5px; }"

        /* 滚动条：细长深灰风格 */
        "QScrollBar:vertical { background: #000; width: 8px; }"
        "QScrollBar::handle:vertical { background: #333; border-radius: 4px; }"
        );
}

// 核心修复点：确保 LogPanel:: 作用域限定符存在
void LogPanel::clearLog() {
    m_textDisplay->clear();
}

void LogPanel::appendLog(const QString &msg, int level) {
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString color;

    switch (level) {
    case 1:  color = "#FFA500"; break; // Warning: 橙色
    case 2:  color = "#FF3333"; break; // Error: 亮红
    default: color = "#00AFFF"; break; // Info: 科技蓝
    }

    QString htmlLog = QString("<span style='color:#777;'>[%1]</span> <span style='color:%2; font-weight:bold;'>%3</span>")
                          .arg(timeStr, color, msg);

    m_textDisplay->appendHtml(htmlLog);

    if (m_checkAutoScroll->isChecked()) {
        m_textDisplay->moveCursor(QTextCursor::End);
    }
}
