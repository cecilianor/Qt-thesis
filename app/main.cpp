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
    auto styleSheetBytes = tileLoader.loadStyleSheetFromWeb(mapTilerKey, styleSheetType, networkController);

    // Gets the link template where we have to switch out x, y,z in the link.
    auto pbfLinkTemplate = tileLoader.getPbfLinkTemplate(styleSheetBytes, "maptiler_planet", networkController);

    // Creates the Widget that displays the map.
    auto *mapWidget = new MapWidget;
    auto &styleSheet = mapWidget->styleSheet;
    auto &tileStorage = mapWidget->tileStorage;

    // Parses the bytes that form the stylesheet into a json-document object.
    QJsonParseError parseError;
    auto styleSheetJsonDoc = QJsonDocument::fromJson(styleSheetBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(
            nullptr,
            "Critical failure",
            QString("Error when parsing stylesheet as JSON. Parse error at 1%: 2%")
                .arg(parseError.offset)
                .arg(parseError.errorString()));
        return -1;
    }
    // Then finally parse the JSonDocument into our StyleSheet.
    styleSheet.parseSheet(styleSheetJsonDoc);

    auto downloadTile = [&](TileCoord tile) -> VectorTile {
        auto result = Bach::tileFromByteArray(
            tileLoader.downloadTile(
                Bach::setPbfLink(tile, pbfLinkTemplate),
            networkController));
        if (!result.has_value()) {
            std::abort();
        }
        return result.value();
    };

    // Download tiles, or load them from disk, then insert into the tile-storage map.
    auto tile000 = downloadTile({0, 0, 0});
    tileStorage.insert({0, 0, 0}, &tile000);

    // Main window setup
    auto app = Bach::MainWindow(mapWidget, downloadTile);
    app.show();

    return a.exec();
}
