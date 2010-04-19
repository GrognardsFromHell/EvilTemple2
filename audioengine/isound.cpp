
 #include <QMetaType>

#include "isound.h"

namespace EvilTemple {

ISound::~ISound()
{
}

}

Q_DECLARE_METATYPE(EvilTemple::SharedSound)
