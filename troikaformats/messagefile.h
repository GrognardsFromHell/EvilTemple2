#ifndef MESSAGEFILE_H
#define MESSAGEFILE_H

#include "troikaformatsglobal.h"

#include <QHash>
#include <QString>

namespace Troika
{

    class TROIKAFORMATS_EXPORT MessageFile
    {
    public:
        static QHash<quint32, QString> parse(const QByteArray &content);
    private:
        MessageFile();
    };

}

#endif // MESSAGEFILE_H
