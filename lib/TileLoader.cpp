#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QNetworkReply>
#include <QDir>
#include <QCoreApplication>
#include <QScopeGuard>
#include <QStandardPaths>

#include "TileLoader.h"
#include "TileCoord.h"
#include "Utilities.h"


TileLoader::TileLoader() :
    tileCacheDiskPath { getTileCacheFolder() }
{

}

std::unique_ptr<TileLoader> TileLoader::fromPbfLink(
    const QString &pbfUrlTemplate,
    StyleSheet&& styleSheet)
{
    auto out = std::unique_ptr<TileLoader>(new TileLoader());
    TileLoader &tileLoader = *out;
    tileLoader.styleSheet = std::move(styleSheet);
    tileLoader.pbfLinkTemplate = pbfUrlTemplate;
    // An empty URL is obviously not going to work with
    // web, so we turn off web capabilities.
    if (pbfUrlTemplate == "")
        tileLoader.useWeb = false;
    else
        tileLoader.useWeb = true;
    return out;
}

std::unique_ptr<TileLoader> TileLoader::newLocalOnly(StyleSheet&& styleSheet)
{
    auto out = std::unique_ptr<TileLoader>(new TileLoader());
    TileLoader &tileLoader = *out;
    tileLoader.styleSheet = std::move(styleSheet);
    tileLoader.useWeb = false;
    return out;
}

std::unique_ptr<TileLoader> TileLoader::newDummy(const QString &diskCachePath)
{
    auto out = std::unique_ptr<TileLoader>(new TileLoader());
    TileLoader &tileLoader = *out;
    tileLoader.tileCacheDiskPath = diskCachePath;
    tileLoader.useWeb = false;
    return out;
}

QString TileLoader::getGeneralCacheFolder()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // Uncomment to store cache next to executable.
    //basePath = QCoreApplication::applicationDirPath();

    return QDir::cleanPath(basePath + QDir::separator() + "cached_files");
}

QString TileLoader::getTileCacheFolder()
{
    return getGeneralCacheFolder() + QDir::separator() + "tiles";
}

std::optional<Bach::LoadedTileState> TileLoader::getTileState(TileCoord coord) const
{
    auto tileLock = createTileMemoryLocker();
    auto itemIt = tileMemory.find(coord);
    if (itemIt == tileMemory.end())
        return std::nullopt;
    else {
        const StoredTile &item = itemIt->second;
        return item.state;
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

QString TileLoader::getTileDiskPath(TileCoord coord)
{
    return tileCacheDiskPath + QDir::separator() + Bach::tileDiskCacheSubPath(coord);
}

QScopedPointer<Bach::RequestTilesResult> TileLoader::requestTiles(
    const std::set<TileCoord> &input,
    TileLoadedCallbackFn signalFn,
    bool loadMissingTiles)
{
    // This might not be ideal place to define this struct.
    struct ResultType : public Bach::RequestTilesResult {
        virtual ~ResultType() {
            // TODO:
            // This should eventually notify the TileLoader
            // that our tiles are no longer being read.
            // Only really necessary when we have an eviction policy in place.
        }

        QMap<TileCoord, const VectorTile*> _map;
        const QMap<TileCoord, const VectorTile*> &map() const override
        {
            return _map;
        }

        const StyleSheet* _styleSheet = nullptr;
        const StyleSheet &styleSheet() const override
        {
            Q_ASSERT_X(
                _styleSheet != nullptr,
                "Stylesheet test",
                "Tried to request stylesheet from a dummy TileLoader.");
            return *_styleSheet;
        }
    };
    auto out = new ResultType;
    // Temporary: We just need some way to handle when the user makes
    // a dummy TileLoader with no stylesheet, but tries to request one anyways.
    if (!styleSheet.m_layerStyles.empty())
        out->_styleSheet = &styleSheet;

    // Contains the list of tiles we want to load deferredly.
    QVector<TileCoord> loadJobs;

    // Create scope for the mutex-locker
    {
        QMutexLocker lock = createTileMemoryLocker();
        for (TileCoord requestedCoord : input) {
            // Load iterator to our tile memory.
            auto tileIt = tileMemory.find(requestedCoord);

            // If found, return it immediately.
            // (Maybe mark it as recently used for cache purposes???)
            if (tileIt != tileMemory.end()) {

                // Key found, check if it can be returned immediately.
                const StoredTile &memoryItem = tileIt->second;

                // If the item is marked as nullptr,
                // it means it is pending and should not be immediately returned.
                if (memoryItem.isReadyToRender()) {
                    out->_map.insert(requestedCoord, memoryItem.tile.get());
                }
            } else if (loadMissingTiles) {
                // Tile not found, queue it for loading.
                // Insert it with the pending status.
                tileMemory.insert({ requestedCoord, StoredTile::newPending() });
                loadJobs.push_back(requestedCoord);
            }
        }
    }

    if (loadMissingTiles)
        queueTileLoadingJobs(loadJobs, signalFn);

    return QScopedPointer<Bach::RequestTilesResult>{ out };
}

bool TileLoader::loadFromDisk(TileCoord coord, TileLoadedCallbackFn signalFn)
{
    // Check if it's in disk.
    QString diskPath = getTileDiskPath(coord);

    QFile file { diskPath };
    if (!file.exists()) {
        // Todo: More error feedback?
        return false;
    }

    // TODO: Check that the file isn't currently being written into
    // by another thread by checking for associated .lock file.
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Tried reading tile from file, but encountered unexpected error when opening file.\n";
        // Error, file exists but didn't open.
        return false;
    }

    // Successfully opened file, read contents.
    QByteArray bytes = file.readAll();

    // Now we need to insert the tile into the tile-memory
    // and NOT insert it into disk cache.
    insertIntoTileMemory(coord, bytes, signalFn);

    // Return success if we found the file.
    return true;
}

void TileLoader::writeTileToDisk(TileCoord coord, const QByteArray &bytes) {
    // TODO unused return value of this function.
    Bach::writeTileToDiskCache(tileCacheDiskPath, coord, bytes);
}

void TileLoader::networkReplyHandler(
    QNetworkReply* reply,
    TileCoord coord,
    TileLoadedCallbackFn signalFn)
{
    QScopeGuard cleanup { [&]() { reply->deleteLater(); } };

    // Check for errors in the reply.
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error when requesting tile from web: " << reply->errorString() << '\n';
        // TODO: Do something meaningful, like retrying the request later or
        // marking this tile as non-functional to stop us from requesting it anymore.
    }

    // TODO: Reply can return 204 No Content, and this is a valid result
    // it just means that the tile has no data and doesn't need to be rendered.
    // Only background needs to be rendered.

    // Extract the bytes we want and discard the reply.
    auto bytes = reply->readAll();

    // We are now on the same thread as the QNetworkAccessManager,
    // which means we are on the GUI thread. We need to dispatch the result of
    // the reply onto a new thread. This is because parsing will block
    // GUI responsiveness.

    // Create async jobs to insert tile into memory
    getThreadPool().start([=]() {
        insertIntoTileMemory(coord, bytes, signalFn);
    });

    // Also insert into disk cache.
    getThreadPool().start([=]() {
        writeTileToDisk(coord, bytes);
    });
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
            [=]() { networkReplyHandler(reply, coord, signalFn); } );
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
                if (!loadedFromDiskSuccess && useWeb) {
                    loadFromWeb(coord, signalFn);
                }
            });
        }
    };
    getThreadPool().start(asyncJob);
}

