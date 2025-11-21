#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#endif // PLAYERITEM_H

// PlayerItem.h
#pragma once
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>

class PlayerItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit PlayerItem(QObject *parent = nullptr)
        : QObject(parent), QGraphicsPixmapItem() {}

    // 允许设置图像
    void setPixmap(const QPixmap &pixmap) {
        QGraphicsPixmapItem::setPixmap(pixmap);
    }


};
