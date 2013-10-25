#include <QtCore/QObject>

class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader(const QString &downloadUrl, QObject* parent = 0);
    virtual ~Downloader();

    void downloadFromQueue();

private:
    class Private;
    Private * const d;
};
