#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QNetworkReply>
#include <QDir>
#include <QCoreApplication>

#include "TileLoader.h"
#include "TileCoord.h"
#include "NetworkController.h"
#include "qjsonarray.h"
#include "Utilities.h"

TileLoader::TileLoader() {};

/*!
 * @brief TileURL::getStylesheet gets a stylesheet from MapTiler
 * @param type The type of the stylesheet passed as an enum.
 * @param key The MapTiler key.
 * @return The response from MapTiler.
 */
HttpResponse TileLoader::getStylesheet(StyleSheetType type, QString key) {
    auto url = QString();
    HttpResponse res;
    switch(type) {
        case (StyleSheetType::basic_v2) : {
            url = "https://api.maptiler.com/maps/basic-v2/style.json?key=" + key;
            res = networkController.sendRequest(url);
            if (res.resultType != ResultType::success) {
                qWarning() << "Error: " << PrintResultTypeInfo(res.resultType);
                return { QByteArray(), res.resultType };
            }
            break;
        }
        default: {
            qWarning() << "Error: " <<PrintResultTypeInfo(ResultType::noImplementation);
            return {QByteArray(), ResultType::noImplementation};
            break;
        }
    }
    return res;
};

/*!
 * @brief TileURL::getTilesLink grabs a link to a mapTiler tile sheet
 * @param styleSheet is the stylesheet to get the link from
 * @param sourceType is the map source type passed as a QString
 * @return link as a string
 */
ParsedLink TileLoader::getTilesLink(const QJsonDocument &styleSheet, QString sourceType)
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
                return {link, ResultType::success};
            } else {
                qWarning() << PrintResultTypeInfo(ResultType::tileSheetNotFound);
                return {QString(), ResultType::tileSheetNotFound};
            }
        } else {
            qWarning() << PrintResultTypeInfo(ResultType::unknownSourceType);
            return {QString(), ResultType::unknownSourceType};
        }
    } else {
        qWarning() << "The stylesheet doesn't contain 'sources' field like it should.\n"
                   << "Check if MapTiler API has been updated to store map sources differently.\n";
        return {QString(), ResultType::parseError};
    }

    qWarning() << PrintResultTypeInfo(ResultType::unknownError);
    return {QString(), ResultType::unknownError};
};

/*!
 * @brief TileURL::getPBFLink gets a PBF link based on the url to a tile sheet.
 *
 * The function returns either a success message or an error message and error code.
 *
 * @param tileSheetUrl the link/url to the stylesheet.
 * @return The link to PBF tiles.
 */
ParsedLink TileLoader::getPBFLink (const QString & tileSheetUrl) {
    QJsonDocument tilesSheet;
    QJsonParseError parseError;

    auto res = networkController.sendRequest(tileSheetUrl);
    if (res.resultType != ResultType::success) {
        qWarning() << "Error: " << PrintResultTypeInfo(res.resultType);
        return { QString(), res.resultType };
    }

    // Parse the stylesheet
    tilesSheet = QJsonDocument::fromJson(res.response, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return {QString(), ResultType::parseError};
    }

    if (tilesSheet.isObject()) {
        QJsonObject jsonObject = tilesSheet.object();
        //qDebug() << "A JSON object was found. It is the following: \n" << jsonObject;

        if(jsonObject.contains("tiles") && jsonObject["tiles"].isArray()) {
            QJsonArray tilesArray = jsonObject["tiles"].toArray();

            for (const auto &tileValue : tilesArray) {
                QString tileLink = tileValue.toString();
                //qDebug() << "\n\t" <<"Link to PBF tiles: " << tileLink <<"\n";
                return {tileLink, ResultType::success};
            }
        }
        else {
            qWarning() << "No 'tiles' array was found in the JSON object...";
        }
    }

    //The else if branch is just used for testing. Do NOT pushto final version!!
    else if (tilesSheet.isArray()) {
        qWarning() << "A JSON array was found. The current functionality doesn't support this...";
    }
    else {
        qWarning() << "There is an unknown error with the loaded JSON data...";
    }
    return {QString(), ResultType::unknownError};
};

