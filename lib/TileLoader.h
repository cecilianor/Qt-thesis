#ifndef GETTILEURL_H
#define GETTILEURL_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>

#include "TileCoord.h"
#include "NetworkController.h"
#include "Utilities.h"

class TileLoader
{
private:
    QByteArray styleSheet;
    QByteArray JSONTileURL;
    QUrl tileURL;
    NetworkController networkController;

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

    // Caching
    QString cacheForStyleSheet() const;
    QString cacheForVectorTile(int z, int x, int y) const;
};

namespace Bach {
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
}


#endif // GETTILEURL_H
