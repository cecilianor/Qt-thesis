#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

#include "MainWindow.h"
#include "TileLoader.h"
#include "Utilities.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
        qWarning() << "Reading of the MapTiler key failed...";
        return EXIT_FAILURE;
    }

    HttpResponse styleSheetBytes;

    // Caching variables.
    QString styleSheetCacheFormat = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) +
                                    QDir::separator() + "styleSheetCache.json";
    QString styleSheetCacheUrl =styleSheetCacheFormat;

    // Try to load the style sheet from file. Download it from MapTiler if it's not foind.
    QFile file(styleSheetCacheUrl);
    if (file.exists())
    {
        qDebug() << "Loading stylesheet from cache. Reading from file...\n";
        // Open the file
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "There was a problem opening the cache file to write to it.";
            return EXIT_FAILURE;
        }

        // Read the cache file contents and convert it to a byteArray().
        QTextStream in(&file);
        qDebug() << "Finished loading from the cache file. Converting data to string...\n";
        auto myString = in.readAll();
        file.close();

        qDebug() << "Converting stylesheet to byteArray...\n";
        auto data = myString.toUtf8();

        if (!data.isValidUtf8()){
            qWarning()<<"Error when converting cached stylesheet to byteArray... Exiting.\n";
            return EXIT_FAILURE;
        }

        // Potential bugs:
        // What if the cache file got garbled at some step before here? There could potentially be
        // more errors here. Note that the stylesheet is only written to the cached file
        // in the first place if the original HTTP request had not-empty data on the expected form.
        styleSheetBytes = HttpResponse{data, ResultType::success}; // Kinda strange signature here, but it must be this way to match its original implementation.
    } else {
        // Load stylesheet from web
        styleSheetBytes = tileLoader.loadStyleSheetFromWeb(mapTilerKey, StyleSheetType);
        qDebug() << "Loading stylesheet from MapTiler...\n";
        if (styleSheetBytes.resultType != ResultType::success) {
            qWarning() << "There was an error loading stylesheet: " << PrintResultTypeInfo(styleSheetBytes.resultType);
            QMessageBox::critical(
                nullptr,
                "Map Loading Bytesheet Failed",
                "The map failed to load. Contact support if the error persists. The application will now shut down.");
            return EXIT_FAILURE;
        }
        qDebug() << "Loading stylesheet from Maptiler completed without issues.\n";

        // Write the response data to the cache.
        // Consider additional error handling here in the future.
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "There was a problem opening the cache file to write to it.";
            return EXIT_FAILURE;
        }
        file.write(styleSheetBytes.response);
        file.close();
    }

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
        auto tileLink = tileLoader.downloadTile(Bach::setPbfLink(tile, pbfLinkTemplate.link));

        if (tileLink.resultType !=ResultType::success) {
            qWarning() << "Error when downloading tile in downloadTile lambda: "
                       << PrintResultTypeInfo(tileLink.resultType) << '\n';
            std::abort();
        } else {
            auto result = Bach::tileFromByteArray(
                tileLoader.downloadTile(
                    Bach::setPbfLink(tile, pbfLinkTemplate.link)).response);
            if (!result.has_value()) {
                std::abort();
                qWarning() << "Error: No data was generated by Bach::tileFromByteArray.\n";
            }
            return result.value();
        }
    };

    // Download tiles, or load them from disk, then insert into the tile-storage map.
    auto tile000 = downloadTile({0, 0, 0});
    tileStorage.insert({0, 0, 0}, &tile000);

    // Main window setup
    auto app = Bach::MainWindow(mapWidget, downloadTile);
    app.show();

    return a.exec();
}
