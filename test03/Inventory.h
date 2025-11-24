#ifndef INVENTORY_H
#define INVENTORY_H

#include <QVector>
#include "Item.h"

const int INVENTORY_SIZE = 9; // 物品栏仍为 9 格

class Inventory {
public:
    Inventory() {
        m_items.resize(INVENTORY_SIZE); // 初始化 9 个空槽位
    }

    // 添加工具（不可堆叠，只能添加到空槽位）
    bool addItem(const Item &item) {
        if (!item.isValid()) return false;

        // 工具不可堆叠，直接找空槽位
        for (int i = 0; i < m_items.size(); ++i) {
            if (!m_items[i].isValid()) {
                m_items[i] = item;
                return true;
            }
        }

        // 物品栏满了，添加失败
        return false;
    }

    // 获取指定槽位物品
    Item getItem(int slotIndex) const {
        if (slotIndex < 0 || slotIndex >= m_items.size()) {
            return Item();
        }
        return m_items[slotIndex];
    }

    // 使用工具（工具不可消耗，仅返回 true 表示使用成功，后续扩展功能）
    bool useItem(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= m_items.size()) {
            return false;
        }

        Item &item = m_items[slotIndex];
        return item.isValid(); // 只要槽位有工具，就视为使用成功
    }

    // 清空指定槽位（丢弃工具）
    void clearSlot(int slotIndex) {
        if (slotIndex >= 0 && slotIndex < m_items.size()) {
            m_items[slotIndex] = Item();
        }
    }

    // 获取所有槽位
    QVector<Item> items() const { return m_items; }

private:
    QVector<Item> m_items;
};

#endif // INVENTORY_H
