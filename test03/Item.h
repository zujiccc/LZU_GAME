#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QPixmap>

class Item {
public:
    Item() = default;
    // 构造函数：工具类物品（不可堆叠，数量固定为 1）
    Item(const QString &name, const QString &toolType, const QString &desc, const QPixmap &icon)
        : m_name(name), m_toolType(toolType), m_desc(desc), m_icon(icon), m_count(1) {}

    // Getter 方法
    QString name() const { return m_name; }       // 物品名称（如“生锈的菜刀”）
    QString toolType() const { return m_toolType; } // 工具类型（菜刀/锅铲/汤勺，统一标识）
    QString description() const { return m_desc; }  // 工具描述（如“砍树专用”）
    QPixmap icon() const { return m_icon; }
    int count() const { return m_count; }         // 工具不可堆叠，数量固定为 1

    // 工具类物品不可修改数量（重写 setCount 禁止堆叠）
    void setCount(int count) { m_count = 1; } // 强制数量为 1
    void addCount(int num) { /* 工具不可堆叠，不响应 */ }
    void reduceCount(int num) { /* 工具不可消耗，不响应 */ }

    // 判断物品是否有效
    bool isValid() const { return !m_name.isEmpty() && !m_icon.isNull() && !m_toolType.isEmpty(); }

private:
    QString m_name;        // 物品名称（如“锋利的锅铲”）
    QString m_toolType;    // 核心：工具类型（仅支持“菜刀”“锅铲”“汤勺”）
    QString m_desc;        // 工具功能描述
    QPixmap m_icon;        // 工具图标
    int m_count = 0;       // 数量（工具固定为 1）
};

#endif // ITEM_H
