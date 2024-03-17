#include "Utilities.h"

#include <QFile>
#include <QtNetwork>
#include <QTextStream>

#include "TileLoader.h"

bool Bach::writeNewFileHelper(const QString& path, const QByteArray &bytes)
{
    auto fileInfo = QFileInfo{ path };
    // Grab the directory part of this full filepath.
    QDir dir = fileInfo.dir();

    // QFile won't create our directories for us.
    // We gotta make them ourselves.
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        qCritical() << "writeNewFileHelper tried writing to file. Creating parent directory failed.\n";
        return false;
    }

    QFile file { fileInfo.absoluteFilePath() };
    if (file.exists()) {
        qCritical() << " writeNewFileHelper tried writing to file. File already exists.\n";
        return false;
    }

    /* Possible improvement:
     * It's conceivable that a thread might try to read a tile-file
     * that is currently being written to by another thread.
     *
     * To solve this, we might want to introduce a .lock file
     * solution whose presence determines whether a file is currently
     * in use.
     */

    if (!file.open(QFile::WriteOnly)) {
        qCritical() << "writeNewFileHelper tried writing to file. Failed to create new file.\n";
        return false;
    }

    if (file.write(bytes) != bytes.length()) {
        qCritical() << " writeNewFileHelper tried writing to file. Couldn't write the correct amount of bytes.\n";
        return false;
    }

    return true;
}

std::optional<QString> Bach::readMapTilerKey(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        //qDebug() << "Couldn't read key...";
        return std::nullopt;
    }
    QTextStream in(&file);

    return in.readAll().trimmed();
}

HttpResponse Bach::requestAndWait(const QString &url)
{
    QNetworkAccessManager manager;
    QNetworkRequest request{ QUrl(url) };

    // Perform the GET request
    QNetworkReply *reply = manager.get(QNetworkRequest{ QUrl{url} });

    // Creates an event loop to wait for the request to finish.
    QEventLoop loop;
    QObject::connect(
        reply,
        &QNetworkReply::finished,
        &loop,
        &QEventLoop::quit);
    loop.exec();

    // Checks for errors.
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error in file NetworkController.cpp: " << reply->errorString() << '\n';
        return {QByteArray(), ResultType::networkError};
    }

    // Processes the response.
    QByteArray responseData = reply->readAll();
    if (responseData.length() == 0) {
        qWarning() << "No data was returned from the external source";
        return {QByteArray(), ResultType::noData};
    }

    // Deletes the reply.
    reply->deleteLater();
    // Returns response data if everything was successful.
    return {responseData, ResultType::success};
}

HttpResponse Bach::requestStyleSheetFromWeb(StyleSheetType type, const QString &key)
{
    switch(type) {
        case (StyleSheetType::basic_v2) : {
            QString url = "https://api.maptiler.com/maps/basic-v2/style.json?key=" + key;
            return Bach::requestAndWait(url);
        }
        default: {
            qWarning() << "Error: " <<PrintResultTypeInfo(ResultType::noImplementation);
            return {QByteArray(), ResultType::noImplementation};
        }
    }
}

HttpResponse Bach::loadStyleSheetBytes(
    StyleSheetType type,
    const std::optional<QString> &mapTilerKey)
{
    // Create full path for the target stylesheet JSON file
    QString styleSheetCachePath =
        TileLoader::getGeneralCacheFolder() + QDir::separator() +
        "styleSheetCache.json";

    // Try to load the style sheet from file first.
    {
        // We create scope for the QFile so that it doesn't interfere when we possibly write
        // to file later.
        QFile file { styleSheetCachePath };
        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Loading stylesheet from cache. Reading from file...\n";

            // Potential bugs:
            // What if the cache file got garbled at some step before here? There could potentially be
            // more errors here. Note that the stylesheet is only written to the cached file
            // in the first place if the original HTTP request had not-empty data on the expected form.
            return { file.readAll(), ResultType::success }; // Kinda strange signature here, but it must be this way to match its original implementation.
        }
    }


    // If we got here, we need to try loading from web.

    // If we don't have any MapTiler key, we can't make a request.
    if (!mapTilerKey.has_value())
        return { {}, ResultType::unknownError };

    // Make and wait for the web request.
    HttpResponse webResponse = Bach::requestStyleSheetFromWeb(type, mapTilerKey.value());
    if (webResponse.resultType != ResultType::success)
        return webResponse;

    // From here we want try to write this stylesheet to disk cache.
    // If it failed, we consider the entire function a failure.
    bool writeSuccess = writeNewFileHelper(styleSheetCachePath, webResponse.response);
    if (!writeSuccess)
        return { {}, ResultType::unknownError };

    return webResponse;
}

ParsedLink Bach::getPbfLinkTemplate(const QJsonDocument &styleSheet, const QString &sourceType)
{
    // First we need to find the URL to the tiles document.
    ParsedLink tilesUrlResult = getTilesLinkFromStyleSheet(styleSheet, sourceType);
    if (tilesUrlResult.resultType != ResultType::success) {
        qWarning() << "";
        return {QString(), tilesUrlResult.resultType};
    }

    // Grab link to the XYZ PBF tile format based on the tiles.json link
    return getPbfLinkFromTileSheet(tilesUrlResult.link);
}

ParsedLink Bach::getTilesLinkFromStyleSheet(
    const QJsonDocument &styleSheet,
    const QString &sourceType)
{
    // Convert stylesheet to a Json Object.
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
    } else {
        qWarning() << "The stylesheet doesn't contain 'sources' field like it should.\n"
                   << "Check if MapTiler API has been updated to store map sources differently.\n";
        return {QString(), ResultType::parseError};
    }

    qWarning() << PrintResultTypeInfo(ResultType::unknownError);
    return {QString(), ResultType::unknownError};
}

ParsedLink Bach::getPbfLinkFromTileSheet(const QString &tileSheetUrl)
{
    HttpResponse response = requestAndWait(tileSheetUrl);
    if (response.resultType != ResultType::success) {
        qWarning() << "Error: " << PrintResultTypeInfo(response.resultType);
        return { QString(), response.resultType };
    }

    // Parse the tilesheet
    QJsonParseError parseError;
    QJsonDocument tilesSheet = QJsonDocument::fromJson(response.response, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        return { QString(), ResultType::parseError };
    }

    if (tilesSheet.isObject()) {
        QJsonObject jsonObject = tilesSheet.object();
        //qDebug() << "A JSON object was found. It is the following: \n" << jsonObject;
        if(jsonObject.contains("tiles") && jsonObject["tiles"].isArray()) {
            QJsonArray tilesArray = jsonObject["tiles"].toArray();

            for (const QJsonValueRef &tileValue : tilesArray) {
                QString tileLink = tileValue.toString();
                //qDebug() << "\n\t" <<"Link to PBF tiles: " << tileLink <<"\n";
                return { tileLink, ResultType::success };
            }
        }
        else {
            qWarning() << "No 'tiles' array was found in the JSON object...";
        }
    } else if (tilesSheet.isArray()) {
        //The else if branch is just used for testing. Do NOT pushto final version!!
        qWarning() << "A JSON array was found. The current functionality doesn't support this...";
    }
    else {
        qWarning() << "There is an unknown error with the loaded JSON data...";
    }
    return { QString(), ResultType::unknownError };
}
