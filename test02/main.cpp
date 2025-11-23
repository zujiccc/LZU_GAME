// main.cpp - 应用程序入口
#include <QApplication>
#include "widget.h"
#include "StartWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    StartWidget startWidget;
    startWidget.show();

    Widget w;

    QObject::connect(&startWidget, &StartWidget::startGame, [&]()
    {
            startWidget.close(); // 关闭开始界面
            w.show();   // 显示游戏界面
    });
    return a.exec();
}
