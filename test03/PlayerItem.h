
// PlayerItem.h
#pragma once
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include "Inventory.h"

class PlayerItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit PlayerItem(QObject *parent = nullptr)
        : QObject(parent), QGraphicsPixmapItem() {}

    // 允许设置图像
    void setPixmap(const QPixmap &pixmap)
    {
        QGraphicsPixmapItem::setPixmap(pixmap);
    }


    Inventory &inventory() { return m_inventory; }
        const Inventory &inventory() const { return m_inventory; }
        bool addItemToInventory(const Item &item) {
            return m_inventory.addItem(item);
        }
        bool useInventoryItem(int slotIndex) {
            return m_inventory.useItem(slotIndex);
        }

    signals:
        void inventoryChanged();//物品栏变化信号

    private:
        Inventory m_inventory;//玩家物品栏

};
