#ifndef UTIL_H
#define UTIL_H

#include "prototypes.h"

#include "gamemath.h"
using namespace GameMath;

#include <QString>
#include <QRegExp>
#include <QXmlStreamWriter>
#include <QVariant>

/**
  Converts radians to degree.
  */
inline float deg2rad(float rad)
{
    static const float Pi = 3.14159265358979323846f;
    return rad / 180.f * Pi;
}

inline QString &mangleMaterialName(QString &materialName)
{
    return materialName.replace(QRegExp("\\.\\w+$"), "").replace('/', '-');
}

inline QString &mangleImageName(QString &materialName)
{
    return materialName.replace(QRegExp("\\.tga$"), "-tga").replace('/', '-');
}

/**
  Implements the ELF hash function
  */
inline uint ELFHash(const QByteArray &string)
{
    uint result = 0;

    for (int i = 0; i < string.length(); ++i) {
        result = (result << 4) + (unsigned char)toupper(string[i]);

        // Take the upper nibble
        uint upperNibble = result & 0xF0000000;

        if (upperNibble)
            result ^= upperNibble >> 24;

        result &= ~ upperNibble;
    }

    return result;
}

inline QString getNewMaterialFilename(const QString &mdfFilename) {
    QString newFilename = QDir::toNativeSeparators(mdfFilename);
    if (newFilename.startsWith(QString("art") + QDir::separator(), Qt::CaseInsensitive)) {
        newFilename = newFilename.right(newFilename.length() - 4);
    }

    if (newFilename.endsWith(".mdf", Qt::CaseInsensitive))
    {
        newFilename = newFilename.left(newFilename.length() - 4);
    }

    newFilename.append(".xml");

    return newFilename;
}

inline QString getNewTextureFilename(const QString &mdfFilename) {
    QString newFilename = QDir::toNativeSeparators(mdfFilename);
    if (newFilename.startsWith(QString("art") + QDir::separator(), Qt::CaseInsensitive)) {
        newFilename = newFilename.right(newFilename.length() - 4);
    }
    return newFilename;
}

inline QString getNewBackgroundMapFolder(const QString &directory) {
    // Get last directory name
    QStringList parts = directory.toLower().split('/', QString::SkipEmptyParts);
    QString dirname = parts.last();
    // Remove superflous information from the dirname
    dirname.replace(QRegExp("map\\d*\\-?\\d+\\-?", Qt::CaseInsensitive), "");
    return "backgroundMaps/" + dirname + "/";
}

class JsonPropertyWriter {
public:
    JsonPropertyWriter(QVariantMap &variantMap) : mMap(variantMap)
    {
    }

    void write(const QString &name, const QString &text) {
        if (text.isEmpty())
            return;

        mMap[name] = text;
    }

    template<typename T>
    void write(const QString &name, T value) {
        mMap[name] = QVariant(value);
    }

    template<>
    void write<Troika::Float>(const QString &name, Troika::Float value) {
        if (value.isDefined())
            mMap[name] = QVariant(value.value());
    }

    template<>
    void write<Troika::Integer>(const QString &name, Troika::Integer value) {
        if (value.isDefined())
            mMap[name] = QVariant(value.value());
    }

    void write(const QString &name, const QVariantList &list) {
        if (list.isEmpty())
            return;
        mMap[name] = list;
    }

    void write(const QString &name, const QStringList &flagList) {
        if (flagList.isEmpty())
            return;
        mMap[name] = flagList;
    }

private:
    QVariantMap &mMap;
};

class PropertyWriter {
public:
    PropertyWriter(QXmlStreamWriter &xml) : mXml(xml)
    {
    }

    void write(const QString &name, const QString &text) {
        if (text.isEmpty())
            return;

        mXml.writeTextElement(name, text);
    }

    void write(const QString &name, float value) {
        write(name, QString("%1").arg(value));
    }

    void write(const QString &name, int value) {
        write(name, QString("%1").arg(value));
    }

    void write(const QString &name, uint value) {
        write(name, QString("%1").arg(value));
    }

    template<typename T>
    void write(const QString &name, Troika::Property<T> value) {
        if (value.isDefined())
            write(name, QString("%1").arg(value.value()));
    }

    void write(const QString &name, const QStringList &flagList) {
        if (flagList.isEmpty())
            return;

        mXml.writeStartElement(name);

        foreach (const QString &flag, flagList) {
            mXml.writeEmptyElement(flag);
        }

        mXml.writeEndElement();
    }

private:
    QXmlStreamWriter &mXml;
};

inline QDataStream &operator <<(QDataStream &stream, const Vector4 &vector)
{
    stream << vector.x() << vector.y() << vector.z() << vector.w();
    return stream;
}

#endif // UTIL_H
