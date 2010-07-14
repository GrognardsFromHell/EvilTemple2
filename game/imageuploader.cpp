
#include <QByteArray>
#include <QBuffer>

#include <QNetworkRequest>
#include <QNetworkReply>

#include <parser.h>

#include "imageuploader.h"

namespace EvilTemple {

ImageUploader::ImageUploader(QObject *parent) :
    QObject(parent), mCurrentReply(NULL)
{
}


static const QByteArray base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


QByteArray base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  QByteArray ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;
}

const char *MULTIPART_BOUNDARY = "------imgurUploadBoundary";
const char *MULTIPART_CRLF = "\r\n";

inline static void addStartBoundary(QByteArray &buffer)
{
    buffer.append(MULTIPART_CRLF).append("--").append(MULTIPART_BOUNDARY).append(MULTIPART_CRLF);
}

inline static void addStopBoundary(QByteArray &buffer)
{
    buffer.append(MULTIPART_CRLF).append("--").append(MULTIPART_BOUNDARY).append("--").append(MULTIPART_CRLF);
}

static QByteArray buildRequest(const QByteArray &apiKey, const QByteArray &imageData)
{
    QByteArray result;

    addStartBoundary(result);

    result.append("Content-Disposition: form-data; name=\"key\";").append(MULTIPART_CRLF).append(MULTIPART_CRLF);

    result.append(apiKey);

    addStartBoundary(result);

    result.append("Content-Disposition: form-data; name=\"image\"; filename=\"screenshot.jpg\";").append(MULTIPART_CRLF);
    result.append("Content-Type: application/octet-data").append(MULTIPART_CRLF).append(MULTIPART_CRLF);

    result.append(imageData);

    addStopBoundary(result);

    return result;
}

void ImageUploader::upload(const QUrl &filename)
{
    if (mCurrentReply) {
        mCurrentReply->abort();
        mCurrentReply->deleteLater();
        mCurrentReply = NULL;
    }

    QNetworkRequest request;
    request.setUrl(QUrl("http://imgur.com/api/upload.json"));

    QFile file(filename.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open file for uploading: %s.", qPrintable(filename.toLocalFile()));
        return;
    }

    QByteArray imageData = file.readAll();

    QByteArray data = buildRequest("ee407cd5193cfc4d01b1fd48bcc52594", imageData);
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString("multipart/form-data; boundary=\"%1\"")
                      .arg(MULTIPART_BOUNDARY));

    mCurrentReply = mAccessManager.post(request, data);
    connect(mCurrentReply, SIGNAL(uploadProgress(qint64,qint64)), SIGNAL(uploadProgress(qint64,qint64)));
    connect(mCurrentReply, SIGNAL(finished()), SLOT(finished()));
}

void ImageUploader::finished()
{
    QByteArray content = mCurrentReply->readAll();

    mCurrentReply->deleteLater();
    mCurrentReply = NULL;

    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(content, &ok);

    if (!ok) {
        emit uploadFailed();
    } else {
        emit uploadFinished(result);
    }
}

}
