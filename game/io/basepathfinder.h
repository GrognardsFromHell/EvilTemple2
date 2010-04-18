#ifndef BASEPATHFINDER_H
#define BASEPATHFINDER_H

#include "gameglobal.h"

#include <QDir>

namespace EvilTemple {

class GAME_EXPORT BasepathFinder
{
public:
    static QDir find();
private:
    explicit BasepathFinder() {}
};

}

#endif // BASEPATHFINDER_H
