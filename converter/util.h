#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QRegExp>

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

#endif // UTIL_H
