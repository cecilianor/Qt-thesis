#ifndef GETTILEURL_H
#define GETTILEURL_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>

#include <set>
#include <functional>
#include <memory>
#include <map>

#include "TileCoord.h"
#include "NetworkController.h"
#include "Utilities.h"
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

class TileLoader : public QObject
{
    Q_OBJECT

public:
    TileLoader();
    TileLoader(QString tileCacheDiskPath, bool useWeb);
    ~TileLoader(){};

    static QString getGeneralCacheFolder();
    static QString getTileCacheFolder();

    /*!
     * @brief Gets the full file-path of a given tile.
     */
    QString getTileDiskPath(TileCoord coord);

    // Thread safe.
    std::optional<Bach::LoadedTileState> getTileState(TileCoord) const;

signals:
    void tileFinished(TileCoord);

private:
    QByteArray styleSheet;
    QByteArray JSONTileURL;
    QUrl tileURL;
    NetworkController networkController;

    QNetworkAccessManager networkManager;
    bool useWeb = true;
    QString tileCacheDiskPath;

    struct StoredTile {
        Bach::LoadedTileState state = {};
        std::unique_ptr<VectorTile> tile;

        // Tells us whether this tile is safe to return to
        // rendering.
        bool isOk() const { return state == Bach::LoadedTileState::Ok; }

        static StoredTile makePending()
        {
            StoredTile temp;
            temp.state = Bach::LoadedTileState::Pending;
            return temp;
        };
    };
    /* This contains our memory tile-cache.
     *
     * The value is a pointer to the loaded tile.
     *
     * IMPORTANT! Only use when 'tileMemoryLock' is locked!
     *
     * We had to use std::map because QMap doesn't support move semantics,
     * which interferes with our automated resource cleanup.
     */
    std::map<TileCoord, StoredTile> tileMemory;
    // We use unique-ptr here to let use the lock in const methods.
    std::unique_ptr<QMutex> _tileMemoryLock = std::make_unique<QMutex>();
    QMutexLocker<QMutex> createTileMemoryLocker() const { return QMutexLocker(_tileMemoryLock.get()); }

public:


    // Needed for loading tiles,
    // probably not the best place to store it?
    QString pbfLinkTemplate;

public:
    // Functionality making different requests
    HttpResponse getStylesheet(StyleSheetType type, QString key);
    ParsedLink getTilesLink(const QJsonDocument & styleSheet, QString sourceType);
    ParsedLink getPBFLink (const QString & tileSheetUrl);

    HttpResponse loadStyleSheetFromWeb(const QString &mapTilerKey, StyleSheetType &StyleSheetType);
    ParsedLink getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType);
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
    HttpResponse downloadTile(const QString &pbfLink);

    // Key reader
    QString readKey(QString tilePath);

public:
    using TileLoadedCallbackFn = std::function<void(TileCoord)>;

    /*!
     * @brief Grabs loaded tiles and enqueues loading tiles that are missing.
     *        This function does not block and is not re-entrant as
     *        this may get called from a QWidget paint event.
     *
     *        Returns the set of tiles that are already in memory at the
     *        time of the request. To grab new tiles as they are loaded,
     *        use the tileLoadedSignalFn parameter and call this function again.
     *
     * @param requestInput is a set of TileCoords that is requested.
     *
     * @param tileLoadedSignalFn is a function that will get called whenever
     *        a tile is loaded, will be called later in time,
     *        can be called from another thread. This function
     *        will be called once for each tile that was loaded successfully.
     *
     *        This function is only called if a tile is successfully loaded.
     *
     *        If this argument is set to null, the missing tiles will not be loaded.
     *
     * @return Returns a RequestTilesResult object containing
     *         the resulting map of tiles. The returned set of
     *         data will always be a subset of all currently loaded tiles.
     */
    QScopedPointer<Bach::RequestTilesResult> requestTiles(
        const std::set<TileCoord> &requestInput,
        TileLoadedCallbackFn tileLoadedSignalFn,
        bool loadMissingTiles);

    auto requestTiles(
        const std::set<TileCoord> &requestInput,
        bool loadMissingTiles)
    {
        return requestTiles(requestInput, nullptr, loadMissingTiles);
    }

    auto requestTiles(
        const std::set<TileCoord> &requestInput,
        TileLoadedCallbackFn tileLoadedSignalFn = nullptr)
    {
        return requestTiles(requestInput, tileLoadedSignalFn, static_cast<bool>(tileLoadedSignalFn));
    }

private:
    /*!
     * @brief
     * Loads the list of tiles into memory, corresponding to the list of
     * TileCoords inputted.
     *
     * This function launches asynchronous jobs, does not block execution!
     */
    void queueTileLoadingJobs(
        const QVector<TileCoord> &input,
        TileLoadedCallbackFn signalFn);

    QThreadPool &getThreadPool() const { return *QThreadPool::globalInstance(); }
    bool loadFromDisk(TileCoord coord, TileLoadedCallbackFn signalFn);
    void networkReplyHandler(
        QNetworkReply* reply,
        TileCoord coord,
        TileLoadedCallbackFn signalFn);
    void loadFromWeb(TileCoord coord, TileLoadedCallbackFn signalFn);
    void writeTileToDisk(TileCoord coord, const QByteArray &bytes);
    void queueTileParsing(
        TileCoord coord,
        QByteArray byteArray,
        TileLoadedCallbackFn signalFn);
    void insertTile(
        TileCoord coord,
        const QByteArray &byteArray,
        TileLoadedCallbackFn signalFn);

public:

    // Caching
    QString cacheForStyleSheet() const;
    QString cacheForVectorTile(int z, int x, int y) const;
};

namespace Bach {
    QString setPbfLink(TileCoord tileCoord, const QString &pbfLinkTemplate);

    bool writeTileToDiskCache(
        const QString& basePath,
        TileCoord coord,
        const QByteArray &bytes);
    QString tileDiskCacheSubPath(TileCoord coord);
}


#endif // GETTILEURL_H
