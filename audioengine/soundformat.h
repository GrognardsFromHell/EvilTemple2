#ifndef SOUNDFORMAT_H
#define SOUNDFORMAT_H

#include "audioengineglobal.h"

#include <QtCore/QtGlobal>

namespace EvilTemple
{

/**
  Describes the low level format of a sound.
  */
class AUDIOENGINE_EXPORT SoundFormat
{
public:
    enum Channels {
        Mono = 1,
        Stereo = 2
    };

    enum SampleFormat {
        UnsignedByte = 8,
        SignedShort = 16
    };

    /**
      Constructs a sound format with frequency 0, which is invalid.
      */
    SoundFormat() : mSampleRate(0), mChannels(Mono), mSampleFormat(UnsignedByte) {
    }

    /**
      Creates a sound format description.

      @param sampleRate The number of samples per second this sound is intended to play.
      @param channels The number of channels this sound contains.
      @param sampleFormat The data type of each sample in this sound.
      */
    SoundFormat(quint32 sampleRate, Channels channels, SampleFormat sampleFormat)
        : mSampleRate(sampleRate), mChannels(channels), mSampleFormat(sampleFormat)
    {
    }

    /**
      Returns true if this sound format is valid. Sound formats with sampling rate 0 are invalid.
      */
    bool isValid() const {
        return mSampleRate > 0;
    }

    /**
      Changes the sample rate of this sound format.
      */
    void setSampleRate(quint32 sampleRate) {
        mSampleRate = sampleRate;
    }

    /**
      Changes the number of channels of this sound format.
      */
    void setChannels(Channels channels) {
        mChannels = channels;
    }

    /**
      Changes the sample format of this sound format.
      */
    void setSampleFormat(SampleFormat sampleFormat) {
        mSampleFormat = sampleFormat;
    }

    /**
      Returns at how many samples per second this sound is supposed to be played.
      Is also called the frequency.
      */
    quint32 sampleRate() const {
        return mSampleRate;
    }

    /**
      Returns the number of channels in this sound.
      Valid values are 1 for mono and 2 for stereo.
      */
    Channels channels() const {
        return mChannels;
    }

    /**
      Returns the sample type returned by this source.
      Samples are either unsigned 8 bit values, or 16 bit signed values.
      */
    SampleFormat sampleFormat() const {
        return mSampleFormat;
    }

private:
    quint32 mSampleRate;
    Channels mChannels;
    SampleFormat mSampleFormat;
};

}

#endif // SOUNDFORMAT_H
