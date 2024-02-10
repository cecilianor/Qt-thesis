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

    MapWidget mapWidget;
    auto& tileStorage = mapWidget.tileStorage;
    auto& layerColors = mapWidget.layerColors;

    auto tile000 = Bach::tileFromFile(Bach::testDataDir + "z0x0y0.mvt");
    tileStorage.insert({0, 0, 0}, &tile000);
    auto tile100 = Bach::tileFromFile(Bach::testDataDir + "z1x0y0.mvt");
    tileStorage.insert({1, 0, 0}, &tile100);
    auto tile101 = Bach::tileFromFile(Bach::testDataDir + "z1x0y1.mvt");
    tileStorage.insert({1, 0, 1}, &tile101);
    auto tile110 = Bach::tileFromFile(Bach::testDataDir + "z1x1y0.mvt");
    tileStorage.insert({1, 1, 0}, &tile110);
    auto tile111 = Bach::tileFromFile(Bach::testDataDir + "z1x1y1.mvt");
    tileStorage.insert({1, 1, 1}, &tile111);

    layerColors.insert("water", QColor(Qt::blue).lighter(75));
    layerColors.insert("landcover", QColor(Qt::lightGray).lighter());
    layerColors.insert("globallandcover", QColor(Qt::green).lighter(75));

    mapWidget.show();

    return a.exec();
}
