#include <QApplication>

#include <QFile>
#include <QImage>
#include <QPainter>
#include <QMap>

#include "VectorTiles.h"
#include "MapWidget.h"

QString testDataDir = "testdata/";

VectorTile tileFromFile(QString string) {
    auto file = QFile(testDataDir + string);
    file.open(QIODevice::ReadOnly);
    VectorTile tile;
    tile.DeserializeMessage(file.readAll());
    return tile;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto tile000 = tileFromFile("z0x0y0.mvt");
    auto tile100 = tileFromFile("z1x0y0.mvt");
    auto tile101 = tileFromFile("z1x0y1.mvt");
    auto tile110 = tileFromFile("z1x1y0.mvt");
    auto tile111 = tileFromFile("z1x1y1.mvt");

    MapWidget mapWidget;
    mapWidget.tileStorage.insert({0, 0, 0}, &tile000);
    mapWidget.tileStorage.insert({1, 0, 0}, &tile100);
    mapWidget.tileStorage.insert({1, 0, 1}, &tile101);
    mapWidget.tileStorage.insert({1, 1, 0}, &tile110);
    mapWidget.tileStorage.insert({1, 1, 1}, &tile111);

    mapWidget.show();

    return a.exec();
}
