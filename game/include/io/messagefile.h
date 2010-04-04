#ifndef MESSAGEFILE_H
#define MESSAGEFILE_H

#include <QHash>
#include <QString>

namespace EvilTemple
{

    class MessageFile
    {
    public:
        static QHash<quint32, QString> parse(const QByteArray &content);
    private:
        MessageFile();
    };

}

#endif // MESSAGEFILE_H
