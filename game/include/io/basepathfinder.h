#ifndef BASEPATHFINDER_H
#define BASEPATHFINDER_H

#include <QDir>

namespace EvilTemple {

class BasepathFinder
{
public:
    static QDir find();
private:
    explicit BasepathFinder() {};
};

}

#endif // BASEPATHFINDER_H
