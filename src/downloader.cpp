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
    Private()
        : netManager(nullptr)
        , totalCount(0)
        , currentCount(0)
    {};

    QString downloadUrl;
    QString title;
    QNetworkAccessManager *netManager;
    QQueue<QString> downloadUrlQueue;
    QFile outputFile;

    int totalCount;     // total files to download
    int currentCount;   // current file downloading
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
    connect(reply, &QNetworkReply::finished, this, &Downloader::onNetworkReplyFinished);
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

        // increment current download counter
        d->currentCount += 1;

        qDebug() << QString::fromLatin1("Downloading '%1' -> %2 of %3")
                        .arg(fileDownloadUrl.split('/').last())
                        .arg(d->currentCount)
                        .arg(d->totalCount);

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
            d->outputFile.close();

            reply->deleteLater();

            downloadFromQueue();
        });
    } else {
        qDebug("Download finished");
        QCoreApplication::quit();
    }
}

void Downloader::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (reply == nullptr) {
        qDebug("[Downloader::onNetworkReplyFinished] reply object is null");
        return;
    }

    QByteArray rcv = reply->readAll();
    reply->deleteLater();

    QString regexStartStr("window.params.thumbs = ");
    QRegExp regex(regexStartStr);
    int startIndex = regex.indexIn(rcv);

    // get the portion of data from the start of the file list.
    QString data = rcv.mid(startIndex);
    QRegExp endRegex(";");
    int endIndex = endRegex.indexIn(data);

    QString fileData = data.mid(0, endIndex);

    QList<QString> images = fileData.trimmed().split(" = ").last()
                            .replace('\\', "")
                            .replace("thumbs", "images")
                            .replace("thumb.","")
                            .split("\",\"");

    // clean
    QList<QString> auxImageUrls;
    for (QString imageUrl : images) {

        // clean
        if (imageUrl.at(0) == '[') {
            imageUrl.remove(0, 2);
        }

        if (imageUrl.at(imageUrl.length() - 1) == ']') {
            imageUrl.remove(imageUrl.length() - 2, 2);
        }

        imageUrl.prepend("http:");

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

    // setup counters
    d->totalCount = d->downloadUrlQueue.count();

    // start downloading!
    downloadFromQueue();
}




