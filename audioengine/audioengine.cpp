
#include "audioengine.h"

#include "wavereader.h"
#include "mp3reader.h"

#include <AL/alut.h>

#include <QFile>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QVarLengthArray>

namespace EvilTemple {

/**
  This class wraps a sound source (basically a buffer) and the corresponding
  OpenAL source.
  */
class AudioEngineHandle : public ISoundHandle {
public:
    AudioEngineHandle(ISoundSource *_source);
    ~AudioEngineHandle();

    void setLooping(bool looping);
    bool looping() const;
    bool isValid() const;
    void setVolume(qreal volume);
    qreal volume() const;
    SoundCategory category() const;
    void setCategory(SoundCategory category);

    bool mLooping;
    ALuint sourceId; // The OpenAL source associated with this stream
    ScopedSoundSource source; // The source used to feed the audio stream
    ALenum sampleFormat; // OpenAL sample format corresponding to source
    ALuint sampleFrequency; // OpenAL frequency corresponding to source's sample rate
    QByteArray sampleBuffer; // Stored here to avoid memory allocation in sound thread
    qreal mVolume; // Volume setting
    bool volumeChanged; // Volume setting changed and needs to be set by the audio thread
    SoundCategory mCategory; // The category of this sound effect. Used mainly for global volume
    bool mPaused;
};

AudioEngineHandle::AudioEngineHandle(ISoundSource *_source)
    : source(_source), mLooping(false), sampleBuffer(_source->desiredBuffer(), Qt::Uninitialized), mVolume(1.0),
    mCategory(SoundCategory_Other), mPaused(false)
{
    alGenSources(1, &sourceId);

    // Get valid OpenAL format identifiers now, since we don't want to convert them
    // everytime we upload samples
    const SoundFormat &format = source->format();
    switch (format.channels()) {
    case SoundFormat::Mono:
        switch (format.sampleFormat()) {
        case SoundFormat::UnsignedByte:
            sampleFormat = AL_FORMAT_MONO8;
            break;
        case SoundFormat::SignedShort:
            sampleFormat = AL_FORMAT_MONO16;
            break;
        default:
            qWarning("Unknown sound format sample format: %d", format.sampleFormat());
            break;
        }
        break;
    case SoundFormat::Stereo:
        switch (format.sampleFormat()) {
        case SoundFormat::UnsignedByte:
            sampleFormat = AL_FORMAT_STEREO8;
            break;
        case SoundFormat::SignedShort:
            sampleFormat = AL_FORMAT_STEREO16;
            break;
        default:
            qWarning("Unknown sound format sample format: %d", format.sampleFormat());
            break;
        }
        break;
    default:
        qWarning("Unknown sound format channels: %d", format.channels());
    }

    sampleFrequency = format.sampleRate();
}

AudioEngineHandle::~AudioEngineHandle()
{
    // Free any buffers that might still be attached
    alSourceStop(sourceId);

    ALint processedBuffers, queuedBuffers;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processedBuffers);
    alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queuedBuffers);
    QVector<ALuint> buffers(processedBuffers);
    alSourceUnqueueBuffers(sourceId, buffers.size(), buffers.data());
    alDeleteBuffers(buffers.size(), buffers.data());

    if (queuedBuffers > processedBuffers) {
        qWarning("Stopped source, but still has queued data.");
    }

    alDeleteSources(1, &sourceId);
}

void AudioEngineHandle::setLooping(bool looping)
{
    mLooping = looping;
}

bool AudioEngineHandle::looping() const
{
    return mLooping;
}

void AudioEngineHandle::setVolume(qreal volume)
{
    if (volume < 0) {
        volume = 0;
    } else if (volume > 1) {
        volume = 1;
    }

    mVolume = volume;
    volumeChanged = true;
}

qreal AudioEngineHandle::volume() const
{
    return mVolume;
}

SoundCategory AudioEngineHandle::category() const
{
    return mCategory;
}

void AudioEngineHandle::setCategory(SoundCategory category)
{
    mCategory = category;
    volumeChanged = true;
}

typedef QSharedPointer<AudioEngineHandle> SharedAudioEngineHandle;

/**
    A thread continuously checks for sources that need data or stopped playing data.
    While the main thread controls starting and stopping a source, this thread
    will handle looping sources and notification of sources that stopped playing.
  */
