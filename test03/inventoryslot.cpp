#include "inventoryslot.h"
#include <QPainter>
#include <QRadialGradient>

InventorySlot::InventorySlot(QWidget *parent) : QFrame(parent)
{
    setFixedSize(64, 64);
    setStyleSheet(R"(
        InventorySlot {
            border: 2px solid rgba(120,120,120,180);
            border-radius: 8px;
            background: rgba(30,30,30,220);
        }
        InventorySlot[hovered="true"] {
            border: 2px solid #4ecca3;
            background: rgba(40,40,40,240);
        }
    )");
    setAttribute(Qt::WA_Hover, true);  // 启用 hover 事件
}

void InventorySlot::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 外发光
    if (m_hovered) {
        QRadialGradient g(rect().center(), 40);
        g.setColorAt(0, QColor("#4ecca3").lighter());
        g.setColorAt(1, Qt::transparent);
        p.fillRect(rect().adjusted(-4,-4,4,4), g);
    }

    // 空槽位提示
    if (!m_item.isValid()) {
        p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
        p.drawRoundedRect(rect().adjusted(4,4,-4,-4), 6, 6);
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, "+");
        return;
    }

    // 画图标
    QPixmap icon = m_item.icon().scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.drawPixmap((width() - icon.width()) / 2, (height() - icon.height()) / 2, icon);

    // 数量
    p.setPen(Qt::white);
    p.drawText(rect().adjusted(-4,-4,0,0), Qt::AlignBottom | Qt::AlignRight,
               QString::number(m_item.count()));

    // 耐久条（示例：Item 需新增 maxDurability()/durability() ）
    int maxD = 100;  // 先写死，后续把 Item 扩展这两个接口
    int curD = 80;
    if (maxD > 0) {
        int w = width() * curD / maxD;
        p.fillRect(0, height() - 4, w, 4, QBrush("#4ecca3"));
    }
}
