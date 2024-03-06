#include <QApplication>
#include <QMessageBox>

#include "MainWindow.h"
#include "TileLoader.h"
#include "Utilities.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Makes individual GET requests.
    NetworkController networkController;

    // Gets different URLs to download PBF tiles.
    TileLoader tileLoader;
    // The style sheet type to load (can be many different types).
    auto StyleSheetType = StyleSheetType::basic_v2;

     // Read key from file.
    QString mapTilerKey = tileLoader.readKey("key.txt");
    if(mapTilerKey == "") {
        // If we couldn't load the key, display an error box.
        QMessageBox::critical(
            nullptr,
            "Internal Error",
            "Internal error. Contact support if the error persists. The application will now shut down.");
        // Add developer comments to QDebug, not to the end user/client.
        qDebug() << "Reading of the MapTiler key failed...";
        return EXIT_FAILURE;
    }

    // Tries to load the stylesheet.
    auto styleSheetBytes = tileLoader.loadStyleSheetFromWeb(mapTilerKey, StyleSheetType, networkController);
    if (styleSheetBytes.resultType != ResultType::success) {
        qWarning() << "There was an error: " << PrintResultTypeInfo(styleSheetBytes.resultType);
        QMessageBox::critical(
            nullptr,
            "Map Loading Failed",
            "The map failed to load. Contact support if the error persists. The application will now shut down.");
        return EXIT_FAILURE;
    }

    // Gets the link template where we have to switch out x, y,z in the link.
    auto pbfLinkTemplate = tileLoader.getPbfLinkTemplate(styleSheetBytes.response, "maptiler_planet", networkController);
    if (pbfLinkTemplate.resultType != ResultType::success) {
        qWarning() << "There was an error: " << PrintResultTypeInfo(pbfLinkTemplate.resultType);
        QMessageBox::critical(
            nullptr,
            "Map Loading Failed",
            "The map failed to load. Contact support if the error persists. The application will now shut down.");
        return EXIT_FAILURE;
    }

    // Creates the Widget that displays the map.
    auto *mapWidget = new MapWidget;
    auto &styleSheet = mapWidget->styleSheet;
    auto &tileStorage = mapWidget->tileStorage;


    /// REFACTOR HERE. Cecilia will discuss on an upcoming meeting how to handle this differently.
    // Parses the bytes that form the stylesheet into a json-document object.
    QJsonParseError parseError;
    auto styleSheetJsonDoc = QJsonDocument::fromJson(styleSheetBytes.response, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(
            nullptr,
            "Critical failure",
            QString("Error when parsing stylesheet as JSON. Parse error at 1%: 2%")
                .arg(parseError.offset)
                .arg(parseError.errorString()));
        return EXIT_FAILURE;
    }
    // Then finally parse the JSonDocument into our StyleSheet.
    styleSheet.parseSheet(styleSheetJsonDoc);
    /// REFACTOR END

    auto downloadTile = [&](TileCoord tile) -> VectorTile {
        auto result = Bach::tileFromByteArray(
            tileLoader.downloadTile(
                Bach::setPbfLink(tile, pbfLinkTemplate.link),
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
