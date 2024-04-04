#ifndef REQUESTTILESRESULT_H
#define REQUESTTILESRESULT_H

#include <QObject>

#include <TileCoord.h>
#include <VectorTiles.h>
#include <Layerstyle.h>

namespace Bach {
    /*!
     *  @brief Acts as the binding point between rendering
     *  and async tile loading.
     *
     *  This is useful because it lets us run custom
     *  cleanup code in the destructor. For us this
     *  means we can mark tiles as no longer being read.
     */
    class RequestTilesResult : public QObject{
        Q_OBJECT
    public:
        virtual ~RequestTilesResult() {}
        // Returns the map of returned tiles.
        virtual const QMap<TileCoord, const VectorTile*> &vectorMap() const = 0;
        virtual const QMap<TileCoord, const QImage*> &rasterImageMap() const = 0;
        virtual const StyleSheet &styleSheet() const = 0;
    };
}



#endif // REQUESTTILESRESULT_H