class AudioEngineThread : public QThread {
public:
    AudioEngineThread(qreal *categoryVolume, bool *categoryVolumeChanged);
    void addSoundHandle(const SharedAudioEngineHandle &handle);
    void cancel();
    void setPaused(bool paused) {
        mPaused = paused;
    }
    bool paused() const {
        return mPaused;
    }

protected:
    void run();
private:
    bool handleStoppedSource(const SharedAudioEngineHandle &handle);
    void handlePlayingSource(const SharedAudioEngineHandle &handle);

    qreal *categoryVolume;
    bool *categoryVolumeChanged;
    QList<SharedAudioEngineHandle> audioHandles; // Current audio handles managed by this thread
    QList<SharedAudioEngineHandle> newAudioHandles; // Handles that were added by other threads
    volatile bool checkNewAudioHandles; // Indicates that newAudioHandles contains new handles
    QMutex mutex; // Global audio engine lock
    volatile bool canceled; // Indicates whether audio processing should stop
    volatile bool mPaused; // Pause playback of all sounds except the interface and movie category
};

class AudioEngineData
{
public:
    AudioEngineData() : device(NULL), audioThread(new AudioEngineThread(categoryVolume, categoryVolumeChanged))
    {
        for (int i = 0; i < (int)SoundCategory_Count; ++i) {
            categoryVolume[i] = 1.0;
        }
    }

    QString lastError; // String describing the last error
    ALCdevice *device; // Current device pointer
    ALCcontext *context; // Current listener context
    AudioEngineThread* audioThread; // Thread responsible for the grunt work of the audio engine

    qreal categoryVolume[SoundCategory_Count]; // Volume for each category
    bool categoryVolumeChanged[SoundCategory_Count]; // Flags that indicate whether the volume for a category changed

    void handleError(const QString &operation)
    {
        ALenum errorCode = alcGetError(device);
        if (errorCode != AL_NO_ERROR)
            lastError = QString("%1: %2").arg(operation).arg(errorCode);
    }
};

AudioEngine::AudioEngine(QObject *parent) :
    QObject(parent), d_ptr(new AudioEngineData)
{
}

AudioEngine::~AudioEngine()
{
    close(); // Clean up if the engine is destroyed.
}

bool AudioEngine::paused() const
{
    return d_ptr->audioThread->paused();
}

void AudioEngine::setPaused(bool paused)
{
    d_ptr->audioThread->setPaused(paused);
}

QStringList AudioEngine::devices() const
{
    Q_ASSERT(supportsEnumeration());

    const char *deviceString = alcGetString(NULL, ALC_DEVICE_SPECIFIER);

    QStringList devices;

    while (*deviceString != 0) {
        devices.append(QString::fromLocal8Bit(deviceString));
        while (*deviceString) {
            deviceString++;
        }
        deviceString++; // Skip over the next null byte.
    }

    return devices;
}

bool AudioEngine::supportsEnumeration() const
{
    return alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE;
}

bool AudioEngine::open(const QString &deviceName)
{
    close();

    qDebug("Opening OpenAL device %s.", qPrintable(deviceName));
    d_ptr->device = alcOpenDevice(qPrintable(deviceName));

    if (!d_ptr->device) {
        qWarning("Unable to open OpenAL device %s", qPrintable(deviceName));
        return false;
    }

    d_ptr->context = alcCreateContext(d_ptr->device, NULL);

    if (!d_ptr->context) {
        d_ptr->handleError("Creating context");
        return false;
    }

    if (!alcMakeContextCurrent(d_ptr->context)) {
        d_ptr->handleError("Making context current");
        return false;
    }

    // Start the audio processing thread
    qDebug("Starting audio processing thread.");
    d_ptr->audioThread->start();

    return true;
}

void AudioEngine::close()
{
    if (d_ptr->audioThread->isRunning()) {
        qDebug("Shutting down audio thread.");
        d_ptr->audioThread->cancel();
        d_ptr->audioThread->wait();
    }

    if (d_ptr->context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(d_ptr->context);
        d_ptr->context = NULL;
    }

    if (d_ptr->device) {
        alcCloseDevice(d_ptr->device);
        d_ptr->device = NULL;
    }
}

