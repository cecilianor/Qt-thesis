#include <QApplication>
#include <QPainter>

#include "VectorTiles.h"
#include "Rendering.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto tile000 = Bach::tileFromFile(Bach::testDataDir + "z0x0y0.mvt");

    QMap<TileCoord, VectorTile const*> tileStorage;
    tileStorage.insert({0, 0, 0}, &tile000);

    QImage image(2000, 2000, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);

    Bach::paintTiles(
        painter,
        0.5,
        0.5,
        0,
        0,
        tileStorage);

    image.save("output.png");

    return 0;
}
