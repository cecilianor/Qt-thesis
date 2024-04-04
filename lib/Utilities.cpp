#include <QtEnvironmentVariables>
#include <QFile>
#include <QtNetwork>
#include <QTextStream>

#include "TileLoader.h"
#include "Utilities.h"

/*!
 * \brief Bach::rasterTilesheetUrlFromMapType gets a raster tile sheet based on the chosen map type.
 * \param mapType is the mapType to get the correspoding raster tile URL for.
 * \return the raster tile sheet URL that corresponds to the chosen map type.
 */
std::optional<QString> Bach::rasterTilesheetUrlFromMapType(MapType mapType)
{
    switch (mapType) {
        case MapType::BasicV2:
            return "https://api.maptiler.com/maps/basic-v2/tiles.json?key=";
        case MapType::BrightV2:
            return "https://api.maptiler.com/maps/bright-v2/tiles.json?key=";
        case MapType::StreetsV2:
            return "https://api.maptiler.com/maps/streets-v2/tiles.json?key=";
        case MapType::Satellite:
            return "https://api.maptiler.com/maps/satellite/tiles.json?key=";
        default:
            return std::nullopt;
    };
}

/*!
 * \brief Helper function to write a byte array to disk.
 * This function will automatically establish any necessary
 * parent directories, unlike the standard QFile::open.
 *
 * This function will only succeed if it was able to establish the path
 * This function will only succeed if the file did not already exist.
 *
 * Created directory will not be removed in the case that writing to file fails.
 *
 * \param path is the path to the file. Must contain filename, cannot be directory only.
 * \param Takes the byte-array to write.
 * \return true if success, false if failed.
 */
bool Bach::writeNewFileHelper(const QString& path, const QByteArray &bytes)
{
    auto fileInfo = QFileInfo{ path };
    // Grab the directory part of this full filepath.
    QDir dir = fileInfo.dir();

    // QFile won't create our directories for us.
    // We gotta make them ourselves.
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        qCritical() << "writeNewFileHelper tried writing to file. Creating parent directory failed.\n";
        return false;
    }

    QFile file { fileInfo.absoluteFilePath() };
    if (file.exists()) {
        qCritical() << " writeNewFileHelper tried writing to file. File already exists.\n";
        return false;
    }

    /* Possible improvement:
     * It's conceivable that a thread might try to read a tile-file
     * that is currently being written to by another thread.
     *
     * To solve this, we might want to introduce a .lock file
     * solution whose presence determines whether a file is currently
     * in use.
     */

    if (!file.open(QFile::WriteOnly)) {
        qCritical() << "writeNewFileHelper tried writing to file. Failed to create new file.\n";
        return false;
    }

    if (file.write(bytes) != bytes.length()) {
        qCritical() << " writeNewFileHelper tried writing to file. Couldn't write the correct amount of bytes.\n";
        return false;
    }

    return true;
}

/*!
 * \brief Reads MapTiler key from file.
 * \param filePath is the relative path + filename that's storing the key.
 * \return The key if successfully read from file.
 */
std::optional<QString> Bach::readMapTilerKey(const QString &filePath)
{
    // First check if the key is available as an environment variable.
    {
        QString envValue = qEnvironmentVariable(mapTilerKeyEnvName.toUtf8());
        if (!envValue.isEmpty()) {
            return envValue;
        }
    }


    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        //qDebug() << "Couldn't read key...";
        return std::nullopt;
    }
    QTextStream in(&file);

    return in.readAll().trimmed();
}

/*!
 * \brief Helper function to make a network request.
 * This function makes a simple GET request to the URL supplied.
 *
 * It waits for the response before
 * returning the result.
 *
 * Should only be used during startup of the program, preferably.
 *
 * This is a re-entrant function.
 * \return Returns the response error code and the byte-array.
 */
HttpResponse Bach::requestAndWait(const QString &url)
{
    QNetworkAccessManager manager;

    // Perform the GET request
    QNetworkReply *reply = manager.get(QNetworkRequest{ QUrl{url} });

    // Creates an event loop to wait for the request to finish.
    QEventLoop loop;
    QObject::connect(
        reply,
        &QNetworkReply::finished,
        &loop,
        &QEventLoop::quit);
    loop.exec();

    // Checks for errors.
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error in file NetworkController.cpp: " << reply->errorString() << '\n';
        return { QByteArray(), ResultType::NetworkError };
    }

    // Processes the response.
    QByteArray responseData = reply->readAll();
    if (responseData.length() == 0) {
        qWarning() << "No data was returned from the external source";
        return { QByteArray(), ResultType::NoData };
    }

    // Deletes the reply.
    reply->deleteLater();
    // Returns response data if everything was successful.
    return { responseData, ResultType::Success };
}