void AudioEngine::setVolume(qreal volume)
{
    alListenerf(AL_GAIN, volume);
}

qreal AudioEngine::volume() const
{
    ALfloat volume;
    alGetListenerf(AL_GAIN, &volume);
    return volume;
}

void AudioEngine::setVolume(SoundCategory category, qreal volume)
{
    int i = (int)category;
    Q_ASSERT(i >= 0 && i < SoundCategory_Count);

    d_ptr->categoryVolume[i] = qMin<qreal>(1, qMax<qreal>(0, volume));
}

qreal AudioEngine::volume(SoundCategory category) const
{
    int i = (int)category;
    Q_ASSERT(i >= 0 && i < SoundCategory_Count);

    return d_ptr->categoryVolume[i];
}

SharedSoundHandle AudioEngine::playSound(ISound *sound, SoundCategory category, bool looping)
{
    // Try to create a sample source
    QScopedPointer<ISoundSource> soundSource(sound->open());

    if (!soundSource) {
        d_ptr->lastError = "Unable to open the sound " + sound->name();
        return SharedSoundHandle(0);
    }

    SharedAudioEngineHandle result(new AudioEngineHandle(soundSource.take()));

    result->setCategory(category);

    if (looping) {
        result->setLooping(looping);
    }

    // We still need to hold on to the handle in order to advance the sound
    d_ptr->audioThread->addSoundHandle(result);

    return result;
}

const QString &AudioEngine::errorString() const
{
    return d_ptr->lastError;
}

bool AudioEngine::playSoundOnce(const QString &filename, SoundCategory category)
{
    QScopedPointer<ISound> sound;

    if (filename.endsWith(".wav", Qt::CaseInsensitive)) {
        sound.reset(WaveReader::read(filename));
    } else if (filename.endsWith(".mp3", Qt::CaseInsensitive)) {
        sound.reset(MP3Reader::read(filename));
    }

    if (!sound) {
        return false;
    }

    playSound(sound.data(), category);
    return true;

}

AudioEngineThread::AudioEngineThread(qreal *_categoryVolume, bool *_categoryVolumeChanged)
    : categoryVolume(_categoryVolume), categoryVolumeChanged(_categoryVolumeChanged),
    canceled(false), checkNewAudioHandles(false), mPaused(false) {

}

/**
  Runs the audio thread and updates the sources playing in OpenAL.
  */
void AudioEngineThread::run()
{
    while (!canceled) {

        // Check the volatile boolean without locking
        if (checkNewAudioHandles) {
            QMutexLocker locker(&mutex);
            audioHandles.append(newAudioHandles);
            newAudioHandles.clear();
            checkNewAudioHandles = false;
        }

        for (int i = 0; i < audioHandles.size(); ++i) {
            const SharedAudioEngineHandle &handle = audioHandles.at(i);

            Q_ASSERT(handle->category() >= 0 && handle->category() < SoundCategory_Count);

            // Adjust volume if necessary
            if (handle->volumeChanged) {
                float volume = handle->mVolume * categoryVolume[handle->category()];
                alSourcef(handle->sourceId, AL_GAIN, volume);
                handle->volumeChanged = false;
            } else if (categoryVolumeChanged[handle->category()]) {
                float volume = handle->mVolume * categoryVolume[handle->category()];
                alSourcef(handle->sourceId, AL_GAIN, volume);
            }

            ALint sourceState;

            // Check the state of the source
            alGetSourcei(handle->sourceId, AL_SOURCE_STATE, &sourceState);

            switch (sourceState) {
            case AL_INITIAL:
                qDebug("Got new source %d. Queuing data and starting playback.", handle->sourceId);
                handlePlayingSource(handle);
                alSourcePlay(handle->sourceId);
                break;
            case AL_PLAYING:
                handlePlayingSource(handle);
                break;
            case AL_STOPPED:
                if (!handleStoppedSource(handle)) {
                    audioHandles.removeAt(i--);
                    continue;
                }
                break;
            case AL_PAUSED:
                if (!mPaused && !handle->mPaused) {
                    alSourcePlay(handle->sourceId);
                }
                break;
            default:
                // Do nothing
                break;
            }
        }

        // Mark the volume of all categories as up to date
        for (int i = 0; i < SoundCategory_Count; ++i) {
            categoryVolumeChanged[i] = false;
        }

        QThread::usleep(100);
    }

    // The thread has been canceled. Stop and clear all sources

    for (int i = 0; i < audioHandles.size(); ++i) {
        const SharedAudioEngineHandle &handle = audioHandles.at(i);

        // TODO: When a source is stopped, are unprocessed buffers marked as processed automatically?
        alSourceStop(handle->sourceId);
        ALint buffersProcessed;
        alGetSourcei(handle->sourceId, AL_BUFFERS_QUEUED, &buffersProcessed);

        if (buffersProcessed > 0) {
            QVector<ALuint> buffers(buffersProcessed);
            alSourceUnqueueBuffers(handle->sourceId, buffers.size(), buffers.data());
            alDeleteBuffers(buffers.size(), buffers.data());
        }
    }

}

