// widget.cpp - 主窗口实现
#include "widget.h"
#include "tmxmap.h"  // 在cpp文件中包含tmxmap.h，而不是在头文件中
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
    setFocusPolicy(Qt::StrongFocus); // 允许接收键盘事件
    setFocus(); // 主动获取焦点
}

Widget::~Widget()
{
    delete m_map;  // 清理资源
}


void Widget::loadMap()
{
    QString tmxPath = "E:/tiled project/test.tmx";  // ← 需要修改的实际路径

    m_statusLabel->setText("正在加载地图: " + tmxPath);

    if (!m_map->load(tmxPath)) {
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
    if (m_isMoving || !m_map || !m_playerItem) {
        event->ignore();
        return;
    }

    int dx = 0, dy = 0;
    switch (event->key()) {
    case Qt::Key_Left:  dx = -1; break;
    case Qt::Key_Right: dx = 1;  break;
    case Qt::Key_Up:    dy = -1; break;
    case Qt::Key_Down:  dy = 1;  break;
    default:
        event->ignore();
        return;
    }

    int newX = m_playerX + dx;
    int newY = m_playerY + dy;

    if (newX < 0 || newX >= m_map->m_mapWidth ||
        newY < 0 || newY >= m_map->m_mapHeight) {
        return;
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
    connect(animPlayer, &QPropertyAnimation::finished, this, [this, newX, newY]() {
        m_playerX = newX;
        m_playerY = newY;
        m_isMoving = false;
    });
}
