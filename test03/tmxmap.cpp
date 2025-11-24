/* tmxmap.cpp - 地图解析器实现
解析图块集 → 构建 m_tiles（GID → 图像裁剪位置）
解析图层 → 构建 m_layers（每层的 GID 网格）
buildScene() → 遍历每层，查 GID，贴图，叠加
*/
#include "tmxmap.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>

TmxMap::TmxMap(QObject *parent) : QObject(parent) {}

bool TmxMap::load(const QString &fileName)
{
    QFile file(fileName);
    //只读模式+文本模式
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Cannot open" << fileName;
        return false;
    }

    // 获取文件所在目录(不包括文件名本身)，用于解析相对路径
    m_basePath = QFileInfo(fileName).absolutePath();


    /*
      整个部分的作用:把打开的TMX文件解析成qt能操作的XML文件
      QDomDocument doc;
      这里创建了一个QDomDocument对象doc。用于存储和操作解析后的XML文件。QDomDocument是Qt框架中用于处理XML文档的类，它提供了对XML文档进行操作的方法。
      doc.setContent(&file, &err, &el, &ec)：从打开的文件对象 file 中读取内容，并解析为 XML 结构；
      QString err; int el, ec;
      定义了三个变量：err是一个QString类型的变量，用来存储可能出现的错误信息；el和ec是两个整数类型的变量，分别代表错误发生的行号（line）和列号（column）。
    */
    QDomDocument doc;
    QString err;
    int el, ec;
    if (!doc.setContent(&file, &err, &el, &ec))
    {
        qWarning() << "XML error:" << err << "at" << el << ec;
        return false;
    }
    file.close();

    /*
    整个部分作用:确保当前文件是标准的 TMX 文件，而非其他 XML 文件；
    documentElement() 是 QDomDocument 的成员函数，它返回 XML 文档的根元素（root element）。
    在 .tmx 文件中，根元素必须是 <map>
    */
    QDomElement root = doc.documentElement();
    if (root.tagName() != "map")
    {
        qWarning() << "Not a TMX file";
        return false;
    }

    /* 1. 地图头 */
    //整个部分作用:读取地图的核心基础参数，存入成员变量,并校验变量是否正确;
    /*TMX文件的根元素中是:
        <map width="100" height="80" tilewidth="32" tileheight="32">
        <!-- 其他子元素（tileset、layer等） -->
        </map>
      此处用attribute()函数解析前四个元素，解析出来是字符串类型，用toInt函数转换为int类型;
    */
    m_mapWidth = root.attribute("width").toInt();  //地图宽度
    m_mapHeight = root.attribute("height").toInt();  //地图高度
    m_tileWidth = root.attribute("tilewidth").toInt();  //单个瓦片宽度
    m_tileHeight = root.attribute("tileheight").toInt();  //单个瓦片高度

    if (m_tileWidth <= 0 || m_tileHeight <= 0 || m_mapWidth <= 0 || m_mapHeight <= 0) {
        qWarning() << "Invalid map dimensions";
        return false;
    }

    /* 2. 图块集
    在 <map> 根元素下，查找所有标签名为 "tileset" 的子元素。
    返回一个 QDomNodeList（节点列表），里面可能有 1 个、2 个或多个 <tileset> 节点。
    parseTileset(...)函数，专门解析一个 <tileset> 节点。
    它会处理：
    是内联图块集（直接包含 <image>）
    还是外部引用（source="xxx.tsx"）
    并最终将瓦片信息存入 m_tiles。
    m_tiles是TmxMap中的Tile结构体的对象：
    QVector<Tile> m_tiles;(这是定义语句)
    struct Tile
    {
        int id;             // 瓦片的全局唯一 ID（GID）
        QString image;      // 瓦片所在的原图（瓦片图集）路径
        QRect source;       // 瓦片在原图中的裁剪区域（x,y,宽,高）
    };
    */

    QDomNodeList tilesetNodes = root.elementsByTagName("tileset");
    for (int i = 0; i < tilesetNodes.size(); ++i) {
        if (!parseTileset(tilesetNodes.at(i).toElement()))
            return false;
    }



    /* 3. 图层
    查找所有 <layer> 子元素。
    Tiled 中每新建一个“图层”，就会生成一个 <layer>。
    parseLayer(...)
    解析单个图层的数据，主要是：
    图层名称、尺寸
    <data encoding="csv">1,2,0,3,...</data> 中的 GID 网格
    结果存入 m_layers
    */
    QDomNodeList layerNodes = root.elementsByTagName("layer");
    for (int i = 0; i < layerNodes.size(); ++i) {
        if (!parseLayer(layerNodes.at(i).toElement()))
            return false;
    }

    qDebug() << "Successfully loaded map:" << m_mapWidth << "x" << m_mapHeight
             << "with" << m_layers.size() << "layers and" << m_tiles.size() << "tiles";
    return true;
}

