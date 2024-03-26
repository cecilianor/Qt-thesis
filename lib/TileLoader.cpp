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

// This might not be ideal place to define this struct.
struct TileResultType : public Bach::RequestTilesResult {
    virtual ~TileResultType() {
        // TODO:
        // This should eventually notify the TileLoader
        // that our tiles are no longer being read.
        // Only really necessary when we have an eviction policy in place.
    }

    // Generate the map holding tile coordinates and a vector tile.
    QMap<TileCoord, const VectorTile*> _vectorMap;
    const QMap<TileCoord, const VectorTile*> &vectorMap() const override
    {
        return _vectorMap;
    }

    // Generate the map holding tile coordinates and a raster tile.
    QMap<TileCoord, const QImage*> _rasterMap;
    const QMap<TileCoord, const QImage*> &rasterImageMap() const override
    {
        return _rasterMap;
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

TileLoader::TileLoader() :
    tileCacheDiskPath { getTileCacheFolder() }
{

}

/*!
 * \brief Create a new TileLoader instance by utilizing a
 * PBF URL template. The TileLoader will then use this URL
 * to download tiles from the web.
 *
 * \param The URL template for downloading PBF files.
 * The function expects this URL to contain the patterns
 * {x}, {y} and {z} (including curly braces). The TileLoader will
 * then insert the actual Tile-coordinates into the URL.
 *
 * \param The stylesheet to pass on to the MapWidget.
 *
 * \return The constructed TileLoader instance.
 */
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

/*!
 * \brief
 * Local-only alternative to 'fromPbfLink' function.
 * Creates a TileLoader that can not access the web and will
 * only try to load from cache.
 */
std::unique_ptr<TileLoader> TileLoader::newLocalOnly(StyleSheet&& styleSheet)
{
    auto out = std::unique_ptr<TileLoader>(new TileLoader());
    TileLoader &tileLoader = *out;
    tileLoader.styleSheet = std::move(styleSheet);
    tileLoader.useWeb = false;
    return out;
}

/*!
 * \brief Creates an incomplete TileLoader without any stylesheet to pass onto
 * rendering. This TileLoader should never be used when it needs to render anything.
 *
 * This function is mostly used for testing purposes.
 *
 * \param Takes the directory path to read/write cache into.
 */
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

/*!
 * \brief Loads the tile-state of a given tile, if it has been loaded in some form.
 * Mostly used in tests to see if tiles were put into correct state.
 *
 * \threadsafe
 */
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

/*!
 * \brief Gets the full file-path of a given tile, whether it exists or not.
 */
QString TileLoader::getTileDiskPath(TileCoord coord, TileType tileType)
{
    return tileCacheDiskPath + QDir::separator() + Bach::tileDiskCacheSubPath(coord, tileType);
}

/*!
 * \brief Grabs loaded tiles, and enqueues loading tiles that are missing
 * onto bakground thread(s).
 *
 * This function does not block and is not re-entrant as
 * this may get called from a QWidget paint event.
 *
 * Returns the set of tiles that are already in memory at the
 * time of the request. To grab new tiles as they are loaded,
 * use the tileLoadedSignalFn parameter and call this function again.
 * Alternatively connect to the tileFinished-signal.
 *
 * \param requestInput is a set of TileCoords that is requested.
 *
 * \param tileLoadedSignalFn is a function that will get called whenever
 * a vectorTile is loaded, will be called later in time,
 * can be called from another thread. This function
 * will be called once for each vectorTile that was loaded successfully.
 *
 * This function is only called if a vectorTile is successfully loaded.
 *
 * If this argument is set to null, the missing tiles will not be loaded.
 *
 * \param loadMissingTiles can be set to 'false' to force the
 * TileLoader to NOT load tiles that are requested but not loaded.
 * This means missing tiles will NOT be loaded in the future.
 *
 * \return Returns a RequestTilesResult object containing
 * the resulting map of tiles. The returned set of
 * data will always be a subset of requested tiles and all currently loaded tiles.
 */
QScopedPointer<Bach::RequestTilesResult> TileLoader::requestTiles(
    const std::set<TileCoord> &input,
    TileLoadedCallbackFn signalFn,
    bool loadMissingTiles)
{
    auto out = new TileResultType;
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
                    out->_vectorMap.insert(requestedCoord, memoryItem.vectorTile.get());
                    out->_rasterMap.insert(requestedCoord, &memoryItem.rasterTile);
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

/*!
 * \brief Loads the given tile from disk and inserts it
 * into memory. This is a blocking call.
 *
 * \return Returns true if it was unable to load from disk
 */
bool TileLoader::loadFromDisk(TileCoord coord, TileLoadedCallbackFn signalFn)
{
    // Check if the tile in disk.
    //
    // MAJOR CHANGE HERE. Cecilia has passed in the vector tile type here.
    // Consider calling this function twice with different tile types or handling just
    // one type at a time. This is just passed this way to allow the code to run when Nils
    // pulls it on his end later on March 26th.)
    QString diskPath = getTileDiskPath(coord, TileType::Vector);

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


/*!
 * \brief TileLoader::writeTileToDisk writes a tile to disk cache.
 * \param coord is the ZXY coordinate of the tile to write to disk.
 * \param bytes is vector tile information stored as a QByteArray.
 */
void TileLoader::writeTileToDisk(TileCoord coord, const QByteArray &bytes) {
    // TODO unused return value of this function.
    Bach::writeTileToDiskCache(tileCacheDiskPath, coord, bytes);
}

/*!
 * \brief TileLoader::networkReplyHandler handles a network reply when a tile is loaded from web.
 * \param reply is the network reply.
 * \param coord is the ZXY tile coordinate.
 * \param signalFn is a callback function that signals when to insert jobs into memory.
 */
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

/*!
 * \brief Starts the async process to load a tile from web.
 * This boots an async task, returns immediately.
 * Tile will be loaded later when network request is done.
 * Tile will then be inserted into memory
 * and into disk cache when done.
 */
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

/*!
 * \brief
 * Loads the list of tiles into memory, corresponding to the list of
 * TileCoords inputted.
 *
 * This function launches asynchronous jobs, does not block execution!
 *
 * \threadsafe
 */
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

/*!
 * \brief Parses byte-array and inserts into vectorTile memory.
 * \threadsafe
 */
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
            qWarning() <<
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
        qCritical() << "Error when parsing tile " << coord.toString();

        // Insert into the tile memory storage.
        QMutexLocker lock = createTileMemoryLocker();

        auto tileIt = tileMemory.find(coord);
        if (!checkIterator(tileIt)) {
            return;
        } else {
            StoredTile &memoryItem = tileIt->second;
            memoryItem.vectorTile = nullptr;
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
            memoryItem.vectorTile = std::move(allocatedTile);
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

/*!
 * \brief writeTileToDiskCache writes tile information to the disk cache.
 *
 * This is a useful helper function that allows us to write tests
 * that preload a tile-cache for a TileLoader to read. It's used by
 * TileLoader internally, and from the unit tests.
 *
 * \param basePath refers to the basic/root path to where the cached data is stored.
 * \param coord is the z (zoom), x, and y coordinates of a tile.
 * \param bytes is the tile data passed as a byte array.
 * \return true if caching the tile was successful, false otherwise.
 */
bool Bach::writeTileToDiskCache(
    const QString& basePath,
    TileCoord coord,
    const QByteArray &bytes)
{
    QString fullPath = basePath + QDir::separator() + tileDiskCacheSubPath(coord, TileType::Vector);
    return Bach::writeNewFileHelper(fullPath, bytes);
}

/*!
 * \brief tileDiskCacheSubPath finds the file-path subpath for a cache folder.
 *
 * An example of one of these paths is "z0/x0/y0.mvt"
 *
 * \param coord is a z, x, y coordinate, where all values are integers.
 * These values represent the XYZ tile coordinate.
 *
 * z is always in the range [0, 16] and represents zoom level.
 * x and y must be in the range [0, tilecount-1], where tilecount = 2^zoom.
 * \return the subpath that was found.
 */
QString Bach::tileDiskCacheSubPath(TileCoord coord, TileType tileType)
{
    QString fileDirPath = QString("z%1x%2y%3")
        .arg(coord.zoom)
        .arg(coord.x)
        .arg(coord.y);

    // Add tile correct file extension to the path name.
    // We treat all raster images as png images for now.
    // Consider a refactor later if necessary when loading other file types.
    switch(tileType) {
        case TileType::Vector:
        {
            fileDirPath += ".mvt";
            break;
        }
        case TileType::Raster:
        {
            fileDirPath += ".png";
            break;
        }
    }
    return fileDirPath;
}
