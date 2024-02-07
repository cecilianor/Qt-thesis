#include <QFile>
#include <QImage>

#include "tilerenderrules.h"
#include "vectortile.h"
#include "tilerenderer.h"

#include "vector_tile.qpb.h"

int main(int argc, char *argv[])
{

    QJsonDocument doc;
    QJsonParseError parseError;

    QFile fStyle(":/testData/tiles.json");
    fStyle.open(QIODevice::ReadOnly);
    doc = QJsonDocument::fromJson(fStyle.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return 0;
    }

    TileRenderRules *rules = TileRenderRules::fromJson(doc);

    QFile fTile(":/testData/z12x2170y1190.mvt");
    fTile.open(QIODevice::ReadOnly);
    QByteArray data = fTile.readAll();

    VectorTile tile;
    tile.load(data);

//    QRect br = tile.boundingRect();
//    qDebug() << br;

    QImage image(QSize(4096, 4096), QImage::Format_RGBA8888);
    QPainter p(&image);

    TileRenderer renderer;
    renderer.m_rules = *rules;
    renderer.m_zoomLevel = 12;
    renderer.render(&p, tile);

    image.save("/home/matti/z12x2170y1190.png");
}
