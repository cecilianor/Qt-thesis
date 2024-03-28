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

ParsedLink loadPngUrlTemplate(
    std::optional<QString> mapTilerKey)
{
    // TODO: First check our disk cache

    // If not found, load it from web.
    // If we don't have a MapTiler key, we can't load it from web.
    if (!mapTilerKey.has_value()) {
        return { {}, ResultType::UnknownError };
    }

    // TODO: Make a switch on the map type to find correct URL for the tile sheet.

    return Bach::getPbfLinkFromTileSheet(
        "https://api.maptiler.com/maps/basic-v2/tiles.json?key=" + mapTilerKey.value());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("qt_thesis_app");

    qDebug() << "Current file cache can be found in: " << TileLoader::getGeneralCacheFolder();

    // Read key from file.
    std::optional<QString> mapTilerKeyResult = Bach::readMapTilerKey("key.txt");
    bool hasMapTilerKey = mapTilerKeyResult.has_value();
    if (!hasMapTilerKey) {
        qWarning() << "Reading of the MapTiler key failed. " <<
                      "App will attempt to only use local cache.";
    }

    // The style sheet type to load (can be many different types).
    auto styleSheetType = MapType::BasicV2;

    std::optional<QJsonDocument> styleSheetJsonResult = Bach::loadStyleSheetJson(
        styleSheetType,
        mapTilerKeyResult);
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
    if (useWeb) {
        auto pbfUrlTemplateResult = Bach::getPbfLinkTemplate(styleSheetJson, "maptiler_planet");
        if (pbfUrlTemplateResult.resultType != ResultType::Success)
            useWeb = false;
        else
            pbfUrlTemplate = pbfUrlTemplateResult.link;
    }
    // Load the tilesheet and grab the PNG url.
    ParsedLink pngUrlTemplateResult = loadPngUrlTemplate(mapTilerKeyResult);

    // Create our TileLoader based on whether we can do web
    // or not.
    std::unique_ptr<TileLoader> tileLoaderPtr;
    if (useWeb) {
        tileLoaderPtr = TileLoader::fromPbfLink(
            pbfUrlTemplate,
            pngUrlTemplateResult.link,
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
