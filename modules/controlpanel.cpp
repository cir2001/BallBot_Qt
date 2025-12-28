#include "controlpanel.h"
#include <QLabel>

ControlPanel::ControlPanel(QWidget *parent) : QWidget(parent) {
    setupStyle();
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *container = new QWidget();
    container->setObjectName("controlContainer");
    setupUi(container);
    rootLayout->addWidget(container);
}

void ControlPanel::setupUi(QWidget *container) {
    QHBoxLayout *mainLayout = new QHBoxLayout(container);
    mainLayout->setContentsMargins(10, 5, 10, 5);
    mainLayout->setSpacing(15);

    auto makeCol = [&](bool left) {
        QVBoxLayout *col = new QVBoxLayout();
        col->setSpacing(0);
        QGroupBox *topGrp = new QGroupBox(left ? "SYSTEM STATUS" : "MAINTENANCE");
        topGrp->setFixedHeight(145);
        QVBoxLayout *tl = new QVBoxLayout(topGrp);
        tl->setContentsMargins(10, 15, 10, 10); tl->setSpacing(6);

        if(left) {
            m_comboMode = new QComboBox(); m_comboMode->addItems({"STANDBY","BALANCE","WALK","AUTO"});
            m_btnStop = new QPushButton("EMERGENCY STOP"); m_btnStop->setObjectName("m_btnStop");
            tl->addWidget(new QLabel("Mode:")); tl->addWidget(m_comboMode); tl->addWidget(m_btnStop);
        } else {
            QPushButton *b1 = new QPushButton("Calibrate IMU"), *b2 = new QPushButton("Reset Position");
            tl->addWidget(b1); tl->addWidget(b2);
        }
        col->addWidget(topGrp);
        col->addSpacing(15);
        col->addWidget(createPidGroup(left?"BALANCE LOOP":"VELOCITY LOOP", left?0:1));
        col->addStretch();
        return col;
    };

    mainLayout->addLayout(makeCol(true), 1);
    mainLayout->addLayout(makeCol(false), 1);
}

QGroupBox* ControlPanel::createPidGroup(const QString &name, int type) {
    QGroupBox *grp = new QGroupBox(name); grp->setFixedHeight(155);
    QGridLayout *grid = new QGridLayout(grp);
    grid->setContentsMargins(8, 22, 8, 10); grid->setSpacing(6);
    QStringList tags = {"P","I","D"};
    for(int i=0; i<3; ++i) {
        QLabel *l = new QLabel(tags[i]); l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet("font-size:14px; color:#BBB; font-weight:bold;");
        QDoubleSpinBox *s = new QDoubleSpinBox(); s->setRange(0,5000); s->setDecimals(2);
        s->setAlignment(Qt::AlignCenter); s->setButtonSymbols(QAbstractSpinBox::NoButtons); s->setFixedHeight(32);
        grid->addWidget(l, 0, i); grid->addWidget(s, 1, i);
    }
    QPushButton *btn = new QPushButton("UPDATE"); btn->setFixedHeight(32);
    btn->setStyleSheet("font-size:14px; background:#004466; color:white;");
    grid->addWidget(btn, 2, 0, 1, 3);
    return grp;
}

void ControlPanel::setupStyle() {
    this->setStyleSheet(
        "QWidget#controlContainer { background-color: #000000; }"
        "QLabel { color: white; font-family: 'Segoe UI'; font-size: 16px; }"
        "QGroupBox { border: 1px solid #333; border-radius: 4px; margin-top: 20px; padding-top: 10px; font-size: 18px; font-weight: bold; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; color: #00AFFF; }"
        "QPushButton { background: #1A1A1A; color: white; border: 1px solid #444; min-height: 30px; font-size: 16px; font-weight: bold; }"
        "QPushButton#m_btnStop { background: #800000; border: 1px solid red; }"
        "QDoubleSpinBox, QComboBox { background: #1A1A1A; color: white; border: 1px solid #444; min-height: 30px; font-size: 16px; font-weight: bold; }"
        );
}
