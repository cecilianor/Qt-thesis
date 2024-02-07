#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

class NetworkController : public QObject
{
    Q_OBJECT
public:
    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController();
    void sendRequest(QString url);

signals:
    void finished(QNetworkReply *reply);

private:
    QString url;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
};

#endif // NETWORKCONTROLLER_H
