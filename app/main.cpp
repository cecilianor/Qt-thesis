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

    // Read key from file.
    std::optional<QString> mapTilerKeyResult = Bach::readMapTilerKey("key.txt");
    bool hasMapTilerKey = mapTilerKeyResult.has_value();
    if (!hasMapTilerKey) {
        // Add developer comments to QDebug, not to the end user/client.
        qWarning() << "Reading of the MapTiler key failed. " <<
                      "App will attempt to only use local cache.";
    }

    // The style sheet type to load (can be many different types).
    auto styleSheetType = StyleSheetType::basic_v2;

    // Load stylesheet raw data from disk/web.
    HttpResponse styleSheetBytes = Bach::loadStyleSheetBytes(
        styleSheetType,
        mapTilerKeyResult);
    // If we couldn't load the style sheet, we can't display anything.
    // So we quit.
    if (styleSheetBytes.resultType != ResultType::success) {
        earlyShutdown("Unable to load stylesheet from disk/web.");
    }

    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(
        styleSheetBytes.response,
        &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        earlyShutdown("Unable to parse stylesheet raw data into JSON.");
    }

    // Parse the stylesheet into data we can render.
    std::optional<StyleSheet> parsedStyleSheetResult = StyleSheet::fromJson(styleSheetJson);
    if (!parsedStyleSheetResult.has_value()) {
        qWarning() << "Unable to parse stylesheet JSON into a parsed StyleSheet object.";
        earlyShutdown();
    }
    StyleSheet &styleSheet = parsedStyleSheetResult.value();

    // First we load useful links from the stylesheet. We only care about this is if we
    // are online (have a MapTiler key)

    // We only want to get the pbfLink if we can connect to internet.

    // Tracks whether we will be using web stuff.
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
    mapWidget->requestTilesFn = [&](auto input, auto signal) {
        return tileLoader.requestTiles(input, signal, true);
    };

    // Main window setup
    auto app = Bach::MainWindow(mapWidget);
    app.show();

    return a.exec();
}
