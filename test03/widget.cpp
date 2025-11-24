// widget.cpp - 主窗口实现
#include "widget.h"
#include "tmxmap.h"// 在cpp文件中包含tmxmap.h，而不是在头文件中
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QPainter>
#include <QScrollBar>        // ← 新增：用于滚动条动画
#include <QEasingCurve>      // ← 新增：缓动曲线
#include "PlayerItem.h"
#include "Item.h"
#include "inventoryslot.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      m_scene(new QGraphicsScene(this)),
      m_view(new QGraphicsView(m_scene, this)),
      m_statusLabel(new QLabel("准备加载地图...")),
      m_map(new TmxMap(this))
{
    setWindowTitle("Qt TMX 瓦片地图 RPG 游戏");
    resize(1000, 800);  // 增大窗口大小
    /*使用 QVBoxLayout 垂直排列：
    顶部：状态标签（m_statusLabel）
    底部：地图视图（m_view）
    设置小边距（setContentsMargins(5,5,5,5)）让界面更紧凑。
    */
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(m_statusLabel);
    lay->addWidget(m_view);
    lay->setContentsMargins(5, 5, 5, 5);

    // 设置视图属性
    /*下列函数作用依次为
     抗锯齿，使图形边缘平滑
     缩放图片时使用双线性插值，避免像素块
     窗口缩放时以视图中心为锚点
     鼠标滚轮缩放时以鼠标位置为中心
     */
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //关键：隐藏滚动条
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 性能优化
    m_view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    m_view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    m_view->setCacheMode(QGraphicsView::CacheBackground);
    m_view->setFocusPolicy(Qt::NoFocus);

    // 设置视图缩放模式
    m_view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    loadMap();
    initInventoryUI();
    updateInventoryUI();
    setFocusPolicy(Qt::StrongFocus); // 允许接收键盘事件
    setFocus(); // 主动获取焦点
}

Widget::~Widget()
{
    delete m_map;  // 清理资源
}


void Widget::loadMap()
{
    QString tmxPath ="E:\\tiled\\myexmples\\c.tmx";  // ← 需要修改的实际路径

    m_statusLabel->setText("正在加载地图: " + tmxPath);

    if (!m_map->load(tmxPath))
    {
        QString errorMsg = "加载 TMX 失败！请检查文件路径和格式。";
        qWarning() << errorMsg;
        m_statusLabel->setText(errorMsg);
        QMessageBox::critical(this, "加载失败", errorMsg);
        return;
    }

    m_map->buildScene(m_scene);



       // 创建玩家图像（红色圆圈）
   QPixmap playerPixmap(32, 32);
   playerPixmap.fill(Qt::red); // 全红，无透明

   QPainter painter(&playerPixmap);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.setBrush(Qt::red);
   painter.drawEllipse(4, 4, 24, 24);

       // 创建 PlayerItem 并添加到场景
   m_playerItem = new PlayerItem(this);
   m_playerItem->setPixmap(playerPixmap);

   m_playerItem->setFlag(QGraphicsItem::ItemIsFocusable, false); // ← 关键
   m_playerItem->clearFocus();

   m_scene->addItem(m_playerItem);

       // 设置初始位置
   m_playerX = 5;
   m_playerY = 5;
   updatePlayerPosition();  // 更新屏幕坐标

       // 视角居中（仅初始化时调用一次）
   m_view->centerOn(m_playerItem);

       // 1. 菜刀（工具类型：菜刀，功能：砍树/破箱）
   QPixmap kitchenKnifeIcon("E:\\tiled\\myexmples\\caidao.png"); // 替换为你的菜刀图标路径
   Item kitchenKnife("崭新的菜刀", "菜刀", "可以切菜", kitchenKnifeIcon);
   m_playerItem->addItemToInventory(kitchenKnife);

       // 2. 锅铲（工具类型：锅铲，功能：炒菜/格挡）
   QPixmap spatulaIcon("E:\\tiled\\myexmples\\guochan.png"); // 替换为你的锅铲图标路径
   Item spatula("铁制锅铲", "锅铲", "烹饪必备", spatulaIcon);
   m_playerItem->addItemToInventory(spatula);

       // 3. 汤勺（工具类型：汤勺，功能：舀汤/挖宝）
   QPixmap ladleIcon("E:\\tiled\\myexmples\\mushao.png"); // 替换为你的汤勺图标路径
   Item ladle("木汤勺", "汤勺", "可以舀取汤", ladleIcon);
   m_playerItem->addItemToInventory(ladle);
    //%1和%2分别是是m_map->m_mapWidth，m_map->m_mapHeight的占位符
    //实际作用是在状态栏（比如窗口底部的 QLabel）显示一条成功提示信息，告诉用户地图的逻辑尺寸，例如："地图加载成功: 100x66 瓦片"
    m_statusLabel->setText(QString("地图加载成功: %1x%2 瓦片").arg(m_map->m_mapWidth).arg(m_map->m_mapHeight));


    qDebug() << "Player focusable:" << m_playerItem->flags().testFlag(QGraphicsItem::ItemIsFocusable);//测试
}