/*!
 * \brief Makes a blocking network request to get a stylesheet from MapTiler
 * \param type The style of the stylesheet
 * \param key The MapTiler key.
 * \return The byte array response from MapTiler.
 */
HttpResponse Bach::requestStyleSheetFromWeb(MapType type, const QString &key)
{
    switch(type) {
        case (MapType::BasicV2) : {
            QString url = "https://api.maptiler.com/maps/basic-v2/style.json?key=" + key;
            return Bach::requestAndWait(url);
        }
        default: {
            qWarning() << "Error: " <<PrintResultTypeInfo(ResultType::NoImplementation);
            return {QByteArray(), ResultType::NoImplementation};
        }
    }
}

/*!
 * \brief Loads the bytes of the stylesheet.
 * Will attempt to load from cache first, then try downloading from web.
 * If loaded from web, it will then try to write the result to disk cache.
 * This is a blocking and re-entrant function.
 */
HttpResponse Bach::loadStyleSheetBytes(
    MapType type,
    const std::optional<QString> &mapTilerKey)
{
    // Create full path for the target stylesheet JSON file
    QString styleSheetCachePath =
        TileLoader::getGeneralCacheFolder() + QDir::separator() +
        "styleSheetCache.json";

    // Try to load the style sheet from file first.
    {
        // We create scope for the QFile so that it doesn't interfere when we possibly write
        // to file later.
        QFile file { styleSheetCachePath };
        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Loading stylesheet from cache. Reading from file...\n";

            // Potential bugs:
            // What if the cache file got garbled at some step before here? There could potentially be
            // more errors here. Note that the stylesheet is only written to the cached file
            // in the first place if the original HTTP request had not-empty data on the expected form.
            return { file.readAll(), ResultType::Success }; // Kinda strange signature here, but it must be this way to match its original implementation.
        }
    }

    // If we got here, we need to try loading from web.

    // If we don't have any MapTiler key, we can't make a request.
    if (!mapTilerKey.has_value())
        return { {}, ResultType::UnknownError };

    // Make and wait for the web request.
    HttpResponse webResponse = Bach::requestStyleSheetFromWeb(type, mapTilerKey.value());
    if (webResponse.resultType != ResultType::Success)
        return webResponse;

    // From here we want try to write this stylesheet to disk cache.
    // If it failed, we consider the entire function a failure.
    bool writeSuccess = writeNewFileHelper(styleSheetCachePath, webResponse.response);
    if (!writeSuccess)
        return { {}, ResultType::UnknownError };

    return webResponse;
}

