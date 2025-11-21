// tmxmap.h - 地图解析器头文件
#ifndef TMXMAP_H
#define TMXMAP_H

#include <QObject>
#include <QVector>
#include <QDomDocument>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

/* 单个瓦片信息 */
struct Tile {
    int id;          // 全局 GID（Global ID）
    QRect source;    // 在图块集图片中的裁剪区域（x, y, w, h）
    QString image;   // 图块集图片路径（如 "tiles.png"）
};

/* 一个图层 */
struct Layer {
    QString name;
    int width;
    int height;
    QVector<int> data;
    /* data 存储的是每个格子的 GID
    顺序是：
    [ (0,0), (1,0), ..., (width-1,0), (0,1), (1,1), ... ]
    所以 (x, y) 对应的索引是：y * width + x
    */
};

class TmxMap : public QObject
{
    Q_OBJECT
public:
    explicit TmxMap(QObject *parent = nullptr);

    /* 解析 .tmx 文件，返回 true 表示成功 */
    bool load(const QString &fileName);

    /* 把解析结果画到 scene 上 */
    void buildScene(QGraphicsScene *scene);

    // 添加公共成员变量，以便在widget.cpp中访问
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    int m_mapWidth = 0;
    int m_mapHeight = 0;

private:
    /* 解析图块集（仅支持单个外部 TSX）
    图块集 = 一张大图 + 瓦片定义
    它把所有小瓦片（如草地、石头、树、水等）拼成一张大图（称为“图块集图片”），并告诉程序：
    “这张大图被切成了若干 32×32 的小格子，第1个是草地，第2个是石头……”
    图块集信息通常来自：
    外部文件：mytiles.tsx（XML 描述 + 引用 tiles.png）
    内联定义：直接写在 .tmx 中
    */
    bool parseTileset(const QDomElement &tilesetElem);

    /* 解析图层（仅支持 CSV 编码）
    图层 = 一张由瓦片组成的网格
    一张完整地图通常由 多个图层叠加而成，比如：
    地面层：草地、泥土（底层）
    装饰层：树、石头（中层）
    碰撞层：不可通行区域（逻辑层，可能不显示）
    对象层：NPC、宝箱位置（非瓦片，用矩形/点表示）
    */
    bool parseLayer(const QDomElement &layerElem);

    /* 解析内联图块集 */
    bool parseInlineTileset(const QDomElement &elem, int firstGid);
    //把瓦片存在m_tiles容器，图层存在m_layers容器
    QVector<Tile> m_tiles;     // 全局 id -> Tile
    QVector<Layer> m_layers;

    QString m_basePath;        // TMX文件所在目录，用于相对路径解析
};

#endif // TMXMAP_H
