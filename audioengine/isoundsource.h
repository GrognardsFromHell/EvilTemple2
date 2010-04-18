#ifndef ISAMPLESOURCE_H
#define ISAMPLESOURCE_H

#include "audioengineglobal.h"

#include <QScopedPointer>

#include "soundformat.h"

namespace EvilTemple {

/**
  Describes a common interface that needs to be implemented by all sources of sound data in the
  system. This includes streams (from network resources for instance), direct sample representations
  without a decoding step and also the output of MP3 decoders and other codecs.
  */
class AUDIOENGINE_EXPORT ISoundSource
{
public:

    virtual ~ISoundSource();

    /**
      Returns a name for this sound source. In case of files, this should be the filename.
      But if this stream originates from somewhere else, it may also be a fabricated name.
      */
    virtual const QString &name() const = 0;

    /**
      Returns the length of the sound presented by this sample source in milliseconds.
      */
    virtual quint32 length() const = 0;

    /**
      Returns the format of this sound.
      */
    virtual const SoundFormat &format() const = 0;

    /**
      Returns whether this sample source is at the end and cannot return any more
      samples.
      */
    virtual bool atEnd() const = 0;

    /**
      Rewinds the sample source to the first sample.
      */
    virtual void rewind() = 0;

    /**
      Reads samples from this sample source and puts the samples into a buffer.

      The method may read less data than the buffer can hold.

      @param buffer Will receive the samples.
      @param bufferSize The size of buffer in bytes. Will receive the number of bytes
        actually read from this source.
      */
    virtual void readSamples(void *buffer, quint32 &bufferSize) = 0;

    /**
      Returns how many bytes the audio engine should use for its audio buffer for this sound.
      */
    virtual quint32 desiredBuffer() const = 0;

};

/**
  Sound sources should not be shared, and therefore this typedef defines a scoped pointer
  for sound sources.
  */
typedef QScopedPointer<ISoundSource> ScopedSoundSource;

}

#endif // ISAMPLESOURCE_H
