#include "io/basepathfinder.h"

namespace EvilTemple {

QDir BasepathFinder::find() {
    return QDir::current();
}

}
