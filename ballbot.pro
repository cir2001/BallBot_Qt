#-------------------------------------------------
#
# Project created by QtCreator 2025-12-12 T20:27:51
#
#-------------------------------------------------
QT      += core gui printsupport widgets
QT      += serialport
QT      += sql
QT      += svg
QT      += network
QT      += opengl
LIBS += -lopengl32 -lglu32
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DataProcess/dataprocessor.cpp \
    main.cpp \
    mainwindow.cpp \
    modules/attitudeglwidget.cpp \
    modules/controlpanel.cpp \
    modules/joystickwidget.cpp \
    modules/logpanel.cpp \
    modules/mainplot.cpp \
    modules/topstatusbar.cpp \
    modules/waveformplot.cpp \
    plot/qcustomplot.cpp \

HEADERS += \
    DataProcess/dataprocessor.h \
    mainwindow.h \
    modules/attitudeglwidget.h \
    modules/controlpanel.h \
    modules/joystickwidget.h \
    modules/logpanel.h \
    modules/mainplot.h \
    modules/topstatusbar.h \
    modules/waveformplot.h \
    plot/qcustomplot.h \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    README.md
