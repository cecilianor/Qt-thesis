#include <QApplication>

#include <QFile>
#include <QImage>
#include <QPainter>
#include <QMap>

#include "VectorTiles.h"
#include "MapWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto tile000 = Bach::tileFromFile(Bach::testDataDir + "z0x0y0.mvt");
    auto tile100 = Bach::tileFromFile(Bach::testDataDir + "z1x0y0.mvt");
    auto tile101 = Bach::tileFromFile(Bach::testDataDir + "z1x0y1.mvt");
    auto tile110 = Bach::tileFromFile(Bach::testDataDir + "z1x1y0.mvt");
    auto tile111 = Bach::tileFromFile(Bach::testDataDir + "z1x1y1.mvt");

    MapWidget mapWidget;
    mapWidget.tileStorage.insert({0, 0, 0}, &tile000);
    mapWidget.tileStorage.insert({1, 0, 0}, &tile100);
    mapWidget.tileStorage.insert({1, 0, 1}, &tile101);
    mapWidget.tileStorage.insert({1, 1, 0}, &tile110);
    mapWidget.tileStorage.insert({1, 1, 1}, &tile111);

    mapWidget.show();

    return a.exec();
}
