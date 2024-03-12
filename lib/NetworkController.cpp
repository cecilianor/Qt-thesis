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
HttpResponse NetworkController::sendRequest(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    // Perform the GET request
    // This will call the appropriate destroy functions
    // on the reply object when the function ends.

    //Nils' code, just commented away in case we want to restore it later.
    //auto reply = QScopedPointer(manager.get(request));
    QNetworkReply *reply = manager.get(request);
    //if (reply == nullptr) {
    //    return std::nullopt;
    //}

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
