#ifndef GETTILEURL_H
#define GETTILEURL_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>

#include "TileCoord.h"
#include "networkcontroller.h"

class TileURL
{
private:
    QByteArray styleSheet;
    QByteArray JSONTileURL;
    QUrl tileURL;

public:
    /**
     * @brief The styleSheetType enum covers the basic style sheets provided by MapTiler.
     *
     * Some of the style sheet types have _vX at the end. This matches the current map version
     * that the code is implemented against in case the MapTiler APIs are updated
     * in the future with new endpoints that include -v2, -v3 and so on.
     */
    enum class styleSheetType {
        backdrop,
        basic_v2,
        bright_v2,
        dataviz,
        ocean,
        open_street_map,
        outdoor_v2,
        satellite,
        streets_v2,
        toner_v2,
        topo_v2,
        winter_v2,
        unknown,
    };

    // Some hardcoded map source types:
    enum class sourceType {
        maptiler_planet,
        land,
        ocean,
        unknown,
    };

    // Error types
    enum class ErrorCode {
        success = 0,
        mapTilerError = 1,
        parseError = 2,
        unknownError = 3,
    };

    // Constructor and destructor
    TileURL();                                  // Not implemented

    // Functionality making different requests
    std::pair<QByteArray, ErrorCode> getStylesheet(styleSheetType type, QString key); // Implemented
    std::pair<QString, ErrorCode> getTilesLink(const QJsonDocument & styleSheet, QString sourceType);// Implemented Gets dynamic url as a string based on source type!
    std::pair<QString, ErrorCode> getPBFLink (const QString & tileSheetUrl);                          // Implemented. Get PBF link based on dynamic or static url.

    QByteArray loadStyleSheetFromWeb(const QString &mapTilerKey, styleSheetType &styleSheetType);
    QString getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType);
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
    QByteArray downloadTile(const QString &pbfLink, NetworkController &controller);
    // Key reader
    QString readKey(QString tilePath);

    // Set the tile URL
    void setTileURL();                          // Not implemented

    ~TileURL(){};
};


#endif // GETTILEURL_H
