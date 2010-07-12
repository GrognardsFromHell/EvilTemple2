
#include "isound.h"
#include "isoundhandle.h"
#include "isoundsource.h"

#include "scripting.h"
#include "scripting_p.h"

using EvilTemple::SharedSound;
using EvilTemple::SharedSoundHandle;
using EvilTemple::SoundCategory;

Q_DECLARE_METATYPE(SharedSound)
Q_DECLARE_METATYPE(SharedSoundHandle)
Q_DECLARE_METATYPE(SoundCategory)

namespace EvilTemple {

QString ISoundPrototype::name() const
{
    SharedSound sound = qscriptvalue_cast<SharedSound>(thisObject());
    if (sound)
        return sound->name();
    return QString();
}

bool ISoundHandlePrototype::looping() const
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        return handle->looping();
    return false;
}

void ISoundHandlePrototype::setLooping(bool looping)
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        handle->setLooping(looping);
}

void ISoundHandlePrototype::stop()
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        handle->stop();
    else
        qWarning("Unable to stop sound handle. Handle is null.");
}

qreal ISoundHandlePrototype::volume() const
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        return handle->volume();
    else
        return 0;
}

void ISoundHandlePrototype::setVolume(qreal volume)
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        handle->setVolume(volume);
}

SoundCategory ISoundHandlePrototype::category() const
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        return handle->category();
    return SoundCategory_Other;
}

void ISoundHandlePrototype::setCategory(SoundCategory category)
{
    SharedSoundHandle handle = qscriptvalue_cast<SharedSoundHandle>(thisObject());
    if (handle)
        handle->setCategory(category);
}

QScriptValue SoundCategory_toScriptValue(QScriptEngine *engine, const SoundCategory &category)
{
    switch (category) {
    case SoundCategory_Music:
        return QScriptValue(engine, "music");
    case SoundCategory_Effect:
        return QScriptValue(engine, "effect");
    case SoundCategory_Interface:
        return QScriptValue(engine, "interface");
    case SoundCategory_Ambience:
        return QScriptValue(engine, "ambience");
    case SoundCategory_Movie:
        return QScriptValue(engine, "movie");
    case SoundCategory_Other:
    default:
        return QScriptValue(engine, "other");
    }
}

void SoundCategory_fromScriptValue(const QScriptValue &obj, SoundCategory &s)
{
  QString categoryName = obj.toString();

  if (categoryName == "music") {
      s = SoundCategory_Music;
  } else if (categoryName == "effect") {
      s = SoundCategory_Effect;
  } else if (categoryName == "interface") {
      s = SoundCategory_Interface;
  } else if (categoryName == "ambience") {
      s = SoundCategory_Ambience;
  } else if (categoryName == "movie") {
      s = SoundCategory_Movie;
  } else if (categoryName == "other") {
      s = SoundCategory_Other;
  } else {
    s = SoundCategory_Other;
    }
}

void registerAudioEngine(QScriptEngine *engine)
{
    qScriptRegisterMetaType<SoundCategory>(engine, SoundCategory_toScriptValue, SoundCategory_fromScriptValue);

    int metaId = qRegisterMetaType<SharedSound>();
    engine->setDefaultPrototype(metaId, engine->newQObject(new ISoundPrototype, QScriptEngine::ScriptOwnership));

    metaId = qRegisterMetaType<SharedSoundHandle>();
    engine->setDefaultPrototype(metaId, engine->newQObject(new ISoundHandlePrototype, QScriptEngine::ScriptOwnership));

    // Register SoundCategory strings in global object
    QScriptValue globalObject = engine->globalObject();
    globalObject.setProperty("SoundCategory_Music", QScriptValue(engine, "music"));
    globalObject.setProperty("SoundCategory_Effect", QScriptValue(engine, "effect"));
    globalObject.setProperty("SoundCategory_Interface", QScriptValue(engine, "interface"));
    globalObject.setProperty("SoundCategory_Ambience", QScriptValue(engine, "ambience"));
    globalObject.setProperty("SoundCategory_Movie", QScriptValue(engine, "movie"));
    globalObject.setProperty("SoundCategory_Other", QScriptValue(engine, "other"));
}

};
