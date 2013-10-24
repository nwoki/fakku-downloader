#include "downloader.h"

#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

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

    if (d->downloadUrl.split('/').last() != "read") {
        d->downloadUrl += "/read";
    }

    // initiate download
    QNetworkReply *reply = d->netManager->get(QNetworkRequest(d->downloadUrl));

    connect(reply, &QNetworkReply::finished, [this, reply] () {
        QByteArray rcv = reply->readAll();
        reply->deleteLater();

        QRegExp regex("window.params.thumbs");
        QRegExp endRegex("function update_params");

        int startIndex = regex.indexIn(rcv);
        int endIndex = endRegex.indexIn(rcv);

        QByteArray midStr = rcv.mid(startIndex, (endIndex - startIndex));

        QList<QByteArray> images = midStr.trimmed().split('=').last().trimmed()
                                        .replace('"', "")
                                        .replace('\\', "")
                                        .replace("thumbs", "images")
                                        .replace("thumb.","")
                                        .split(',');


        QList<QByteArray> imageUrls;
        for (QByteArray imageUrl : images) {
            // clean
            if (imageUrl.at(0) == '[') {
                imageUrl.remove(0, 1);
            }

            if (imageUrl.at(imageUrl.count()-1) == ';') {
                imageUrl.remove((imageUrl.count()-2), 2);
            }

            imageUrls << imageUrl;
        }

        qDebug() << "finals" << imageUrls;
    });
}


Downloader::~Downloader()
{
    delete d;
}


