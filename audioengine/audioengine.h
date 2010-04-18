#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include "audioengineglobal.h"

#include <QObject>
#include <QScopedPointer>
#include <QStringList>
#include <QByteArray>
#include <QSharedPointer>

#include "isoundsource.h"
#include "isound.h"
#include "isoundhandle.h"

namespace EvilTemple {

class AudioEngineData;

class AUDIOENGINE_EXPORT AudioEngine : public QObject
{
Q_OBJECT
Q_PROPERTY(QStringList devices READ devices)
public:
    explicit AudioEngine(QObject *parent = 0);
    ~AudioEngine();

signals:

public slots:

    /**
      Returns whether this audio engine supports the enumeration of installed devices,
      or whether only the default device can be opened.
      */
    bool supportsEnumeration() const;

    /**
        If enumeration of devices is supported, this will return a list of the devices
        in the system.
      */
    QStringList devices() const;

    /**
        Tries to open the device given by deviceName. Closes an opened device first.
        Giving a null string will open the preferred device.
      */
    bool open(const QString &deviceName = QString::null);

    /**
      Closes the current device. This method is called by the destructor automatically.
      */
    void close();

    /**
      Returns a string describing the last error. If a method call fails, this method
      can be used to retrieve the reason for failure.
      */
    const QString &errorString() const;

    /**
        Plays a sound and returns a handle to it, that can be used to alter the sound.
        @param sound The sound to play. The engine will hold no reference to this object, while the
        returned handle might.
        @return Null if the sound cannot be played, or a shared pointer to the sound handle.
      */
    SharedSoundHandle playSound(ISound *sound, bool looping = false);

    /**
      Changes the overall volume for all sounds.

      The volume for a particular sound will be multiplied by this volume and the corresponding category's volume.

      @param volume The new volume for all sounds. This will be clamped to [0,1].
      */
    void setVolume(qreal volume);

    /**
      Returns the overall volume for all played sounds.
      */
    qreal volume() const;

    /**
      Changes the volume for a single sound category.

      The volume for a particular sound will be multiplied by this volume and the overall volume.

      @param category The category for which the volume should be changed.
      @param volume The volume for sounds in this category. Will be clamped to [0,1].
      */
    void setVolume(SoundCategory category, qreal volume);

    /**
      Returns the volume of a particular sound category.
      */
    qreal volume(SoundCategory category) const;

    /**
      Pauses all currently playing sounds and future sounds, except sounds in the interface and movie category.
      */
    void setPaused(bool paused);

    /**
      Returns whether sounds are currently paused (except the interface and movie category).
      */
    bool paused() const;

private:
    QScopedPointer<AudioEngineData> d_ptr;

    Q_DISABLE_COPY(AudioEngine)
};

}

#endif // AUDIOENGINE_H