//parseTileset函数
//解析一个 <tileset> 图块集节点（无论是内嵌在 .tmx 中，还是外部引用的 .tsx 文件），并将其包含的所有瓦片信息加载到内存中，
//为后续地图渲染提供“GID → 图像裁剪位置”的映射基础
bool TmxMap::parseTileset(const QDomElement &ts)
{
    //firstgid 是 Tiled 的关键属性，表示这个图块集的第一个瓦片的全局 ID（GID）
    int firstGid = ts.attribute("firstgid").toInt();

    /* 1. 如果路径是外部 TSX */
    if (ts.hasAttribute("source"))
    {
        QString tsxFile = ts.attribute("source");
        // 如果是相对路径，需要相对于TMX文件路径
        if (!tsxFile.startsWith(":/") && !tsxFile.startsWith("/"))
        {
            //m_basePath 是 .tmx 文件的绝对目录（在 load() 中已设置）
            //使用 QDir(m_basePath).absoluteFilePath(tsxFile) 将相对路径转为绝对路径
            tsxFile = QDir(m_basePath).absoluteFilePath(tsxFile);
        }


        /*打开 .tsx 文件
        用 QDomDocument 解析其内容
        获取根元素（即 <tileset>）
        调用 parseInlineTileset ——
        */
        QFile tsx(tsxFile);
        if (!tsx.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << "Cannot open external tsx:" << tsxFile;
            return false;
        }
        //XML 文档对象。
        QDomDocument tsxDoc;
       // 从已打开的 QFile 对象 tsx 中读取数据，并尝试解析为 XML 文档。
        if (!tsxDoc.setContent(&tsx))
        {
            qWarning() << "Invalid tsx XML";
            return false;
        }
        tsx.close();
        //根据解析后的XML文档，为每个瓦片分配GID
        return parseInlineTileset(tsxDoc.documentElement(), firstGid);
    }

    /* 2. 如果tmx内嵌tsx tileset，就直接为每个瓦片分配GID */
    return parseInlineTileset(ts, firstGid);
}

/*  parseLayer函数
1   读取图层名称和尺寸
2	校验尺寸合法性
3	检查是否为支持的 CSV 编码
4	提取并清洗 CSV 文本
5	验证数据量是否匹配
6	转换字符串为 GID 整数数组
7	存入全局图层列表
8   解析障碍物图层
*/
bool TmxMap::parseLayer(const QDomElement &layerElem)
{
    Layer lay;
    lay.name = layerElem.attribute("name");
    lay.width = layerElem.attribute("width").toInt();
    lay.height = layerElem.attribute("height").toInt();
    //校验图层尺寸是否匹配地图整体尺寸
    if (lay.width != m_mapWidth || lay.height != m_mapHeight)
    {
        qWarning() << "Layer dimensions mismatch:" << lay.name;
        return false;
    }

    QDomElement data = layerElem.firstChildElement("data");
    QString encoding = data.attribute("encoding");
    if (encoding != "csv")
    {
        qWarning() << "Only CSV encoding is supported, got:" << encoding;
        return false;
    }

    QString csv = data.text();
    csv.remove('\n').remove('\r');
    const auto tiles = csv.split(',');
    if (tiles.size() != lay.width * lay.height)
    {
        qWarning() << "Data size mismatch for layer" << lay.name;
        return false;
    }

    lay.data.reserve(tiles.size());
    for (const QString &s : tiles)
    {
        bool ok;
        int gid = s.trimmed().toInt(&ok);
        if (!ok) {
            qWarning() << "Invalid tile ID in layer" << lay.name << ":" << s;
            return false;
        }
        lay.data.append(gid);
    }
    if (lay.name == "Obstacle") //障碍物层
    {
        m_obstacleLayerIndex = m_layers.size(); // 记录当前图层索引
    }

    m_layers.append(lay);
    return true;
}


