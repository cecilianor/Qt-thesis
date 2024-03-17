#ifndef GETTILEURL_H
#define GETTILEURL_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QThreadPool>

#include <set>
#include <functional>
#include <memory>
#include <map>

#include "TileCoord.h"
#include "VectorTiles.h"
#include "RequestTilesResult.h"

class UnitTesting;

namespace Bach {
    enum class LoadedTileState {
        Ok,
        Pending,
        ParsingFailed,
        Cancelled,
        UnknownError
    };
}

/*!
 * @brief System for loading, storing and caching map-tiles.
 *
 * Can be used to load tiles for the MapWidget.
 */
class TileLoader : public QObject
{
    Q_OBJECT

private:
    // Use the static creator functions.
    // This constructor alone will not guarantee you a functional
    // TileLoader object.
    TileLoader();
public:
    // We disallow implicit copying.
    TileLoader(const TileLoader&) = delete;
    // Inheriting from QObject makes our class non-movable.
    TileLoader(TileLoader&&) = delete;
    ~TileLoader(){};

    // Disallow copying.
    TileLoader& operator=(const TileLoader&) = delete;
    // Our class is non-movable.
    TileLoader& operator=(TileLoader&&) = delete;

    // Returns the path to the general cache storage for the application.
    static QString getGeneralCacheFolder();
    // Returns the path to the tile cache storage for the application.
    // This guaranteed to be a subfolder of the general cache.
    static QString getTileCacheFolder();


    // We can't return by value below, because TileLoader is a QObject and therefore
    // doesn't support move-semantics.

    /*!
     * @brief Create a new TileLoader instance by utilizing a
     * PBF URL template. The TileLoader will then use this URL
     * to download tiles from the web.
     *
     * @param The URL template for downloading PBF files.
     * The function expects this URL to contain the patterns
     * {x}, {y} and {z} (including curly braces). The TileLoader will
     * then insert the actual Tile-coordinates into the URL.
     *
     * @param The stylesheet to pass on to the MapWidget.
     *
     * @return The constructed TileLoader instance.
     */
    static std::unique_ptr<TileLoader> fromPbfLink(
        const QString &pbfUrlTemplate,
        StyleSheet&& styleSheet);

    /*!
     * @brief Local-only alternative to 'fromPbfLink' function.
     * Creates a TileLoader that can not access the web and will
     * only try to load from cache.
     */
    static std::unique_ptr<TileLoader> newLocalOnly(StyleSheet&& styleSheet);

    /*!
     * @brief Creates an incomplete TileLoader without any stylesheet to pass onto
     * rendering. This TileLoader should never be used when it needs to render anything.
     *
     * This function is mostly used for testing purposes.
     *
     * @param Takes the directory path to read/write cache into.
     */
    static std::unique_ptr<TileLoader> newDummy(const QString &diskCachePath);

    /*!
     * @brief Gets the full file-path of a given tile, whether it exists or not.
     */
    QString getTileDiskPath(TileCoord coord);

    /*!
     * @brief Loads the tile-state of a given tile, if it has been loaded in some form.
     * Mostly used in tests to see if tiles were put into correct state.
     *
     * @threadsafe
     */
    std::optional<Bach::LoadedTileState> getTileState(TileCoord) const;

signals:
    /*!
     * @brief Gets signalled whenever a tile is finished loading,
     * the coordinates of said tile. Will not be signalled if the
     * tile loading process is cancelled.
     */
    void tileFinished(TileCoord);

private:
    StyleSheet styleSheet;

    QString pbfLinkTemplate;

    QNetworkAccessManager networkManager;

    // Controls whether the TileLoader should try to access
    // web when loading.
    bool useWeb = true;

    // Directory path to tile cache storage.
    QString tileCacheDiskPath;

    struct StoredTile {
        // Current loading-state of this tile.
        Bach::LoadedTileState state = {};

        // Stores the final tile data.
        //
        // We use std::unique_ptr over QScopedPointer
        // because QScopedPointer doesn't support move semantics.
        std::unique_ptr<VectorTile> tile;

        // Tells us whether this tile is safe to return to
        // rendering.
        bool isReadyToRender() const
        {
            return state == Bach::LoadedTileState::Ok;
        }

        // Creates a new tile-item with a pending state.
        static StoredTile newPending()
        {
            StoredTile temp;
            temp.state = Bach::LoadedTileState::Pending;
            return temp;
        }
    };
    /* This contains our memory tile-cache.
     *
     * IMPORTANT! Only use when 'tileMemoryLock' is locked!
     *
     * We had to use std::map because QMap doesn't support move semantics,
     * which interferes with our automated resource cleanup.
     */
    std::map<TileCoord, StoredTile> tileMemory;

    // We use unique-ptr here to let use the lock in const methods.
    std::unique_ptr<QMutex> _tileMemoryLock = std::make_unique<QMutex>();

