#include "downloader.h"

#include <QtCore/QDebug>

#include <QtNetwork/QNetworkAccessManager>

class Downloader::Private
{
public:
    Private() {};

    QString downloadUrl;
    QNetworkAccessManager *netManager;
};

Downloader::Downloader(const QString &downloadUrl, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->netManager = new QNetworkAccessManager(this);
    d->downloadUrl = downloadUrl;

    qDebug() << "download url: " << d->downloadUrl;
}


Downloader::~Downloader()
{
    delete d;
}


