#ifndef IMAGEUPLOADER_H
#define IMAGEUPLOADER_H

#include <QObject>
#include <QVariantMap>
#include <QImage>
#include <QNetworkAccessManager>

namespace EvilTemple {

class ImageUploader : public QObject
{
    Q_OBJECT
public:
    explicit ImageUploader(QObject *parent = 0);

signals:

    void uploadProgress(qint64 uploadedBytes, qint64 totalBytes);

    void uploadFinished(const QVariant &result);

    void uploadFailed();

public slots:

    void upload(const QUrl &image);

protected slots:
    void finished();

private:
    QNetworkAccessManager mAccessManager;
    QNetworkReply *mCurrentReply;
};

}

#endif // IMAGEUPLOADER_H