bool AudioEngineThread::handleStoppedSource(const SharedAudioEngineHandle &handle)
{
    Q_ASSERT(!handle->source.isNull());

    // The source may have been starved to the stopped state, in that case queue data
    // and restart the source
    if (!handle->source->atEnd()) {
        handlePlayingSource(handle);
        alSourcePlay(handle->sourceId);
        return true;
    }

    if (handle->looping()) {
        // If the handle is looping, rewind the source and reset to the playing state
        handle->source->rewind();
        alSourcePlay(handle->sourceId);
        handlePlayingSource(handle);
        return true;
    } else {
        // Clean up buffers used by the source
        ALint buffersProcessed;
        alGetSourcei(handle->sourceId, AL_BUFFERS_QUEUED, &buffersProcessed);

        if (buffersProcessed > 0) {
            QVector<ALuint> processedBuffers(buffersProcessed);
            alSourceUnqueueBuffers(handle->sourceId, buffersProcessed, processedBuffers.data());
            alDeleteBuffers(processedBuffers.size(), processedBuffers.data());
        }

        return false;
    }
}

void AudioEngineThread::handlePlayingSource(const SharedAudioEngineHandle &handle)
{

    // Total number of buffers in the queue (incudling processed ones)
    ALint buffersQueued, buffersProcessed;
    alGetSourcei(handle->sourceId, AL_BUFFERS_QUEUED, &buffersQueued);
    alGetSourcei(handle->sourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);

    if (buffersQueued - buffersProcessed > 1) {
        return; // Still got more than one unprocesed buffers
    }

    // We can't do anything if the data ended anyway (TODO: Do looping in here?)
    if (handle->source->atEnd()) {
        qDebug("Got a source that has no more data, stopping the source.");
        alSourceStop(handle->sourceId);
        return;
    }

    // Unqueue used buffers
    if (buffersProcessed > 0) {
        QVector<ALuint> processedBuffers(buffersProcessed);
        alSourceUnqueueBuffers(handle->sourceId, processedBuffers.size(), processedBuffers.data());
        alDeleteBuffers(processedBuffers.size(), processedBuffers.data());
    }

    // Retrieve data from the sound source and upload it to the buffer
    QByteArray &sampleBuffer = handle->sampleBuffer;
    quint32 bufferSize = sampleBuffer.size();
    handle->source->readSamples(sampleBuffer.data(), bufferSize);

    if (bufferSize > 0) {
        ALuint bufferId;
        alGenBuffers(1, &bufferId);
        alBufferData(bufferId, handle->sampleFormat, sampleBuffer.data(), bufferSize, handle->sampleFrequency);
        alSourceQueueBuffers(handle->sourceId, 1, &bufferId);
    }

    // Pause sources if the global state is paused and the sound is not from the interface/movie category
    if (mPaused && handle->category() != SoundCategory_Interface && handle->category() != SoundCategory_Movie) {
        alSourcePause(handle->sourceId);
    }
}

void AudioEngineThread::cancel()
{
    canceled = true;
}

void AudioEngineThread::addSoundHandle(const SharedAudioEngineHandle &handle)
{
    if (canceled) {
        qWarning("Trying to add a sound handle to the audio thread, which is already canceled.");
        return;
    }

    QMutexLocker locker(&mutex);
    newAudioHandles.append(handle);
    checkNewAudioHandles = true;
}

}
