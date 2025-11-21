// widget.h - 主窗口头文件
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

class TmxMap;   // 前向声明，避免循环 include

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void loadMap();      // 解析 tmx 并加载到场景

    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    QLabel *m_statusLabel;
    TmxMap *m_map; // 我们的解析器
};

#endif // WIDGET_H
