#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#include "TileLoader.h"
#include "TileCoord.h"
#include "networkcontroller.h"
#include "qjsonarray.h"

TileLoader::TileLoader() {};

/*!
 * @brief TileURL::getStylesheet gets a stylesheet from MapTiler
 * @param type The type of the stylesheet passed as an enum.
 * @param key The MapTiler key.
 * @return The response from MapTiler.
 */
std::pair<QByteArray, TileLoader::ErrorCode> TileLoader::getStylesheet(styleSheetType type, QString key) {
    NetworkController networkController;
    QByteArray response="";
    QString url;

    switch(type) {
    case (TileLoader::styleSheetType::basic_v2) : {
        url = "https://api.maptiler.com/maps/basic-v2/style.json?key=" + key;
        response = networkController.sendRequest(QString(url));
        break;
        }
    default: {
        qDebug() << "No implementation of stylesheet type!";
        return std::make_pair(response, ErrorCode::unknownError);
        break;
        }
    }
    return std::make_pair(response, ErrorCode::success);
};

/*!
 * @brief TileURL::getTilesLink grabs a link to a mapTiler tile sheet
 * @param styleSheet is the stylesheet to get the link from
 * @param sourceType is the map source type passed as a QString
 * @return link as a string
 */
std::pair<QString, TileLoader::ErrorCode> TileLoader::getTilesLink(const QJsonDocument & styleSheet, QString sourceType)
{
    //Pair <QString, errorCode>
    //  Make an errorCode enum
    //
    // Map <errorCode, errorMessage>
    //
    // std::expected and std::optional

    // Grab link to tile sheet
    if (styleSheet.isObject()) {
        QJsonObject jsonObject = styleSheet.object();
        //qDebug() << "A JSON object was found!"; // It is the following: \n" << jsonObject;

        if(jsonObject.contains("sources") && jsonObject["sources"].isObject()) {
            //qDebug() << jsonObject["sources"];

            QJsonObject sourcesObject = jsonObject["sources"].toObject();

            if (sourcesObject.contains(sourceType) && sourcesObject[sourceType].isObject()) {
                //qDebug() << "Found maptiler_planet";

                QJsonObject maptilerObject = sourcesObject[sourceType].toObject();
                //qDebug() << maptilerObject;

                if(maptilerObject.contains("url")) {
                    QString link = maptilerObject["url"].toString();
                    //qDebug() << "Link to tiles API: " << link;
                    return std::make_pair(link, ErrorCode::success);
                }
                else {
                    qDebug() << "The URL to the tilesheet couldn't be found...";
                }
            }
        }
        else {
            qDebug() << "No 'maptiler_planet' object was found in the JSON object...";
        }
    } else  qDebug() << "The original stylesheet isn't a JSON object...";

    return std::make_pair("", ErrorCode::unknownError);
};

/*!
 * @brief TileURL::getPBFLink gets a PBF link based on the url to a tile sheet.
 *
 * The function returns either a success message or an error message and error code.
 *
 * @param tileSheetUrl the link/url to the stylesheet.
 * @return The link to PBF tiles.
 */
std::pair<QString, TileLoader::ErrorCode> TileLoader::getPBFLink (const QString & tileSheetUrl) {
    NetworkController networkController;
    QJsonDocument tilesSheet;
    QJsonParseError parseError;

    auto data = networkController.sendRequest(tileSheetUrl);

    // Parse the stylesheet
    tilesSheet = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return std::make_pair("Parse error", ErrorCode::parseError);
    }

    if (tilesSheet.isObject()) {
        QJsonObject jsonObject = tilesSheet.object();
        //qDebug() << "A JSON object was found. It is the following: \n" << jsonObject;

        if(jsonObject.contains("tiles") && jsonObject["tiles"].isArray()) {
            QJsonArray tilesArray = jsonObject["tiles"].toArray();

            for (const auto &tileValue : tilesArray) {
                QString tileLink = tileValue.toString();
                //qDebug() << "\n\t" <<"Link to PBF tiles: " << tileLink <<"\n";
                return std::make_pair(tileLink, ErrorCode::success);
            }
        }
        else {
            qWarning() << "No 'tiles' array was found in the JSON object...";
        }
    }

    //The else if branch is just used for testing. Do NOT pushto final version!!
    else if (tilesSheet.isArray()) {
        QJsonArray jsonArray = tilesSheet.array();
        qDebug() << "A JSON array was found. The current functionality doesn't support this...";

    }
    else {
        qWarning() << "There is an error with the loaded JSON data...";
    }
    return std::make_pair("Link wasn't found...", ErrorCode::unknownError);
};

