#ifndef INVENTORYSLOT_H
#define INVENTORYSLOT_H

#include <QFrame>
#include "item.h"

class InventorySlot : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool hovered READ hovered WRITE setHovered)
public:
    explicit InventorySlot(QWidget *parent = nullptr);

    void setItem(const Item &item)  { m_item = item; update(); }
    void clear()                    { m_item = Item(); update(); }

    bool hovered() const            { return m_hovered; }
    void setHovered(bool h)         { m_hovered = h; update(); }

protected:
    void enterEvent(QEvent *) override { setHovered(true); }
    void leaveEvent(QEvent *) override { setHovered(false); }
    void mousePressEvent(QMouseEvent *) override { m_pressed = true; }
    void mouseReleaseEvent(QMouseEvent *) override { m_pressed = false; emit clicked(); }
    void paintEvent(QPaintEvent *) override;

signals:
    void clicked();          // 主窗口接收，当做“使用物品”

private:
    Item m_item;
    bool m_hovered = false;
    bool m_pressed = false;
};

#endif // INVENTORYSLOT_H
