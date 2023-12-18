#include "mainwindow.h"
#include <QApplication>
#include <QTextBlock>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    // 注册类型
    //    qRegisterMetaType<QTextCursor>("QTextCursor");
    //    qRegisterMetaType<QTextBlock>("QTextBlock");
    MainWindow w;
    w.show();
    return a.exec();
}
