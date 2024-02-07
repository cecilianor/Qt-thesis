#include "networkcontroller.h"

NetworkController::NetworkController(QObject *parent)
    : QObject{parent}
{
    manager = new QNetworkAccessManager;
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkController::finished);
}

NetworkController::~NetworkController()
{
    delete manager;
}

void NetworkController::sendRequest(QString url)
{
    if(this->url == url) return;
    this->url = url;
    request.setUrl(QUrl(url));
    manager->get(request);
}
