#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QNetworkAccessManager>

class NetworkController : public QObject
{
    Q_OBJECT
public:
    explicit NetworkController(QObject *parent = nullptr);

    ~NetworkController();

    QByteArray sendRequest(QString url);

signals:
    void finished(QNetworkReply *reply);

private:
    QString url;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
};

#endif // NETWORKCONTROLLER_H
/*
class NetworkController : public QObject
{
    Q_OBJECT
public:
    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController();
    void sendRequest(QString url);
    QByteArray HTTPGetData(QNetworkRequest request);

signals:
    void finished(QNetworkReply *reply);

private:
    QString url;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
};
*/

/*
class HTTPHandler
{
public:
    HTTPHandler();

    QByteArray HTTPGetData(QNetworkRequest request);
};
*/
