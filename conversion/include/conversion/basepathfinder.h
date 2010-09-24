#ifndef BASEPATHFINDER_H
#define BASEPATHFINDER_H

#include "global.h"

#include <QDir>

namespace EvilTemple {

class CONVERSIONSHARED_EXPORT BasepathFinder
{
public:
    static QDir find();
private:
    explicit BasepathFinder() {}
};

}

#endif // BASEPATHFINDER_H