void Widget::updatePlayerPosition()
{
    if (!m_playerItem || !m_map) return;

    qreal x = m_playerX * m_map->m_tileWidth + m_map->m_tileWidth / 2.0;
    qreal y = m_playerY * m_map->m_tileHeight + m_map->m_tileHeight / 2.0;

    x -= m_playerItem->boundingRect().width() / 2.0;
    y -= m_playerItem->boundingRect().height() / 2.0;

    m_playerItem->setPos(x, y);
}

//实现键盘控制与平滑移动（无抽搐版本：使用滚动条动画）
void Widget::keyPressEvent(QKeyEvent *event)
{
    if (m_isMoving || !m_map || !m_playerItem)
    {
        event->ignore();
        return;
    }

    int dx = 0, dy = 0;
    bool isMoveKet = true;
    switch (event->key())
    {
    case Qt::Key_Left:  dx = -1; break;
    case Qt::Key_Right: dx = 1;  break;
    case Qt::Key_Up:    dy = -1; break;
    case Qt::Key_Down:  dy = 1;  break;
    default:
        isMoveKet = false;
        return;
    }
    if(isMoveKet)
    {
        int newX = m_playerX + dx;
        int newY = m_playerY + dy;

        //地图边界检测
        if (newX < 0 || newX >= m_map->m_mapWidth ||
            newY < 0 || newY >= m_map->m_mapHeight)
        {
            return;
        }

        //地图障碍物检测
        if (m_map->isObstacle(newX, newY))
        {
                return; // 是障碍物 → 不移动，直接返回
        }

        m_isMoving = true;

        const qreal tileW = m_map->m_tileWidth;
        const qreal tileH = m_map->m_tileHeight;
        const qreal playerW = m_playerItem->boundingRect().width();
        const qreal playerH = m_playerItem->boundingRect().height();

        // 计算玩家目标位置（强制整数像素，避免亚像素抖动）
        qreal targetPlayerX = qRound(newX * tileW + tileW / 2.0 - playerW / 2.0);
        qreal targetPlayerY = qRound(newY * tileH + tileH / 2.0 - playerH / 2.0);
        QPointF playerTargetPos(targetPlayerX, targetPlayerY);

        // 视图应滚动到的目标中心 = 玩家中心
        QPointF targetViewCenter(
        targetPlayerX + playerW / 2.0,
        targetPlayerY + playerH / 2.0
    );

        // 获取当前视图中心（场景坐标）
        QPointF currentViewCenter = m_view->mapToScene(m_view->viewport()->rect().center());

        // 计算滚动条目标值
        int currentH = m_view->horizontalScrollBar()->value();
        int currentV = m_view->verticalScrollBar()->value();
        int targetH = currentH + qRound(targetViewCenter.x() - currentViewCenter.x());
        int targetV = currentV + qRound(targetViewCenter.y() - currentViewCenter.y());

        // === 动画1：平滑移动玩家 ===
        QPropertyAnimation *animPlayer = new QPropertyAnimation(m_playerItem, "pos");
        animPlayer->setDuration(60);
        animPlayer->setEasingCurve(QEasingCurve::OutQuad); // 更自然的缓动
        animPlayer->setStartValue(m_playerItem->pos());
        animPlayer->setEndValue(playerTargetPos);

        // === 动画2：平滑滚动视图（通过滚动条）===
        QPropertyAnimation *animH = new QPropertyAnimation(m_view->horizontalScrollBar(), "value");
        QPropertyAnimation *animV = new QPropertyAnimation(m_view->verticalScrollBar(), "value");

        animH->setDuration(60);
        animV->setDuration(60);
        animH->setEasingCurve(QEasingCurve::OutQuad);
        animV->setEasingCurve(QEasingCurve::OutQuad);
        animH->setStartValue(currentH);
        animH->setEndValue(targetH);
        animV->setStartValue(currentV);
        animV->setEndValue(targetV);

        // 启动所有动画
        animPlayer->start(QAbstractAnimation::DeleteWhenStopped);
        animH->start(QAbstractAnimation::DeleteWhenStopped);
        animV->start(QAbstractAnimation::DeleteWhenStopped);

        // 动画结束后更新逻辑坐标
        connect(animPlayer, &QPropertyAnimation::finished, this, [this, newX, newY]()
        {
            m_playerX = newX;
            m_playerY = newY;
            m_isMoving = false;
        });
        return;
    }
    // 工具使用逻辑（数字键1-9）
    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9)
    {
        int slotIndex = event->key() - Qt::Key_1;
        Item item = m_playerItem->inventory().getItem(slotIndex);

        if (!item.isValid())
        {
            m_statusLabel->setText("槽位 " + QString::number(slotIndex+1) + " 无工具！");
            return;
        }

        // 使用工具
        bool used = m_playerItem->useInventoryItem(slotIndex);
        if (used)
        {
            QString toolType = item.toolType();
            if (toolType == "菜刀")
            {
                m_statusLabel->setText("使用【" + item.name() + "】→ " + item.description() + "（切了菜）");
            }
            else if (toolType == "锅铲")
            {
                m_statusLabel->setText("使用【" + item.name() + "】→ " + item.description() + "（抄了菜）");
            }
            else if (toolType == "汤勺")
            {
                m_statusLabel->setText("使用【" + item.name() + "】→ " + item.description() + "（舀取了汤）");
            }
        }
    }
}

