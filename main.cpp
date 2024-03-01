#include <QApplication>
#include <QMessageBox>

#include "MainWindow.h"
#include "TileLoader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Makes individual GET requests.
    NetworkController networkController;

    // Gets different URLs to download PBF tiles.
    TileLoader tileLoader;

     // Read key from file.
    QString mapTilerKey = tileLoader.readKey("key.txt");
    if(mapTilerKey == "") {
        // If we couldn't load the key, display an error box.
        QMessageBox::critical(
            nullptr,
            "Critical failure",
            "Unable to load file 'key.txt'. This key is necessary to contact MapTiler. The application will now shut down.");
        return -1;
    }

    // Picks stylesheet to load and loads it.
    auto styleSheetType = TileLoader::styleSheetType::basic_v2;
    auto styleSheetBytes = tileLoader.loadStyleSheetFromWeb(mapTilerKey, styleSheetType);

    // Gets the link template where we have to switch out x, y,z in the link.
    auto pbfLinkTemplate = tileLoader.getPbfLinkTemplate(styleSheetBytes, "maptiler_planet");

    // Creates the Widget that displays the map.
    auto *mapWidget = new MapWidget;
    auto &styleSheet = mapWidget->styleSheet;
    auto &tileStorage = mapWidget->tileStorage;

    // Parses the bytes that form the stylesheet into a json-document object.
    QJsonParseError parseError;
    auto styleSheetJsonDoc = QJsonDocument::fromJson(styleSheetBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return 0;
    }
    // Then finally parse the JSonDocument into our StyleSheet.
    styleSheet.parseSheet(styleSheetJsonDoc);

    auto downloadTile = [&](TileCoord tile) {
        return Bach::tileFromByteArray(
            tileLoader.downloadTile(
                tileLoader.setPbfLink(tile, pbfLinkTemplate),
            networkController));
    };

    // Download tiles, or load them from disk, then insert into the tile-storage map.
    auto tile000 = downloadTile({0, 0, 0});
    tileStorage.insert({0, 0, 0}, &tile000);
    auto tile100 = downloadTile({1, 0, 0});
    tileStorage.insert({1, 0, 0}, &tile100);
    auto tile101 = Bach::tileFromFile(Bach::testDataDir + "z1x0y1.mvt");
    tileStorage.insert({1, 0, 1}, &tile101);
    auto tile110 = downloadTile({1, 1, 0});
    tileStorage.insert({1, 1, 0}, &tile110);
    auto tile111 = Bach::tileFromFile(Bach::testDataDir + "z1x1y1.mvt");
    tileStorage.insert({1, 1, 1}, &tile111);

    // This can download entire zoom levels and insert them
    // Temporarily commented out, can be used for testing.
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

    // Main window setup
    auto app = Bach::MainWindow(mapWidget, downloadTile);
    app.show();

    return a.exec();
}
