#ifndef WAVEREADER_H
#define WAVEREADER_H

#include "audioengineglobal.h"

#include "isound.h"

#include <QString>

namespace EvilTemple {

class WaveReader
{
public:
    static ISound *read(const QString &filename);
};

}
#endif // WAVEREADER_H
