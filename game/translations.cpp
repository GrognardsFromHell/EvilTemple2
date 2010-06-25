
#include <QMetaObject>
#include <QObject>
#include <QHash>
#include <QFile>
#include <QDataStream>

#include "translations.h"

namespace EvilTemple {

class TranslationsData
{
public:
    QHash<QString,QString> translations;
};

Translations::Translations(QObject *parent) :
    QObject(parent), d(new TranslationsData)
{
}

Translations::~Translations()
{
}

bool Translations::load(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Unable to read translations from %s.", qPrintable(filename));
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Read pairs while the stream is not at the end
    while (!stream.atEnd()) {
        QString key, value;
        stream >> key >> value;
        d->translations[key] = value;
    }

    return true;
}

QString Translations::get(const QString &key) const
{
    QHash<QString,QString>::const_iterator it = d->translations.find(key);

    if (it == d->translations.end()) {
        return QString("[%1]").arg(key);
    }

    return it.value();
}

}
