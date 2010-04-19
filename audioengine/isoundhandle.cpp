
#include <QMetaType>

#include "isoundhandle.h"

namespace EvilTemple {

ISoundHandle::~ISoundHandle()
{
}

}

Q_DECLARE_METATYPE(EvilTemple::SharedSoundHandle)
