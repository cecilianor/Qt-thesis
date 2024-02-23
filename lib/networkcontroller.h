#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QNetworkAccessManager>

class NetworkController : public QObject
{
    Q_OBJECT
private:
    QString url;
    QNetworkAccessManager *manager;
    QNetworkRequest request;

public:
    explicit NetworkController(QObject *parent = nullptr);

    ~NetworkController();

    QByteArray sendRequest(QString url);

signals:
    void finished(QNetworkReply *reply);
};

#endif // NETWORKCONTROLLER_H
