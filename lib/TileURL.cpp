#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "TileURL.h"
#include "networkcontroller.h"
#include "qjsonarray.h"

TileURL::TileURL() {};

/**
 * @brief TileURL::getStylesheet gets a stylesheet from mapTiler
 * @param type The type of the stylesheet passed as an enum
 * @return response
 */
std::pair<QByteArray, TileURL::ErrorCode> TileURL::getStylesheet(styleSheetType type) {
    NetworkController networkController;
    QByteArray response="";

    switch(type) {
    case (TileURL::styleSheetType::basic_v2) : {
        response = networkController.sendRequest(QString(
            "https://api.maptiler.com/maps/basic-v2/style.json?key=bWo4cKyYIs8K3SkrLiTk"));
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

/**
 * @brief TileURL::getTilesLink grabs a link to a mapTiler tile sheet
 * @param styleSheet is the stylesheet to get the link from
 * @param sourceType is the map source type passed as a QString
 * @return link as a string
 */
std::pair<QString, TileURL::ErrorCode> TileURL::getTilesLink(const QJsonDocument & styleSheet, QString sourceType)
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

/**
 * @brief TileURL::getTilelesheet gets a stylesheet based on a map's mapTiler source type.
 * It's been implemented for the maptiler_planet source type, but the class can support
 * other source types as well (not implemented yet).
 *
 * The function makes a get request to the corresponding maptiler API and returns the
 * response.
 *
 * BUG: This function should not have the sourceTypes hardcoded. I managed to mix
 * up the style sheets and tile sheets while implementing. This source code
 * should just be used for testing purposes.
 *
 * @param type the source type (enum).
 * @return The response from the GET request. The default is to return "".
 */
std::pair<QByteArray, TileURL::ErrorCode> TileURL::getPBFLink(sourceType type) {
    // Set up a network controller to make a request to mapTiler
    NetworkController networkController;
    QByteArray response ="";

    switch (type) {
    case (TileURL::sourceType::maptiler_planet) : {
        // Note here that maptiler_planet links to maptype 'v3'.
        response = networkController.sendRequest(QString(
            "https://api.maptiler.com/tiles/v3/tiles.json?key=bWo4cKyYIs8K3SkrLiTk"));
        break;
    }
    case(TileURL::sourceType::land) : {
        response = networkController.sendRequest(QString(
            "https://api.maptiler.com/tiles/land/tiles.json?key=bWo4cKyYIs8K3SkrLiTk"));
        break;
    }
    case (TileURL::sourceType::ocean) : {
        response = networkController.sendRequest(QString(
            "https://api.maptiler.com/maps/ocean/style.json?key=bWo4cKyYIs8K3SkrLiTk"));
        break;

    }
    case(TileURL::sourceType::unknown) : {
        return std::make_pair("Unknown source type", ErrorCode::unknownError);
        break;
    }
    default: {
        // Return an empty string as the default case.
        return std::make_pair("Unknown source type", ErrorCode::unknownError);
        break;
        }
    }

    return std::make_pair(response, ErrorCode::success);
};

/**
 * @brief TileURL::getPBFLink gets a PBF link based on tileSheet link.
 * @param tileSheetUrl the link/url to the stylesheet.
 * @return The link to PBF tiles.
 */
std::pair<QString, TileURL::ErrorCode> TileURL::getPBFLink (const QString & tileSheetUrl) {
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