/*!
 * @brief TileURL::readKey reads a key from file.
 * @param filePath is the relative path + filename that's storing the key.
 * @return The key if successfully read from file.
 */
QString TileLoader::readKey(QString filePath) {

    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Couldn't read key...";
        return QString();
    }
    QTextStream in(&file);

    return in.readAll().trimmed();
}

/*!
 * \brief TileURL::loadStyleSheetFromWeb loads a style sheet from the MapTiler API.
 * \param mapTilerKey is the personal key to the MapTiler API. This must be placed in a file
 * called 'key.txt' and stored in the same folder as the executable.
 * \param StyleSheetType is the type of style sheet to load.
 * \return
 */
HttpResponse TileLoader::loadStyleSheetFromWeb(
    const QString &mapTilerKey,
    StyleSheetType &StyleSheetType)
{
    HttpResponse styleSheetResult = getStylesheet(StyleSheetType, mapTilerKey);
    if (styleSheetResult.resultType != ResultType::success) {
        qWarning() << "There was an error getting the stylesheet: " << PrintResultTypeInfo(styleSheetResult.resultType);
        return {QByteArray(), styleSheetResult.resultType};
    }
    if(styleSheetResult.response.isNull()) {
        qWarning() << "Error: An empty stylesheet was returned: " << PrintResultTypeInfo(styleSheetResult.resultType);
        return{QByteArray(), styleSheetResult.resultType};
    }
    return styleSheetResult;
}

/*!
 * \brief TileURL::getPbfLinkTemplate templatises the PBF link used to get MapTiler vector tiles.
 * \param styleSheetBytes is the style sheet as a byte array.
 * \param sourceType is the source type used by MapTiler. This is currently passed as a string.
 * \return the PBF template.
 */
ParsedLink TileLoader::getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType)
{
     // Assert that the passed styleSheet isn't null.
     if(styleSheetBytes.isNull()) {
         qWarning() << "Empty stylesheet passed to getPbfLinkTemplate...\n";
        return {QString(), ResultType::noData};
    }

    // Parse the stylesheet
    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(styleSheetBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return {QString(), ResultType::parseError};
    }

    // Grab link to tiles.json format link
    ParsedLink tilesLinkResult = getTilesLink(styleSheetJson, sourceType);
    if (tilesLinkResult.resultType != ResultType::success) {
        qWarning() << "";
        return {QString(), tilesLinkResult.resultType};
    }

    // Grab link to the XYZ PBF tile format based on the tiles.json link
    ParsedLink pbfLink = getPBFLink(tilesLinkResult.link);
    return {pbfLink.link, pbfLink.resultType};
}

/*!
 * \brief TileURL::setPbfLink sets the link that will be used to download a protobuf tile.
 *
 * This function takes the templatized version that's used when getting a protobuf tile.
 * It will consist of a link on the form:
 *      'https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=myKey'
 *
 * To get an actual tile, the x, y, and z coordinates must be replaced with the correct
 * coordinates of the tile.
 *
 * \param tileCoord is the coordinate to the PBF tile.
 * \param pbfLinkTemplate is the templatized link.
 * \return The generated link to the requested PBF tile.
 */
QString TileLoader::setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate)
{
    return Bach::setPbfLink(tileCoord, pbfLinkTemplate);
}

/*!
 * \brief TileURL::downloadTile downloads a protobuf tile.
 * \param pbfLink is the URL to make the get request to.
 * \param controller is the network controller making the request.
 * \return the response from the GET request.
 */
HttpResponse TileLoader::downloadTile(const QString &pbfLink)
{
    auto res = networkController.sendRequest(pbfLink);
    if (res.resultType != ResultType::success) {
        qWarning() << "Error: " << PrintResultTypeInfo(res.resultType);
        return { QByteArray(), res.resultType};
    } else {
        return res; // Everything went well, return.
    }
}