    // Generates the scoped lock for our tile-memory.
    // Will block if mutex is already held.
    QMutexLocker<QMutex> createTileMemoryLocker() const { return QMutexLocker(_tileMemoryLock.get()); }

public:
    // Function signature of the tile-loaded
    // callback passed into 'requestTiles'.
    using TileLoadedCallbackFn = std::function<void(TileCoord)>;

    /*!
     * @brief Grabs loaded tiles, and enqueues loading tiles that are missing
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
     * @param requestInput is a set of TileCoords that is requested.
     *
     * @param tileLoadedSignalFn is a function that will get called whenever
     * a tile is loaded, will be called later in time,
     * can be called from another thread. This function
     * will be called once for each tile that was loaded successfully.
     *
     * This function is only called if a tile is successfully loaded.
     *
     * If this argument is set to null, the missing tiles will not be loaded.
     *
     * @param loadMissingTiles can be set to 'false' to force the
     * TileLoader to NOT load tiles that are requested but not loaded.
     * This means missing tiles will NOT be loaded in the future.
     *
     * @return Returns a RequestTilesResult object containing
     * the resulting map of tiles. The returned set of
     * data will always be a subset of requested tiles and all currently loaded tiles.
     */
    QScopedPointer<Bach::RequestTilesResult> requestTiles(
        const std::set<TileCoord> &requestInput,
        TileLoadedCallbackFn tileLoadedSignalFn,
        bool loadMissingTiles);
    // Overload where we don't need to pass any callback function.
    auto requestTiles(
        const std::set<TileCoord> &requestInput,
        bool loadMissingTiles)
    {
        return requestTiles(requestInput, nullptr, loadMissingTiles);
    }

    // Overload where callback can get passed as nullptr and
    // the TileLoader will not load missing tiles if the
    // callback is nullptr.
    auto requestTiles(
        const std::set<TileCoord> &requestInput,
        TileLoadedCallbackFn tileLoadedSignalFn = nullptr)
    {
        return requestTiles(
            requestInput,
            tileLoadedSignalFn,
            static_cast<bool>(tileLoadedSignalFn));
    }

private:
    /*!
     * @brief
     * Loads the list of tiles into memory, corresponding to the list of
     * TileCoords inputted.
     *
     * This function launches asynchronous jobs, does not block execution!
     *
     * @threadsafe
     */
    void queueTileLoadingJobs(
        const QVector<TileCoord> &input,
        TileLoadedCallbackFn signalFn);


    // Thread-pool for the tile-loader worker threads.
    QThreadPool threadPool;
    // Thread-pool for the tile-loader worker threads.
    QThreadPool &getThreadPool() { return threadPool; }

    /*!
     * @brief Loads the given tile from disk and inserts it
     * into memory. This is a blocking call.
     *
     * @return Returns true if it was unable to load from disk
     */
    bool loadFromDisk(TileCoord coord, TileLoadedCallbackFn signalFn);
    /*!
     * @brief Handles a network reply when a tile has been loaded from web.
     */
    void networkReplyHandler(
        QNetworkReply* reply,
        TileCoord coord,
        TileLoadedCallbackFn signalFn);
    /*!
     * \brief Starts the async process to load a tile from web.
     * This boots an async task, returns immediately.
     * Tile will be loaded later when network request is done.
     * Tile will then be inserted into memory
     * and into disk cache when done.
     */
    void loadFromWeb(TileCoord coord, TileLoadedCallbackFn signalFn);
    /*!
     * \brief Immediately writes a tile to disk cache.
     */
    void writeTileToDisk(TileCoord coord, const QByteArray &bytes);

    /*!
     * \brief Parses byte-array and inserts into tile memory.
     * @threadsafe
     */
    void insertIntoTileMemory(
        TileCoord coord,
        const QByteArray &byteArray,
        TileLoadedCallbackFn signalFn);
};

/*!
 * \namespace Bach
 *
 * \brief This namespace stores custom functionality used for the bachelor thesis
 * project: Setting of links to the external MapTiler API and caching functionalitssy.
 */
namespace Bach {
    QString setPbfLink(TileCoord tileCoord, const QString &pbfLinkTemplate);

/*!
     * \brief writeTileToDiskCache writes tile information to the disk cache.
     * \param basePath refers to the basic/root path to where the cached data is stored.
     * \param coord is the z (zoom), x, and y coordinates of a tile.
     * \param bytes is the tile data passed as a byte array.
     * \return true if caching the tile was successful, false otherwise.
     */
    bool writeTileToDiskCache(
        const QString& basePath,
        TileCoord coord,
        const QByteArray &bytes);

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
    QString tileDiskCacheSubPath(TileCoord coord);
}


#endif // GETTILEURL_H
