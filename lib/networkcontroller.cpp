#include "networkcontroller.h"

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QDebug>

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
    manager = new QNetworkAccessManager;
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkController::finished);
}

/**
 * Network controller destructor.
 *
 * Deletes the network access manager when the destructor is called.
 *
 */
NetworkController::~NetworkController()
{
    delete manager;
}

/**
 * @brief NetworkController::sendRequest sends a get request.
 * @param url The url to make a get request to (as a QString).
 * @return The response from a the get request (sent as a QString url).
 */
QByteArray NetworkController::sendRequest(QString url)
{
    this->url = url;
    request.setUrl(QUrl(url));

    // Perform the GET request
    QNetworkReply *reply = manager->get(request);

    // Create an event loop to wait for the request to finish
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Check for errors
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->errorString();
        reply->deleteLater();
        return QByteArray("Error parsing request.");
    }

    // Process the response
    QByteArray responseData = reply->readAll();
    //qDebug() << "Response: " << responseData; Used for debugging

    // Clean up
    reply->deleteLater();

    // Return response data
    return responseData;
}
