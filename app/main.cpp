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
        qWarning() << msg;
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
    std::optional<QString> mapTilerKeyResult = Bach::readMapTilerKey("key.txt");
    bool hasMapTilerKey = mapTilerKeyResult.has_value();
    if (!hasMapTilerKey) {
        qWarning() << "Reading of the MapTiler key failed. " <<
                      "App will attempt to only use local cache.";
    }

    // The style sheet type to load (can be many different types).
    auto styleSheetType = StyleSheetType::basic_v2;

    // Load stylesheet raw data from disk/web.
    HttpResponse styleSheetBytes = Bach::loadStyleSheetBytes(
        styleSheetType,
        mapTilerKeyResult);
    // If loading the style sheet failed, there is nothing to display. Shut down.
    if (styleSheetBytes.resultType != ResultType::success) {
        qCritical() << "Unable to load stylesheet from disk/web.";
        earlyShutdown("Unable to load stylesheet from disk/web.");
    }

    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(
        styleSheetBytes.response,
        &parseError);
    // No stylesheet, so we shut down.
    if (parseError.error != QJsonParseError::NoError) {
        qCritical() << "Unable to parse stylesheet data into JSON.";
        earlyShutdown("Unable to parse stylesheet raw data into JSON.");
    }

    // Parse the stylesheet into data we can render.
    std::optional<StyleSheet> parsedStyleSheetResult = StyleSheet::fromJson(styleSheetJson);
    // If the stylesheet can't be parsed, there is nothing to render. Shut down.
    if (!parsedStyleSheetResult.has_value()) {
        qCritical() << "Unable to parse stylesheet JSON into a parsed StyleSheet object.";
        earlyShutdown();
    }
    StyleSheet &styleSheet = parsedStyleSheetResult.value();


    // Load useful links from the stylesheet.
    // This only matters if one is online and has a MapTiler key.

    // Tracks whether or not to download data from web.
    bool useWeb = hasMapTilerKey;
    QString pbfUrlTemplate;
    if (useWeb) {
        auto pbfUrlTemplateResult = Bach::getPbfLinkTemplate(styleSheetJson, "maptiler_planet");
        if (pbfUrlTemplateResult.resultType != ResultType::success)
            useWeb = false;
        else
            pbfUrlTemplate = pbfUrlTemplateResult.link;
    }

    // Create our TileLoader based on whether we can do web
    // or not.
    std::unique_ptr<TileLoader> tileLoaderPtr;
    if (useWeb) {
        tileLoaderPtr = TileLoader::fromPbfLink(
            pbfUrlTemplate,
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
    mapWidget->requestTilesFn = [&](auto input, auto signal) {
        return tileLoader.requestTiles(input, signal, true);
    };

    // Main window setup
    auto app = Bach::MainWindow(mapWidget);
    app.show();

    return a.exec();
}
