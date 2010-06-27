#ifndef WAVEREADER_H
#define WAVEREADER_H

#include <QString>

#include "audioengineglobal.h"

#include "isound.h"

namespace EvilTemple {

class WaveReader
{
public:
    static ISound *read(const QString &filename);
};

}

#endif // WAVEREADER_H
