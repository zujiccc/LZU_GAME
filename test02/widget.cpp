// widget.cpp - 主窗口实现
#include "widget.h"
#include "tmxmap.h"  // 在cpp文件中包含tmxmap.h，而不是在头文件中
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      m_scene(new QGraphicsScene(this)),
      m_view(new QGraphicsView(m_scene, this)),
      m_statusLabel(new QLabel("准备加载地图...")),
      m_map(new TmxMap(this))
{
    setWindowTitle("Qt TMX 瓦片地图 RPG 游戏");
    resize(500, 400);  // 增大窗口大小
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
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 设置视图缩放模式
    m_view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    loadMap();
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


    if (!m_scene->items().isEmpty()) {
        QRectF sceneRect = m_scene->itemsBoundingRect();//获取地图在场景中所占的像素范围sceneRect
        qreal viewWidth = m_view->width() - 20;   // 留出边距
        qreal viewHeight = m_view->height() - 20; // 留出状态标签和边距
        m_view->resetTransform();
        //缩放地图
        m_view->scale(0.595, 0.45);//在我电脑上一步一步试出来的缩放比例，不一定合适
        m_view->centerOn(sceneRect.center());//视图（QGraphicsView）的显示中心 移动到 场景中地图sceneRect的几何中心。
    }


    //%1和%2分别是是m_map->m_mapWidth，m_map->m_mapHeight的占位符
    //实际作用是在状态栏（比如窗口底部的 QLabel）显示一条成功提示信息，告诉用户地图的逻辑尺寸，例如："地图加载成功: 100x66 瓦片"
    m_statusLabel->setText(QString("地图加载成功: %1x%2 瓦片").arg(m_map->m_mapWidth).arg(m_map->m_mapHeight));
}

