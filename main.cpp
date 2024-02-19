#include <QApplication>

#include <QFile>
#include <QImage>
#include <QPainter>
#include <QMap>

#include "MapWidget.h"

#include "TileURL.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TileURL tileURL; // Gets different URLs to download PBF tiles

    QString mapTilerKey = tileURL.readKey("key.txt"); // Read key from file
        if(mapTilerKey == "") {return 0;}

    std::pair<QByteArray, TileURL::ErrorCode> styleSheetURL =
        tileURL.getStylesheet(TileURL::styleSheetType::basic_v2, mapTilerKey);
    auto responseData = styleSheetURL;
        if (responseData.second != TileURL::ErrorCode::success) {
            qWarning() << "There was an error: " << responseData.first;
            return 0;
        }

        // Parse the stylesheet
        QJsonParseError parseError;
        QJsonDocument styleSheetJson = QJsonDocument::fromJson(responseData.first, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
            return 0;
        }

        // Grab link to tiles.json format link
        std::pair<QString, TileURL::ErrorCode> tilesLink = tileURL.getTilesLink(styleSheetJson, "maptiler_planet");
        qDebug () <<"\n\tLink to tiles: " << tilesLink.first;

        // Grab link to the XYZ PBF tile format based on the tiles.json link
        std::pair<QString, TileURL::ErrorCode> pbfLink = tileURL.getPBFLink(tilesLink.first);
        qDebug () <<"\n\tLink to the PBF tiles: " << pbfLink.first;

        auto download = [&](TileCoord coord) -> QByteArray
        {
        // Exchange the {x, y z} in link

        auto fn = [=](TileCoord const& tileCoord) -> QString {
            auto temp = pbfLink.first;
            temp.replace("{z}", QString::number(tileCoord.zoom));
            temp.replace("{x}", QString::number(tileCoord.x));
            temp.replace("{y}", QString::number(tileCoord.y));
            return temp;
        };
        NetworkController controller;
        return controller.sendRequest(fn(coord));
    };

    MapWidget mapWidget;
    auto& styleSheet = mapWidget.styleSheet;
    auto& tileStorage = mapWidget.tileStorage;

    QJsonDocument doc;
    //QJsonParseError parseError;
    QFile fStyle(Bach::testDataDir + "tiles.json");
    fStyle.open(QIODevice::ReadOnly);
    doc = QJsonDocument::fromJson(fStyle.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return 0;
    }
    styleSheet.parseSheet(doc);

    auto tile000 = Bach::tileFromByteArray(download({0, 0, 0}));
    tileStorage.insert({0, 0, 0}, &tile000);
    auto tile100 = Bach::tileFromByteArray(download({1, 0, 0}));
    tileStorage.insert({1, 0, 0}, &tile100);
    auto tile101 = Bach::tileFromFile(Bach::testDataDir + "z1x0y1.mvt");
    tileStorage.insert({1, 0, 1}, &tile101);
    auto tile110 = Bach::tileFromByteArray(download({1, 1, 0}));
    tileStorage.insert({1, 1, 0}, &tile110);
    auto tile111 = Bach::tileFromFile(Bach::testDataDir + "z1x1y1.mvt");
    tileStorage.insert({1, 1, 1}, &tile111);

    /*
    auto tileCount = 1 << 2;
    for (int x = 0; x < tileCount; x++) {
        for (int y = 0; y < tileCount; y++) {
            auto temp = new VectorTile(Bach::tileFromByteArray(download({2, x, y})));
            tileStorage.insert({2, x, y}, temp);
        }
    }

    {
        auto tileCount = 1 << 3;
        for (int x = 0; x < tileCount; x++) {
            for (int y = 0; y < tileCount; y++) {
                auto temp = new VectorTile(Bach::tileFromByteArray(download({3, x, y})));
                tileStorage.insert({3, x, y}, temp);
            }
        }
    }
    */



    mapWidget.show();

    return a.exec();
}
