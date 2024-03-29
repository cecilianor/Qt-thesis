#include <QApplication>
#include <QMessageBox>

#include "MainWindow.h"
#include "TileLoader.h"
#include "Utilities.h"

// Helper function to let us do early shutdown during startup.
[[noreturn]] void earlyShutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    QMessageBox::critical(
        nullptr,
        "Unexpected error.",
        "Application will now quit.");
    std::exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("qt_thesis_app");

    qDebug() << "Current file cache can be found in: " << TileLoader::getGeneralCacheFolder();

    // Read key from file.
    std::optional<QString> mapTilerKeyOpt = Bach::readMapTilerKey("key.txt");
    bool hasMapTilerKey = mapTilerKeyOpt.has_value();
    if (!hasMapTilerKey) {
        qWarning() << "Reading of the MapTiler key failed. " <<
                      "App will attempt to only use local cache.";
    }

    // The style sheet type to load (can be many different types).
    auto mapType = MapType::BasicV2;

    std::optional<QJsonDocument> styleSheetJsonResult = Bach::loadStyleSheetJson(
        mapType,
        mapTilerKeyOpt);
    if (!styleSheetJsonResult.has_value()) {
        earlyShutdown("Unable to load stylesheet from disk/web.");
    }
    const QJsonDocument &styleSheetJson = styleSheetJsonResult.value();

    // Parse the stylesheet into data we can render.
    std::optional<StyleSheet> parsedStyleSheetResult = StyleSheet::fromJson(styleSheetJson);
    // If the stylesheet can't be parsed, there is nothing to render. Shut down.
    if (!parsedStyleSheetResult.has_value()) {
        earlyShutdown("Unable to parse stylesheet JSON into a parsed StyleSheet object.");
    }
    StyleSheet &styleSheet = parsedStyleSheetResult.value();

    // Load useful links from the stylesheet.
    // This only matters if one is online and has a MapTiler key.

    // Tracks whether or not to download data from web.
    bool useWeb = hasMapTilerKey;
    QString pbfUrlTemplate;
    QString pngUrlTemplate;
    if (useWeb) {
        ParsedLink pbfUrlTemplateResult = Bach::getPbfUrlTemplate(styleSheetJson, "maptiler_planet");
        ParsedLink pngUrlTemplateResult = Bach::getPngUrlTemplate(mapType, mapTilerKeyOpt);

        if (pbfUrlTemplateResult.resultType != ResultType::Success)
            useWeb = false;
        else if (pngUrlTemplateResult.resultType != ResultType::Success)
            useWeb = false;
        else {
            pbfUrlTemplate = pbfUrlTemplateResult.link;
            pngUrlTemplate = pngUrlTemplateResult.link;
        }
    }

    // Create our TileLoader based on whether we can do web
    // or not.
    std::unique_ptr<TileLoader> tileLoaderPtr;
    if (useWeb) {
        tileLoaderPtr = TileLoader::fromTileUrlTemplate(
            pbfUrlTemplate,
            pngUrlTemplate,
            std::move(styleSheet));
    } else {
        tileLoaderPtr = TileLoader::newLocalOnly(std::move(styleSheet));
    }
    TileLoader &tileLoader = *tileLoaderPtr;

    // Creates the Widget that displays the map.
    auto *mapWidget = new MapWidget;
    // Set up the function that forwards requests from the
    // MapWidget into the TileLoader. This lambda does the
    // two components together.
    mapWidget->requestTilesFn = [&](auto tileList, auto tileLoadedCallback) {
        return tileLoader.requestTiles(tileList, tileLoadedCallback, true);
    };

    // Main window setup
    auto app = Bach::MainWindow(mapWidget);
    app.show();

    return a.exec();
}