void Widget::initInventoryUI()
{
    // 物品栏容器（底部半透明）
    m_inventoryWidget = new QWidget(this);
    m_inventoryWidget->setFixedSize(600, 80);
    m_inventoryWidget->setStyleSheet(R"(
        background-color: rgba(20, 20, 20, 200);
        border-radius: 12px;
    )");

    // 槽位布局（水平排列）
    QHBoxLayout *slotLayout = new QHBoxLayout(m_inventoryWidget);
    slotLayout->setContentsMargins(12, 8, 12, 8);
    slotLayout->setSpacing(10);

    // 创建9个槽位
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        InventorySlot *slot = new InventorySlot(this);
        connect(slot, &InventorySlot::clicked, this, [this, i]()
        {
            if (m_playerItem)
            m_playerItem->useInventoryItem(i);

        });
        m_inventorySlots.append(slot);
        slotLayout->addWidget(slot);
    }

    // 添加到主布局（底部居中）
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addStretch();
        mainLayout->addWidget(m_inventoryWidget, 0, Qt::AlignCenter);
        mainLayout->setSpacing(20);
    }
}

// 更新物品栏显示
void Widget::updateInventoryUI()
{
    if (!m_playerItem) return;
    const Inventory &inv = m_playerItem->inventory();
    for (int i = 0; i < INVENTORY_SIZE; ++i)
        m_inventorySlots[i]->setItem(inv.getItem(i));
}
