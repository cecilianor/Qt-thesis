#ifndef GETTILEURL_H
#define GETTILEURL_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>

#include "TileCoord.h"
#include "NetworkController.h"

class TileLoader
{
private:
    QByteArray styleSheet;
    QByteArray JSONTileURL;
    QUrl tileURL;

public:
    /**
     * @brief The StyleSheetType enum covers the basic style sheets provided by MapTiler.
     *
     * Some of the style sheet types have _vX at the end. This matches the current map version
     * that the code is implemented against in case the MapTiler APIs are updated
     * in the future with new endpoints that include -v2, -v3 and so on.
     */
    enum class StyleSheetType {
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
    enum class SourceType {
        maptiler_planet,
        land,
        ocean,
        unknown,
    };

    // Result types
    enum class ResultType {
        success,
        mapTilerError,
        parseError,
        unknownError,
    };

    // Converts a result type to a string.
    const QString ResultTypeToString(ResultType r) {
        QString str;

        switch (r)
        {
        case TileLoader::ResultType::success:
            str = "Success";
        case TileLoader::ResultType::mapTilerError:
            str = "Maptiler error";
        case TileLoader::ResultType::parseError:
            str = "Parsing error";
        case TileLoader::ResultType::unknownError:
            str = "Unknown error";
        default:
            str = "Unimplemented error. Check documentation.";
        }

        return str;
    }


    // Http response type
    struct HttpResponse  {
        QByteArray response;
        TileLoader::ResultType resultType;
    };

    // Url after parsing HTTP data
    struct ParsedLink {
        QString link;
        TileLoader::ResultType resultType;
    };

    // Constructor and destructor
    TileLoader();                                  // Not implemented

    // Functionality making different requests
    TileLoader::HttpResponse getStylesheet(StyleSheetType type, QString key, NetworkController &networkController); // Implemented
    std::pair<QString, ResultType> getTilesLink(const QJsonDocument & styleSheet, QString sourceType);// Implemented Gets dynamic url as a string based on source type!
    std::pair<QString, ResultType> getPBFLink (const QString & tileSheetUrl, NetworkController &networkController);                          // Implemented. Get PBF link based on dynamic or static url.

    QByteArray loadStyleSheetFromWeb(const QString &mapTilerKey, StyleSheetType &StyleSheetType, NetworkController &networkController);
    QString getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType, NetworkController &networkController);
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
    QByteArray downloadTile(const QString &pbfLink, NetworkController &controller);
    // Key reader
    QString readKey(QString tilePath);

    // Set the tile URL
    void setTileURL();                          // Not implemented

    ~TileLoader(){};
};

namespace Bach {
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
}


#endif // GETTILEURL_H
