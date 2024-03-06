#include "NetworkController.h"

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QDebug>

#include <QScopedPointer>

/**
 * Network Controller class.
 *
 * @param parent The parent QObject. This object should be the QObject window or widget 
 * you want to draw to.
 *
 */
NetworkController::NetworkController(QObject *parent)
    : QObject{parent}
{
}

/**
 * Network controller destructor.
 *
 * Deletes the network access manager when the destructor is called.
 *
 */
NetworkController::~NetworkController()
{
}

/**
 * @brief NetworkController::sendRequest sends a get request.
 * @param url The url to make a get request to (as a QString).
 * @return The response from a the get request (sent as a QString url).
 */
std::optional<QByteArray> NetworkController::sendRequest(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    // Perform the GET request
    // This will call the appropriate destroy functions
    // on the reply object when the function ends.
    auto reply = QScopedPointer(manager.get(request));
    if (reply == nullptr) {
        return std::nullopt;
    }

    // Create an event loop to wait for the request to finish
    QEventLoop loop;
    QObject::connect(
        reply.get(),
        &QNetworkReply::finished,
        &loop,
        &QEventLoop::quit);
    loop.exec();

    // Check for errors
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->errorString();
        return std::nullopt;
    }

    // Process the response
    QByteArray responseData = reply->readAll();
    if (responseData.length() == 0) {
        return std::nullopt;
    }

    // Return response data
    return responseData;
}