//将各个图层的瓦片添加到场景中，并设置场景边界大小
void TmxMap::buildScene(QGraphicsScene *scene)
{
    if (!scene) return;

    scene->clear();
    //遍历所有图层（Layer）先绘制的图层在底层（如地面,后绘制的在上层（如装饰物、角色）
    for (const Layer &lay : m_layers)
    {
        for (int y = 0; y < lay.height; ++y)
        {
            for (int x = 0; x < lay.width; ++x)
            {
                //获取图层中每一个瓦片的gid
                int gid = lay.data[y * lay.width + x];
                if (gid == 0) continue;   // 0 表示空瓦片

                const Tile *t = nullptr;
                //通过gid在瓦片集中寻找对应的瓦片
                for (const Tile &tile : m_tiles) {
                    if (tile.id == gid) {
                        t = &tile;
                        break;
                    }
                }

                if (!t) {
                    qWarning() << "Tile ID not found:" << gid;
                    continue;
               }

                // 加载瓦片图片并裁剪
                QString imagePath = t->image;
                //路径检查
                if (!imagePath.startsWith(":/") && !imagePath.startsWith("/")) {
                    imagePath = QDir(m_basePath).absoluteFilePath(imagePath);
                }
                //从指定的文件路径 imagePath 加载一张图像，并将其存储在 QPixmap 对象 tilePixmap 中，供后续绘制使用。
                QPixmap tilePixmap(imagePath);

                if (tilePixmap.isNull()) {
                    qWarning() << "Cannot load image:" << imagePath;
                    // 如果图片加载失败，绘制一个彩色矩形作为占位符
                    QGraphicsRectItem *item = scene->addRect(
                        x * m_tileWidth, y * m_tileHeight,
                        m_tileWidth, m_tileHeight,
                        QPen(Qt::black),
                        QColor((gid * 37) % 255, (gid * 61) % 255, (gid * 113) % 255)
                    );
                    continue;
                }

                // 从图块集中裁剪出指定瓦片
                //t->source 是一个 QRect，表示该瓦片在大图中的位置和尺寸（如 (32, 0, 32, 32)）
                //copy() 提取子图像
                QPixmap subPixmap = tilePixmap.copy(t->source);
                //添加到场景并定位
                QGraphicsPixmapItem *item = scene->addPixmap(subPixmap);
                item->setPos(x * m_tileWidth, y * m_tileHeight);
            }
        }
    }

    // 设置场景大小
    scene->setSceneRect(0, 0, m_mapWidth * m_tileWidth, m_mapHeight * m_tileHeight);
}


//解析一个内联（或已加载的外部）图块集（tileset）XML 元素，并为每个瓦片分配 GID 和裁剪区域
bool TmxMap::parseInlineTileset(const QDomElement &tilesetElem, int firstGid)
{
    int tw = tilesetElem.attribute("tilewidth").toInt();
    int th = tilesetElem.attribute("tileheight").toInt();
    int columns = tilesetElem.attribute("columns").toInt();
    int tileCount = tilesetElem.attribute("tilecount").toInt();

    if (tw <= 0 || th <= 0 || tileCount <= 0) {
        qWarning() << "Invalid tileset dimensions";
        return false;
    }

    QDomElement imageElem = tilesetElem.firstChildElement("image");
    if (imageElem.isNull()) {
        qWarning() << "Tileset without <image>";
        return false;
    }

    QString imgPath = imageElem.attribute("source");
    if (imgPath.isEmpty()) {
        qWarning() << "Image source is empty";
        return false;
    }

    if (columns <= 0) {
        // 如果没有columns属性，根据图像尺寸计算
        QPixmap tempPixmap(imgPath);
        if (!tempPixmap.isNull()) {
            columns = tempPixmap.width() / tw;
        } else {
            // 如果图片加载失败，使用默认值
            columns = 10; // 假设默认有10列
        }
    }

    for (int i = 0; i < tileCount; ++i) {
        Tile t;
        t.id = firstGid + i;
        t.image = imgPath;
        int row = i / columns;
        int col = i % columns;
        t.source = QRect(col * tw, row * th, tw, th);
        m_tiles.append(t);
    }

    qDebug() << "Loaded tileset with" << tileCount << "tiles from" << imgPath;
    return true;
}
/*
 判断障碍物的接口
*/
bool TmxMap::isObstacle(int tileX, int tileY) const
{
    // 1. 边界检测：坐标超出地图范围 → 不是障碍物（避免越界）
    if (tileX < 0 || tileX >= m_mapWidth || tileY < 0 || tileY >= m_mapHeight)
    {
        return false;
    }

    // 2. 无障碍物图层 → 不是障碍物
    if (m_obstacleLayerIndex == -1)
    {
        return false;
    }

    // 3. 获取障碍物图层的数据
    const Layer &obstacleLayer = m_layers[m_obstacleLayerIndex];
    // 4. 计算该瓦片在图层数据中的索引（data 是一维数组，存储顺序：行优先）
    int index = tileY * obstacleLayer.width + tileX;
    // 5. GID != 0 → 该位置有障碍物瓦片（Tiled 中绘制的瓦片 GID 不为 0）
    return obstacleLayer.data[index] != 0;
}