/*!
 * \brief Bach::setPbfLink exchanges x, y, z coordinates in a Protobuf link.
 *
 * The function searches for {x}, {y}, and {z} in the link and replaces them
 * with their corresponding coordinate values from the TileCoord struct:
 * - x: x-coordinate
 * - y: y-coordinate
 * - z: zoomlevel
 *
 * \param tileCoord is a struct containing x and y coordinates, z (zoom) information.
 * \param pbfLinkTemplate is the templated URL/link.
 * \return The generated link.
 */
QString Bach::setPbfLink(TileCoord tileCoord, const QString &pbfLinkTemplate)
{
    // Exchange the {x, y z} in link
    auto copy = pbfLinkTemplate;
    copy.replace("{z}", QString::number(tileCoord.zoom));
    copy.replace("{x}", QString::number(tileCoord.x));
    copy.replace("{y}", QString::number(tileCoord.y));
    return copy;
}

QString Bach::getTileDiskPath(TileCoord tile)
{
    QString basePath = QCoreApplication::applicationDirPath();
    auto sep = QDir::separator();
    QString fileDirPath =
        QString("tilecache") + sep +
        QString("z%1") + sep +
        QString("x%2");
    fileDirPath = fileDirPath
        .arg(tile.zoom)
        .arg(tile.x);

    QString filename = QString("y%1.mvt").arg(tile.y);

    QString finalPath =
        basePath + sep +
        fileDirPath + sep +
        filename;
    finalPath = QDir::cleanPath(finalPath);
    return finalPath;
}

QScopedPointer<Bach::RequestTilesResult> TileLoader::requestTiles(
    const std::set<TileCoord> &input,
    TileLoadedCallbackFn signalFn)
{
    // This might not be ideal place to define this struct.
    struct ResultType : public Bach::RequestTilesResult {
        virtual ~ResultType() {
            // TODO:
            // This should eventually notify the TileLoader
            // that our tiles are no longer being read.
        }

        QMap<TileCoord, const VectorTile*> _map;

        const QMap<TileCoord, const VectorTile*> &map() const override
        {
            return _map;
        }
    };
    auto out = new ResultType;

    // Contains the list of tiles we want to load deferredly.
    QVector<TileCoord> loadJobs;

    // Create scope for the mutex-locker
    {
        QMutexLocker lock(&tileMemoryLock);
        for (TileCoord requestedCoord : input) {
            // Load iterator to our tile memory.
            auto tileIt = tileMemory.find(requestedCoord);

            // If found, return it immediately.
            // (Maybe mark it as recently used for cache purposes???)
            if (tileIt != tileMemory.end()) {

                // Key found, check if it can be returned immediately.
                const VectorTile* memoryItem = *tileIt;

                // If the item is marked as nullptr,
                // it means it is pending and should not be immediately returned.
                if (memoryItem != nullptr) {
                    out->_map.insert(requestedCoord, memoryItem);
                }
            } else {
                // Tile not found, queue it for loading.
                // Insert it with the pending status.
                tileMemory.insert(requestedCoord, nullptr);
                loadJobs.push_back(requestedCoord);
            }
        }
    }

    queueTileLoadingJobs(loadJobs, signalFn);

    return QScopedPointer<Bach::RequestTilesResult>{ out };
}

bool TileLoader::loadFromDisk(TileCoord coord, TileLoadedCallbackFn signalFn)
{
    // Check if it's in disk.
    QString diskPath = Bach::getTileDiskPath(coord);

    QFile file { diskPath };

    if (file.exists()) {

        // TODO: Check that the file isn't currently being written into
        // by another thread by checking for associated .lock file.
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "Tried reading tile from file, but encountered unexpected error when opening file.\n";
            // Error, file exists but didn't open.
        }

        // Successfully opened file, read contents.
        QByteArray bytes = file.readAll(); 

        // Now we need to insert the tile into the tile-memory
        // and NOT insert it into disk cache.
        insertTile(coord, bytes, signalFn);

        // Return success if we found the file.
        return true;
    }

    return false;
}

