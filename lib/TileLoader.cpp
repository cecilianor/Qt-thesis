#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#include "TileLoader.h"
#include "TileCoord.h"
#include "NetworkController.h"
#include "qjsonarray.h"
#include "Utilities.h"

TileLoader::TileLoader() {};

/*!
 * @brief TileURL::getStylesheet gets a stylesheet from MapTiler
 * @param type The type of the stylesheet passed as an enum.
 * @param key The MapTiler key.
 * @return The response from MapTiler.
 */
HttpResponse TileLoader::getStylesheet(StyleSheetType type, QString key) {
    auto url = QString();
    HttpResponse res;
    switch(type) {
        case (StyleSheetType::basic_v2) : {
            url = "https://api.maptiler.com/maps/basic-v2/style.json?key=" + key;
            res = networkController.sendRequest(url);
            if (res.resultType != ResultType::success) {
                qWarning() << "Error: " << PrintResultTypeInfo(res.resultType);
                return { QByteArray(), res.resultType };
            }
            break;
        }
        default: {
            qWarning() << "Error: " <<PrintResultTypeInfo(ResultType::noImplementation);
            return {QByteArray(), ResultType::noImplementation};
            break;
        }
    }
    return res;
};

/*!
 * @brief TileURL::getTilesLink grabs a link to a mapTiler tile sheet
 * @param styleSheet is the stylesheet to get the link from
 * @param sourceType is the map source type passed as a QString
 * @return link as a string
 */
ParsedLink TileLoader::getTilesLink(const QJsonDocument &styleSheet, QString sourceType)
{
    // Grab link to tile sheet
    if (styleSheet.isObject()) {
        QJsonObject jsonObject = styleSheet.object();

        if (jsonObject.contains("sources") && jsonObject["sources"].isObject()) {
            QJsonObject sourcesObject = jsonObject["sources"].toObject();

            if (sourcesObject.contains(sourceType) && sourcesObject[sourceType].isObject()) {
                QJsonObject maptilerObject = sourcesObject[sourceType].toObject();

                // Return the tile sheet if the url to it was found.
                if (maptilerObject.contains("url")) {
                    QString link = maptilerObject["url"].toString();
                    return {link, ResultType::success};
                } else {
                    qWarning() << PrintResultTypeInfo(ResultType::tileSheetNotFound);
                    return {QString(), ResultType::tileSheetNotFound};
                }
            } else {
                qWarning() << PrintResultTypeInfo(ResultType::unknownSourceType);
                return {QString(), ResultType::unknownSourceType};
            }
        }
    }

    qWarning() << PrintResultTypeInfo(ResultType::unknownError);
    return {QString(), ResultType::unknownError};
};

/*!
 * @brief TileURL::getPBFLink gets a PBF link based on the url to a tile sheet.
 *
 * The function returns either a success message or an error message and error code.
 *
 * @param tileSheetUrl the link/url to the stylesheet.
 * @return The link to PBF tiles.
 */
ParsedLink TileLoader::getPBFLink (const QString & tileSheetUrl) {
    QJsonDocument tilesSheet;
    QJsonParseError parseError;

    auto res = networkController.sendRequest(tileSheetUrl);
    if (res.resultType != ResultType::success) {
        qWarning() << "Error: " << PrintResultTypeInfo(res.resultType);
        return { QString(), res.resultType };
    }

    // Parse the stylesheet
    tilesSheet = QJsonDocument::fromJson(res.response, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return {QString(), ResultType::parseError};
    }

    if (tilesSheet.isObject()) {
        QJsonObject jsonObject = tilesSheet.object();
        //qDebug() << "A JSON object was found. It is the following: \n" << jsonObject;

        if(jsonObject.contains("tiles") && jsonObject["tiles"].isArray()) {
            QJsonArray tilesArray = jsonObject["tiles"].toArray();

            for (const auto &tileValue : tilesArray) {
                QString tileLink = tileValue.toString();
                //qDebug() << "\n\t" <<"Link to PBF tiles: " << tileLink <<"\n";
                return {tileLink, ResultType::success};
            }
        }
        else {
            qWarning() << "No 'tiles' array was found in the JSON object...";
        }
    }

    //The else if branch is just used for testing. Do NOT pushto final version!!
    else if (tilesSheet.isArray()) {
        qWarning() << "A JSON array was found. The current functionality doesn't support this...";
    }
    else {
        qWarning() << "There is an unknown error with the loaded JSON data...";
    }
    return {QString(), ResultType::unknownError};
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
 * \param StyleSheetType is the type of style sheet to load.
 * \return
 */
HttpResponse TileLoader::loadStyleSheetFromWeb(
    const QString &mapTilerKey,
    StyleSheetType &StyleSheetType
    )
{
    HttpResponse styleSheetResult = getStylesheet(StyleSheetType, mapTilerKey);
    if (styleSheetResult.resultType != ResultType::success) {
        qWarning() << "There was an error getting the stylesheet: " << PrintResultTypeInfo(styleSheetResult.resultType);
        return {QByteArray(), styleSheetResult.resultType};
    }
    if(styleSheetResult.response.isNull()) {
        qWarning() << "Error: An empty stylesheet was returned: " << PrintResultTypeInfo(styleSheetResult.resultType);
        return{QByteArray(), styleSheetResult.resultType};
    }
    return styleSheetResult;
}

/*!
 * \brief TileURL::getPbfLinkTemplate templatises the PBF link used to get MapTiler vector tiles.
 * \param styleSheetBytes is the style sheet as a byte array.
 * \param sourceType is the source type used by MapTiler. This is currently passed as a string.
 * \return the PBF template.
 */
ParsedLink TileLoader::getPbfLinkTemplate(const QByteArray &styleSheetBytes, const QString sourceType)
{
     // Assert that the passed styleSheet isn't null.
     if(styleSheetBytes.isNull()) {
         qWarning() << "Empty stylesheet passed to getPbfLinkTemplate...\n";
        return {QString(), ResultType::noData};
    }

    // Parse the stylesheet
    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(styleSheetBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return {QString(), ResultType::parseError};
    }

    // Grab link to tiles.json format link
    ParsedLink tilesLinkResult = getTilesLink(styleSheetJson, sourceType);
    if (tilesLinkResult.resultType != ResultType::success) {
        qWarning() << "";
        return {QString(), tilesLinkResult.resultType};
    }

    // Grab link to the XYZ PBF tile format based on the tiles.json link
    ParsedLink pbfLink = getPBFLink(tilesLinkResult.link);
    return {pbfLink.link, pbfLink.resultType};
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
    return Bach::setPbfLink(tileCoord, pbfLinkTemplate);
}

/*!
 * \brief TileURL::downloadTile downloads a protobuf tile.
 * \param pbfLink is the URL to make the get request to.
 * \param controller is the network controller making the request.
 * \return the response from the GET request.
 */
HttpResponse TileLoader::downloadTile(const QString &pbfLink)
{
    auto res = networkController.sendRequest(pbfLink);
    if (res.resultType != ResultType::success) {
        qWarning() << "Error: " << PrintResultTypeInfo(res.resultType);
        return { QByteArray(), res.resultType};
    } else {
        return res; // Everything went well, return.
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
QString Bach::setPbfLink(const TileCoord &tileCoord, const QString &pbfLinkTemplate)
{
    // Exchange the {x, y z} in link
    auto copy = pbfLinkTemplate;
    copy.replace("{z}", QString::number(tileCoord.zoom));
    copy.replace("{x}", QString::number(tileCoord.x));
    copy.replace("{y}", QString::number(tileCoord.y));
    return copy;
}