void TileLoader::insertIntoTileMemory(
    TileCoord coord,
    const QByteArray &bytes,
    TileLoadedCallbackFn signalFn)
{
    // Check iterator to see if it's fine to access
    // this tile-memory element.
    auto checkIterator = [&](auto tileIt) {
        if (tileIt == tileMemory.end() || tileIt->second.state != Bach::LoadedTileState::Pending) {
            // Error because tile needs to exist and be pending
            // before we insert it.
            qDebug() <<
                "TileLoader error: Tile " <<
                coord.toString() <<
                " needs to be in pending state before insertion.";
            return false;
        }
        return true;
    };

    // Try parsing the bytes into our tile.
    std::optional<VectorTile> newTileResult = Bach::tileFromByteArray(bytes);

    // If we failed to parse our tile,
    // mark the memory as parsing failed.
    if (!newTileResult.has_value()) {
        qDebug() << "Error when parsing tile " << coord.toString();

        // Insert into the tile memory storage.
        QMutexLocker lock = createTileMemoryLocker();

        auto tileIt = tileMemory.find(coord);
        if (!checkIterator(tileIt)) {
            return;
        } else {
            StoredTile &memoryItem = tileIt->second;
            memoryItem.tile = nullptr;
            memoryItem.state = Bach::LoadedTileState::ParsingFailed;
        }
        emit tileFinished(coord);
        return;
    }

    // Turn our VectorTile into a dedicated allocation that fits our storage.
    auto allocatedTile = std::make_unique<VectorTile>(std::move(newTileResult.value()));
    // Create a scope for our mutex lock.
    {
        QMutexLocker lock = createTileMemoryLocker();
        auto tileIt = tileMemory.find(coord);
        if (!checkIterator(tileIt)) {
            return;
        } else {
            // Mark our tile as OK and insert the Tile data.
            StoredTile &memoryItem = tileIt->second;
            memoryItem.tile = std::move(allocatedTile);
            memoryItem.state = Bach::LoadedTileState::Ok;
        }
    }
    emit tileFinished(coord);

    // Fire the signal.
    // In the case of the application, this
    // is gonna trigger the MapWidget to redraw.
    if (signalFn)
        signalFn(coord);
}

bool Bach::writeTileToDiskCache(
    const QString& basePath,
    TileCoord coord,
    const QByteArray &bytes)
{
    QString fullPath = basePath + QDir::separator() + tileDiskCacheSubPath(coord);
    return Bach::writeNewFileHelper(fullPath, bytes);
}

QString Bach::tileDiskCacheSubPath(TileCoord coord)
{
    QString fileDirPath =
        QString("z%1") +
        QString("x%2") +
        QString("y%3.mvt");
    fileDirPath = fileDirPath
        .arg(coord.zoom)
        .arg(coord.x)
        .arg(coord.y);
    return fileDirPath;
}