void writeToDisk(TileCoord coord, const QByteArray &bytes) {
    QFileInfo diskPath { Bach::getTileDiskPath(coord) };

    // QFile won't create our directories for us.
    // We gotta make them ourselves.

    QDir dir = diskPath.dir();
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        qDebug() << "Tried writing tile to disk. Creating parent directory failed.\n";
        return;
    }

    QFile file { diskPath.absoluteFilePath() };
    if (file.exists()) {
        qDebug() << "Tried writing tile to disk. File already exists.\n";
        return;
    }

    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "Tried writing tile to disk. Failed to create new file.\n";
        return;
    }

    // Todo: make a lock file
    file.write(bytes);
}

void TileLoader::loadFromWeb(TileCoord coord, TileLoadedCallbackFn signalFn)
{
    // Load the URL for this particular tile.
    QString tileUrl = Bach::setPbfLink(coord, pbfLinkTemplate);

    // We expect this function to be called on the background thread, but our
    // QNetworkAccessManager lives on the main thread.
    // We can't make requests from this thread. Queue a request to it.

    auto job = [=]() {
        // We don't want to use NetworkController here, because it
        // forces us to wait. And since we are on main GUI thread,
        // that would block GUI logic until we get a reply.

        QNetworkReply *reply = networkManager.get(QNetworkRequest{ tileUrl });
        QObject::connect(
            reply,
            &QNetworkReply::finished,
            this,
            [=]() {
                // Check for errors in the reply.
                if (reply->error() != QNetworkReply::NoError) {
                    qDebug() << "Error when requesting tile from web: " << reply->errorString() << '\n';
                    // TODO: Do something meaningful, like retrying the request or
                    // marking this tile as non-functional to stop us from requesting it anymore.
                }

                // TODO: Reply can return 204 No Content, and this is a valid result
                // it just means that the tile has no data and doesn't need to be rendered.

                // Extract the bytes we want and discard the reply.
                auto bytes = reply->readAll();
                reply->deleteLater();

                // We are now on the same thread as the QNetworkAccessManager,
                // which means we are on the GUI thread. We need to dispatch the result of
                // the reply onto a new thread. This is because parsing will block
                // GUI responsiveness.
                queueTileParsing(coord, bytes, signalFn);
            });
    };
    QMetaObject::invokeMethod(
        &networkManager,
        job);
}

// This function should not block!
// Offload all work to background thread(s)!
void TileLoader::queueTileLoadingJobs(
    const QVector<TileCoord> &input,
    TileLoadedCallbackFn signalFn)
{
    // We can assume all input tiles do not exist in memory.

    // We queue up one task to launch
    // the smaller tasks to return as early as possible.
    auto asyncJob = [=]() {
        for (TileCoord coord : input) {

            // Then we spawn one async task per item.
            getThreadPool().start([=]() {

                // First we try loading from disk. If found, the disk function will handle
                // the rest of this async process.
                // If not found, start the process to download from web.
                bool loadedFromDiskSuccess = loadFromDisk(coord, signalFn);
                if (!loadedFromDiskSuccess) {
                    loadFromWeb(coord, signalFn);
                }

            });

        }
    };
    getThreadPool().start(asyncJob);
}

void TileLoader::queueTileParsing(
    TileCoord coord,
    QByteArray bytes,
    TileLoadedCallbackFn signalFn)
{
    getThreadPool().start([=]() {
        insertTile(coord, bytes, signalFn);
    });

    getThreadPool().start([=]() {
        writeToDisk(coord, bytes);
    });
}

void TileLoader::insertTile(
    TileCoord coord,
    const QByteArray &bytes,
    TileLoadedCallbackFn signalFn)
{
    // Try parsing the bytes into our tile.
    std::optional<VectorTile> newTileResult = Bach::tileFromByteArray(bytes);

    // TODO: Could have better error handling here.
    if (!newTileResult.has_value()) {
        qDebug() << "Error when parsing tile.\n";
    }

    // Turn our VectorTile into a dedicated allocation that fits our storage.
    VectorTile* newTile = new VectorTile(newTileResult.value());

    {
        // Insert into the common storage.
        QMutexLocker lock(&tileMemoryLock);
        tileMemory.insert(coord, newTile);
    }

    // Fire the signal.
    if (signalFn)
        signalFn(coord);
}