/*!
 * @brief TileURL::readKey reads a key from file.
 * @param filePath is the relative path + filename that's storing the key.
 * @return The key if successfully read from file.
 */
QString TileLoader::readKey(QString filePath) {

    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Couldn't read key...";
        return QString();
    }
    QTextStream in(&file);

    return in.readAll().trimmed();
}
/*!
 * \brief TileURL::loadStyleSheetFromWeb loads a style sheet from the MapTiler API.
 * \param mapTilerKey is the personal key to the MapTiler API. This must be placed in a file
 * called 'key.txt' and stored in the same folder as the executable.
 * \param styleSheetType is the type of style sheet to load.
 * \return
 */
QByteArray TileLoader::loadStyleSheetFromWeb(const QString &mapTilerKey, TileLoader::styleSheetType &styleSheetType)
{

    std::pair<QByteArray, TileLoader::ErrorCode> styleSheetResult = getStylesheet(styleSheetType, mapTilerKey);
    if (styleSheetResult.second != TileLoader::ErrorCode::success) {
        qWarning() << "There was an error: " << styleSheetResult.first;
    }
    return styleSheetResult.first;
}

/*!
 * \brief TileURL::getPbfLinkTemplate templatises the PBF link used to get MapTiler vector tiles.
 * \param styleSheetBytes is the style sheet as a byte array.
 * \param sourceType is the source type used by MapTiler. This is currently passed as a string.
 * \return the PBF template.
 */
QString TileLoader::getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType)
{
    // Parse the stylesheet
    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(styleSheetBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return 0;
    }

    // Grab link to tiles.json format link
    std::pair<QString, TileLoader::ErrorCode> tilesLinkResult = getTilesLink(styleSheetJson, sourceType);

    // Grab link to the XYZ PBF tile format based on the tiles.json link
    std::pair<QString, TileLoader::ErrorCode> pbfLink = getPBFLink(tilesLinkResult.first);
    return pbfLink.first;
}

/*!
 * \brief TileURL::setPbfLink sets the link that will be used to download a protobuf tile.
 *
 * This function takes the templatized version that's used when getting a protobuf tile.
 * It will consist of a link on the form:
 *      'https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=myKey'
 *
 * To get an actual tile, the x, y, and z coordinates must be replaced with the correct
 * coordinates of the tile.
 *
 * \param tileCoord is the coordinate to the PBF tile.
 * \param pbfLinkTemplate is the templatized link.
 * \return The generated link to the requested PBF tile.
 */
QString TileLoader::setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate)
{
    // Exchange the {x, y z} in link
    auto copy = pbfLinkTemplate;
    copy.replace("{z}", QString::number(tileCoord.zoom));
    copy.replace("{x}", QString::number(tileCoord.x));
    copy.replace("{y}", QString::number(tileCoord.y));
    return copy;
}

/*!
 * \brief TileURL::downloadTile downloads a protobuf tile.
 * \param pbfLink is the URL to make the get request to.
 * \param controller is the network controller making the request.
 * \return the response from the GET request.
 */
QByteArray TileLoader::downloadTile(const QString &pbfLink, NetworkController &controller)
{
    return controller.sendRequest(pbfLink);
}
