
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGraphicsPixmapItem> // 添加头文件
#include "PlayerItem.h"
class TmxMap;   // 前向声明，避免循环 include

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void loadMap();      // 解析 tmx 并加载到场景
    void keyPressEvent(QKeyEvent *event) override; // ← 新增键盘事件
    void updatePlayerPosition();//辅助函数：更新玩家屏幕坐标

    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    QLabel *m_statusLabel;
    TmxMap *m_map; // 我们的解析器


    // ===== 新增玩家相关成员 =====
       PlayerItem *m_playerItem = nullptr; // 替换原来的 QGraphicsPixmapItem*
       int m_playerX = 0;
       int m_playerY = 0;
       bool m_isMoving = false;
};

#endif // WIDGET_H
