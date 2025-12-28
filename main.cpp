#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QDir>
#include <QPluginLoader>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QDebug>
#include <QMetaType>

int main(int argc, char *argv[])
{

    // 适配 1080p 高分屏缩放
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    /*****************************读取样式表********************************/
    // QFile qss("style/my.qss");
    // if( qss.open(QFile::ReadOnly))
    // {
    //     qDebug("open success");
    //     QString style = QLatin1String(qss.readAll());
    //     qApp->setStyleSheet(style);
    //     qss.close();
    // }
    // else
    // {
    //     qDebug("Open failed");
    // }

    qRegisterMetaType<uint32_t>("uint32_t");//防止跨线程（可能是 UDP 接收线程到 UI 线程）传递 uint32_t 类型的数据，但 Qt 的元对象系统默认不认识这个类型，导致信号槽调用失败。
    qRegisterMetaType<int16_t>("int16_t");
    qRegisterMetaType<int32_t>("int32_t");

    //加载全局样式
    QFile file(":/style/style.qss"); // 如果没用资源文件，直接读相对路径
    if(file.open(QFile::ReadOnly)) {
        QString style = QLatin1String(file.readAll());
        a.setStyleSheet(style);
        file.close();
    }

    MainWindow w;
    w.show();



    //w.move ((QApplication::desktop()->width() - w.width())/2,(QApplication::desktop()->height() - w.height())/2);


    return a.exec();

}
