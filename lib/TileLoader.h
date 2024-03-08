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

public:
    // Constructor and destructor
    TileLoader();
    ~TileLoader(){};

    // Functionality making different requests
    HttpResponse getStylesheet(StyleSheetType type, QString key, NetworkController &networkController);
    ParsedLink getTilesLink(const QJsonDocument & styleSheet, QString sourceType);
    ParsedLink getPBFLink (const QString & tileSheetUrl, NetworkController &networkController);

    HttpResponse loadStyleSheetFromWeb(const QString &mapTilerKey, StyleSheetType &StyleSheetType, NetworkController &networkController);
    ParsedLink getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType, NetworkController &networkController);
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
    HttpResponse downloadTile(const QString &pbfLink, NetworkController &controller);

    // Key reader
    QString readKey(QString tilePath);
};

namespace Bach {
    QString setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate);
}


#endif // GETTILEURL_H
