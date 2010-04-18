#ifndef ISOUND_H
#define ISOUND_H

#include "audioengineglobal.h"

#include <QSharedPointer>
#include <QString>

namespace EvilTemple {

class ISoundSource;

/**
  This interface represents a sound sample, music track or just any other piece of sound.

  It can be opened to supply the audio engine with a stream of samples.
  */
class AUDIOENGINE_EXPORT ISound
{
public:
    virtual ~ISound();

    /**
      Returns the name of this sound. This may be a filename, but fabricated names are also
      allowed. This is used mostly for debugging.
      */
    virtual const QString &name() const = 0;

    /**
      Open a sound source from this sound.
      @returns A sound source, the caller is responsible for deleting it.
      */
    virtual ISoundSource *open() = 0;
};

/**
  Models a sound that holds no data except its name. All processing is done by the
  created sound source.
  */
template<typename T> class LightweightSound : public ISound
{
public:
    LightweightSound(const QString &filename) : mFilename(filename) {
    }

    const QString &name() const
    {
        return mFilename;
    }

    ISoundSource *open()
    {
        return new T(mFilename);
    }

private:
    QString mFilename;
};

typedef QSharedPointer<ISound> SharedSound;

}

#endif // ISOUND_H
