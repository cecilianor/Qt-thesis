#include "Utilities.h"

#include <QFile>
#include <QtNetwork>
#include <QTextStream>

#include "TileLoader.h"

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
    // This will call the appropriate destroy functions
    // on the reply object when the function ends.

    QNetworkReply *reply = manager.get(request);

    // Creates an event loop to waits for the request to finish.
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
    QString styleSheetCachePath =
        TileLoader::getGeneralCacheFolder() + QDir::separator() +
        "styleSheetCache.json";

    // Try to load the style sheet from file. Download it from MapTiler if it's not foind.
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

    // If we got here, we need to try loading from web.

    // If we don't have any MapTiler key, we can't make a request.
    if (!mapTilerKey.has_value())
        return { {}, ResultType::unknownError };

    // Make and wait for the web request.
    HttpResponse webResponse = Bach::requestStyleSheetFromWeb(type, mapTilerKey.value());
    if (webResponse.resultType != ResultType::success)
        return webResponse;

    // From here we want try to write this stylesheet to disk cache.
    // It's an error if we failed to open the file.
    if (!file.open(QIODevice::WriteOnly))
        return { {}, ResultType::unknownError };

    // Write to file while checking that we wrote the correct amount.
    if (file.write(webResponse.response) != webResponse.response.size())
        return { {}, ResultType::unknownError };

    return webResponse;
}