std::optional<QJsonDocument> Bach::loadStyleSheetJson(
    MapType type,
    const std::optional<QString> &mapTilerKey)
{
    // Load stylesheet raw data from disk/web.
    HttpResponse styleSheetBytes = loadStyleSheetBytes(
        type,
        mapTilerKey);
    // If loading the style sheet failed, there is nothing to display. Shut down.
    if (styleSheetBytes.resultType != ResultType::Success) {
        return std::nullopt;
    }

    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(
        styleSheetBytes.response,
        &parseError);
    // No stylesheet, so we shut down.
    if (parseError.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    return styleSheetJson;
}

std::optional<QJsonDocument> Bach::loadVectorTileSheet(
    const QJsonDocument &styleSheet,
    const QString &sourceType)
{
    const QString cachePath = QDir::cleanPath(
        TileLoader::getGeneralCacheFolder() +
        QDir::separator() +
        "vectorTileSheet.json");

    // Firste check if the tilesheet exists in cache?
    {
        QFile file{ cachePath };
        if (file.exists()) {
            // If the file exists, we read it and parse it.
            if (!file.open(QFile::ReadOnly)) {
                qWarning() << "Error: Found cached tilesheet but unable to open it.";
                return std::nullopt;
            }

            QByteArray tileSheetBytes = file.readAll();
            // Parse it in to JSON and return it.
            QJsonParseError parseError;
            QJsonDocument tileSheetJson = QJsonDocument::fromJson(tileSheetBytes, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qWarning() << "Error: Found cached tileshhet but unable to parse into JSON";
                return std::nullopt;
            }
            return tileSheetJson;
        }
    }

    // We didn't find it in cache, we need to download it.

    // First we need to find the URL to the tiles document.
    ParsedLink tilesUrlResult = getTilesLinkFromStyleSheet(styleSheet, sourceType);
    if (tilesUrlResult.resultType != ResultType::Success) {
        qWarning() << "Unable to grab tilesheet URL from stylesheet.";
        return std::nullopt;
    }

    HttpResponse response = Bach::requestAndWait(tilesUrlResult.link);
    if (response.resultType != ResultType::Success) {
        qWarning() << "Error when performing network request for tilesheet.";
        return std::nullopt;
    }

    // Parse into JSON.
    // Parse it in to JSON and return it.
    QJsonParseError parseError;
    QJsonDocument tileSheetJson = QJsonDocument::fromJson(response.response, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error: Got valid network reply for tilesheet but unable to parse into JSON";
        return std::nullopt;
    }

    // Then save it to file.
    bool writeDiskSuccess = Bach::writeNewFileHelper(cachePath, response.response);
    if (!writeDiskSuccess) {
        qWarning() << "Error when writing tilesheet to disk cache";
        return std::nullopt;
    }
    return tileSheetJson;
}

std::optional<QJsonDocument> Bach::loadRasterTileSheet(
    MapType mapType,
    std::optional<QString> mapTilerKeyOpt)
{
    // Check if this tilesheet exists in disk.
    const QString cachePath = QDir::cleanPath(
        TileLoader::getGeneralCacheFolder() +
        QDir::separator() +
        "rasterTileSheet.json");

    // Firste check if the tilesheet exists in cache?
    {
        QFile file{ cachePath };
        if (file.exists()) {
            // If the file exists, we read it and parse it.
            if (!file.open(QFile::ReadOnly)) {
                qWarning() << "Error: Found cached tilesheet but unable to open it.";
                return std::nullopt;
            }

            QByteArray tileSheetBytes = file.readAll();
            // Parse it in to JSON and return it.
            QJsonParseError parseError;
            QJsonDocument tileSheetJson = QJsonDocument::fromJson(tileSheetBytes, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qWarning() << "Error: Found cached tileshhet but unable to parse into JSON";
                return std::nullopt;
            }
            return tileSheetJson;
        }
    }

    if (!mapTilerKeyOpt.has_value()) {
        // This is not an error. This means we can't use web
        // because we have no MapTiler key.
        return std::nullopt;
    }

    QString mapTilerKey = mapTilerKeyOpt.value();

    // TODO: In the future, this needs a switch case to choose the correct
    // URL based on the maptype. For now it's hardcoded to basic-v2.
    std::optional<QString> tileSheetUrlOpt = rasterTilesheetUrlFromMapType(mapType);
    if (!tileSheetUrlOpt.has_value()) {
        // Couldn't find the corresponding Url for this maptype.
        return std::nullopt;
    }
    QString tileSheetUrl = tileSheetUrlOpt.value();

    // Insert the key
    tileSheetUrl += mapTilerKey;

    HttpResponse response = Bach::requestAndWait(tileSheetUrl);
    if (response.resultType != ResultType::Success) {
        qWarning() << "Error when performing network request for tilesheet.";
        return std::nullopt;
    }

    // Parse into JSON.
    // Parse it in to JSON and return it.
    QJsonParseError parseError;
    QJsonDocument tileSheetJson = QJsonDocument::fromJson(response.response, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error: Got valid network reply for tilesheet but unable to parse into JSON";
        return std::nullopt;
    }

    // Then save it to file.
    bool writeDiskSuccess = Bach::writeNewFileHelper(cachePath, response.response);
    if (!writeDiskSuccess) {
        qWarning() << "Error when writing tilesheet to disk cache";
        return std::nullopt;
    }
    return tileSheetJson;
}

/*!
 * \brief getPbfLinkTemplate
 * Finds the PBF link template from the stylesheet JSON.
 *
 * \param styleSheet
 * \param sourceType
 * \return Returns the string that can be turned into the URL.
 * This string will have patterns {z}/{x}/{y} where the tile indices
 * need to be inserted.
 */
ParsedLink Bach::getPbfUrlTemplate(
    const QJsonDocument &styleSheet,
    const QString &sourceType)
{
    std::optional<QJsonDocument> tileSheetJsonOpt = Bach::loadVectorTileSheet(
        styleSheet,
        sourceType);
    if (!tileSheetJsonOpt.has_value())
        return { {}, ResultType::UnknownError };

    QJsonDocument &tileSheetJson = tileSheetJsonOpt.value();
    return Bach::getTileUrlTemplateFromTileSheet(tileSheetJson);
}
/*
ParsedLink Bach::getPngTileLinkTemplate(
    const QJsonDocument &tileSheet,
    const QString &sourceType)
{
    ParsedLink PngTilesUrlResult = getTilesLinkFromTileSheet(tileSheet);
}
*/

/*!
 * @brief Finds the to a mapTiler tilesheet given a stylesheet
 * @param styleSheet is the stylesheet to get the link from
 * @param sourceType is the map source type passed as a QString
 * @return link as a string
 */
ParsedLink Bach::getTilesLinkFromStyleSheet(
    const QJsonDocument &styleSheet,
    const QString &sourceType)
{
    // Convert stylesheet to a Json Object.
    QJsonObject jsonObject = styleSheet.object();

    if (jsonObject.contains("sources") && jsonObject["sources"].isObject()) {
        QJsonObject sourcesObject = jsonObject["sources"].toObject();

        if (sourcesObject.contains(sourceType) && sourcesObject[sourceType].isObject()) {
            QJsonObject maptilerObject = sourcesObject[sourceType].toObject();

            // Return the tile sheet if the url to it was found.
            if (maptilerObject.contains("url")) {
                QString link = maptilerObject["url"].toString();
                return {link, ResultType::Success};
            } else {
                qWarning() << PrintResultTypeInfo(ResultType::TileSheetNotFound);
                return {QString(), ResultType::TileSheetNotFound};
            }
        } else {
            qWarning() << PrintResultTypeInfo(ResultType::UnknownSourceType);
            return {QString(), ResultType::UnknownSourceType};
        }
    } else {
        qWarning() << "The stylesheet doesn't contain 'sources' field like it should.\n"
                   << "Check if MapTiler API has been updated to store map sources differently.\n";
        return {QString(), ResultType::ParseError};
    }

    qWarning() << PrintResultTypeInfo(ResultType::UnknownError);
    return {QString(), ResultType::UnknownError};
}

/*!
 * \brief Gets a PBF link based on the url to a tile sheet.
 *
 * The function returns either a success message or an error message and error code.
 *
 * \param tileSheetUrl the link/url to the stylesheet.
 * \return The link to PBF tiles.
 */
ParsedLink Bach::getTileUrlTemplateFromTileSheet(const QJsonDocument &tileSheet)
{
    if (tileSheet.isObject()) {
        QJsonObject jsonObject = tileSheet.object();
        if(jsonObject.contains("tiles") && jsonObject["tiles"].isArray()) {
            QJsonArray tilesArray = jsonObject["tiles"].toArray();
            for (const QJsonValueRef &tileValue : tilesArray) {
                QString tileLink = tileValue.toString();
                //qDebug() << "\n\t" <<"Link to PBF tiles: " << tileLink <<"\n";
                return { tileLink, ResultType::Success };
            }
        }
        else {
            qWarning() << "No 'tiles' array was found in the JSON object...";
        }
    } else if (tileSheet.isArray()) {
        //The else if branch is just used for testing. Do NOT pushto final version!!
        qWarning() << "A JSON array was found. The current functionality doesn't support this...";
    }
    else {
        qWarning() << "There is an unknown error with the loaded JSON data...";
    }
    return { QString(), ResultType::UnknownError };
}

ParsedLink Bach::getPngUrlTemplate(
    MapType mapType,
    std::optional<QString> mapTilerKey)
{
    std::optional<QJsonDocument> tileSheetOpt = Bach::loadRasterTileSheet(mapType, mapTilerKey);
    if (!tileSheetOpt.has_value()) {
        return { {}, ResultType::UnknownError };
    }

    QJsonDocument &tileSheet = tileSheetOpt.value();
    return Bach::getTileUrlTemplateFromTileSheet(tileSheet);
}
