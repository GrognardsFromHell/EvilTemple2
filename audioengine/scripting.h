#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "audioengineglobal.h"

#include <QScriptEngine>

namespace EvilTemple {

    /**
      Registers the audio engine scripting interface with a QScriptEngine.
      */
    AUDIOENGINE_EXPORT void registerAudioEngine(QScriptEngine *engine);

}

#endif // SCRIPTING_H
