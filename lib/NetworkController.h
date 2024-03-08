#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QNetworkAccessManager>

#include "Utilities.h"

class NetworkController : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager manager;

public:
    explicit NetworkController(QObject *parent = nullptr);

    ~NetworkController();

    HttpResponse sendRequest(QString url);

signals:
    void finished(QNetworkReply *reply);
};

#endif // NETWORKCONTROLLER_H
