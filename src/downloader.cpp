#include "downloader.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QQueue>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class Downloader::Private
{
public:
    Private() {};

    QString downloadUrl;
    QString title;
    QNetworkAccessManager *netManager;
    QQueue<QString> downloadUrlQueue;
    QFile outputFile;
};

Downloader::Downloader(const QString &downloadUrl, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->netManager = new QNetworkAccessManager(this);
    d->downloadUrl = downloadUrl;

    QStringList urlSplit = d->downloadUrl.split('/');

    // get correct link and extract title
    if (urlSplit.last() != "read") {
        d->downloadUrl += "/read";
    }

    d->title = d->downloadUrl.split('/').at(d->downloadUrl.split('/').count() - 2);

    // initiate download
    QNetworkReply *reply = d->netManager->get(QNetworkRequest(d->downloadUrl));

    connect(reply, &QNetworkReply::finished, [this, reply] () {
        QByteArray rcv = reply->readAll();
        reply->deleteLater();

        QRegExp regex("window.params.thumbs");
        QRegExp endRegex("function update_params");

        int startIndex = regex.indexIn(rcv);
        int endIndex = endRegex.indexIn(rcv);
        QString midStr = rcv.mid(startIndex, (endIndex - startIndex));

        QList<QString> images = midStr.trimmed().split('=').last().trimmed()
                                        .replace('\\', "")
                                        .replace("thumbs", "images")
                                        .replace("thumb.","")
                                        .split("\",\"");

        QList<QString> auxImageUrls;
        for (QString imageUrl : images) {
            imageUrl.replace("\"", "");

            // clean
            if (imageUrl.at(0) == '[') {
                imageUrl.remove(0, 1);
            }

            if (imageUrl.at(imageUrl.count()-1) == ';') {
                imageUrl.remove((imageUrl.count()-2), 2);
            }

            auxImageUrls.append(imageUrl);
            d->downloadUrlQueue.enqueue(imageUrl);
        }

        // create folder for download
        QDir downloadDir;

        if (!downloadDir.mkdir(d->title)) {
            qWarning("Already downloaded this!");
            QCoreApplication::quit();
            return;
        }

        qDebug() << QString::fromLatin1("Downloading into '%1' ...").arg(d->title);

        // start downloading!
        downloadFromQueue();
    });
}


Downloader::~Downloader()
{
    delete d;
}


void Downloader::downloadFromQueue()
{
    if (!d->downloadUrlQueue.isEmpty()) {
        QString fileDownloadUrl = d->downloadUrlQueue.dequeue();
        QNetworkReply *reply = d->netManager->get(QNetworkRequest(fileDownloadUrl));

        d->outputFile.setFileName(d->title + QDir::separator() + fileDownloadUrl.split('/').last());

        qDebug() << QString::fromLatin1("Downloading '%1'").arg(fileDownloadUrl.split('/').last());

        if (!d->outputFile.open(QIODevice::WriteOnly)) {
            qWarning("Can't write to file. Something's wrong");
            QCoreApplication::quit();
            return;
        }

        connect(reply, &QNetworkReply::readyRead, [this, reply] () {
            d->outputFile.write(reply->readAll());
        });

        connect(reply, &QNetworkReply::finished, [this, reply] () {
            d->outputFile.write(reply->readAll());
            reply->deleteLater();
            d->outputFile.close();

            downloadFromQueue();
        });
    } else {
        qDebug("Download finished");
        QCoreApplication::quit();
    }
}



