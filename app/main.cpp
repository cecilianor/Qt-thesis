#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

#include "MainWindow.h"
#include "TileLoader.h"
#include "Utilities.h"

#include <QNetworkReply>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Read key from file.
    std::optional<QString> mapTilerKeyResult = Bach::readMapTilerKey("key.txt");
    if (!mapTilerKeyResult.has_value()) {
        // Add developer comments to QDebug, not to the end user/client.
        qWarning() << "Reading of the MapTiler key failed...";
    }

    // The style sheet type to load (can be many different types).
    auto styleSheetType = StyleSheetType::basic_v2;

    HttpResponse styleSheetBytes = Bach::loadStyleSheetBytes(styleSheetType, mapTilerKeyResult);
    // If we couldn't load the style sheet, we can't display anything.
    // So we quit.
    if (styleSheetBytes.resultType != ResultType::success) {
        QMessageBox::critical(
            nullptr,
            "Unexpected error.",
            "Application will now quit.");
        return EXIT_FAILURE;
    }

    // Gets different URLs to download PBF tiles.
    TileLoader tileLoader;

    // Gets the link template where we have to switch out x, y,z in the link.
    auto pbfLinkTemplate = tileLoader.getPbfLinkTemplate(styleSheetBytes.response, "maptiler_planet");
    qDebug() << "Getting the link to download PPF tiles from MapTiler...\n";
    if (pbfLinkTemplate.resultType != ResultType::success) {
        qWarning() << "Loading tiles failed: There was an error getting PBF link template: " << PrintResultTypeInfo(pbfLinkTemplate.resultType);
        QMessageBox::critical(
            nullptr,
            "Map Loading Tiles Failed",
            "The map failed to load. Contact support if the error persists. The application will now shut down.");
        return EXIT_FAILURE;
    }
    qDebug() << "Getting PBF link completed without issues.\n";

    tileLoader.pbfLinkTemplate = pbfLinkTemplate.link;

    // Creates the Widget that displays the map.
    auto *mapWidget = new MapWidget;
    mapWidget->requestTilesFn = [&](auto input, auto signal) {
        return tileLoader.requestTiles(input, signal, true);
    };
    auto &styleSheet = mapWidget->styleSheet;

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


    // Main window setup
    auto app = Bach::MainWindow(mapWidget);
    app.show();

    return a.exec();
}
