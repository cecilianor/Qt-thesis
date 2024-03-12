#ifndef GETTILEURL_H
#define GETTILEURL_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QThreadPool>

#include <set>
#include <functional>

#include "TileCoord.h"
#include "NetworkController.h"
#include "Utilities.h"
#include "VectorTiles.h"
#include "RequestTilesResult.h"

class TileLoader : public QObject
{
    Q_OBJECT
private:
    QByteArray styleSheet;
    QByteArray JSONTileURL;
    QUrl tileURL;
    NetworkController networkController;

    QNetworkAccessManager networkManager;

    /* This contains our memory tile-cache.
     *
     * The value is a pointer to the loaded tile.
     *
     * This can only be accessed when tileMemoryLock is locked.
     *
     * A value of nullptr means that this tile is requested, but is not
     * yet finished loading.
     *
     * This container type needs to be modified if we
     * want to track tiles that have been requested
     * but have failed to load.
     */
    struct StoredTile {
        const VectorTile* tile = nullptr;
    };

    QMap<TileCoord, const VectorTile*> tileMemory;
    // Mutex lock for the tileMemory
    QMutex tileMemoryLock;
public:
    // Needed for loading tiles,
    // probably not the best place to store it?
    QString pbfLinkTemplate;

public:
    // Constructor and destructor
    TileLoader();
    ~TileLoader(){};

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
     * @brief Returns loaded tiles and enqueues loading tiles that are missing.
     *        Cannot block, and cannot be re-entrant as this may get called from
     *        a QWidget paint event.
     * @param requestInput is a set of TileCoords that is requested.
     * @param tileLoadedSignalFn is a function that will get called whenever
     *        a tile is loaded, will be called later in time,
     *        can be called from another thread.
     * @return Returns a RequestTilesResult object containing
     *         the resulting map of tiles. The returned set of
     *         data will always be a subset of all currently loaded tiles.
     */
    QScopedPointer<Bach::RequestTilesResult> requestTiles(
        const std::set<TileCoord> &requestInput,
        TileLoadedCallbackFn tileLoadedSignalFn);

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
    void loadFromWeb(TileCoord coord, TileLoadedCallbackFn signalFn);
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

    /*!
     * @brief Gets the full file-path of a given tile.
     */
    QString getTileDiskPath(TileCoord tileCoord);
}


#endif // GETTILEURL_H
