#include "wavereader.h"
#include "soundformat.h"
#include "isound.h"
#include "isoundsource.h"

#include <QFile>
#include <QDataStream>

namespace EvilTemple {

struct RiffChunk {
    char id[4];
    int size;
};

class StaticWaveSoundSource : public ISoundSource
{
public:

    StaticWaveSoundSource(const QString &name, const QByteArray &data, const SoundFormat &soundFormat)
        : mName(name), mData(data), mSoundFormat(soundFormat), mOffset(0)
    {
    }

    const QString &name() const
    {
        return mName;
    }

    quint32 length() const
    {
        return 0;
    }

    const SoundFormat &format() const
    {
        return mSoundFormat;
    }

    bool atEnd() const
    {
        Q_ASSERT(mOffset <= mData.size() && mOffset >= 0);

        return mOffset >= mData.size();
    }

    void rewind()
    {
        mOffset = 0;
    }

    void readSamples(void *buffer, quint32 &bufferSize)
    {
        Q_ASSERT(mOffset <= mData.size() && mOffset >= 0);

        quint32 remaining = mData.size() - mOffset;
        bufferSize = qMin(bufferSize, remaining);
        memcpy(buffer, mData.data() + mOffset, bufferSize);
        mOffset += bufferSize;
    }

    quint32 desiredBuffer() const
    {
        return 2048;
    }

private:
    QString mName;
    QByteArray mData;
    qint32 mOffset;
    SoundFormat mSoundFormat;

};

class StaticWaveSound : public ISound
{
public:
    StaticWaveSound(const QString &name, const QByteArray &data, const SoundFormat &soundFormat)
        : mName(name), mData(data), mSoundFormat(soundFormat)
    {
    }

    const QString &name() const
    {
        return mName;
    }

    ISoundSource *open()
    {
        return new StaticWaveSoundSource(mName, mData, mSoundFormat);
    }

private:
    QString mName;
    QByteArray mData;
    SoundFormat mSoundFormat;
};

class WaveFile {
public:
    bool open(const QString &filename);

    const SoundFormat &soundFormat() const
    {
        return mSoundFormat;
    }

    const QByteArray &sampleData() const
    {
        return mSampleData;
    }
private:
    bool readChunkHeader(QDataStream &stream, RiffChunk &chunk);
    bool readRiffHeader(QDataStream &stream);
    bool readFormatChunk(QDataStream &stream);
    bool readDataChunk(QDataStream &stream);

    SoundFormat mSoundFormat;
    QString mFilename;
    quint32 waveChunkSize;
    QByteArray mSampleData;
};

ISound *WaveReader::read(const QString &filename)
{
    WaveFile waveFile;

    if (!waveFile.open(filename)) {
        return NULL;
    }

    return new StaticWaveSound(filename, waveFile.sampleData(), waveFile.soundFormat());
}

bool WaveFile::open(const QString &filename)
{
    mFilename = filename;

    QFile waveFile(filename);

    if (!waveFile.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open file %s: %s.", qPrintable(filename), qPrintable(waveFile.errorString()));
        return false;
    }

    QDataStream waveStream(&waveFile);
    waveStream.setByteOrder(QDataStream::LittleEndian);

    if (!readRiffHeader(waveStream)) {
        return false;
    }

    if (!readFormatChunk(waveStream)) {
        return false;
    }

    if (!readDataChunk(waveStream)) {
        return false;
    }

    waveFile.close();

    return true;
}

bool WaveFile::readRiffHeader(QDataStream &stream)
{
    RiffChunk chunk;
    if (!readChunkHeader(stream, chunk)) {
        qWarning("Unable to read RIFF chunk from wave file %s.", qPrintable(mFilename));
        return false;
    }

    if (memcmp(chunk.id, "RIFF", sizeof(chunk.id))) {
        qWarning("Wave file %s doesnt start with RIFF header.", qPrintable(mFilename));
        return false;
    }

    char riffFormat[4];
    if (stream.readRawData(riffFormat, sizeof(riffFormat)) < sizeof(riffFormat)) {
        qWarning("Unable to read format of RIFF structure in Wave file %s.", qPrintable(mFilename));
        return false;
    }

    if (memcmp(riffFormat, "WAVE", sizeof(riffFormat))) {
        qWarning("WAVE file has invalid RIFF format. (%s)", qPrintable(mFilename));
        return false;
    }

    waveChunkSize = chunk.size;

    return true;
}

bool WaveFile::readFormatChunk(QDataStream &stream)
{
    RiffChunk chunkHeader;
    if (!readChunkHeader(stream, chunkHeader)) {
        qWarning("Unable to read header of format chunk for wave %s.", qPrintable(mFilename));
        return false;
    }

    if (memcmp(chunkHeader.id, "fmt ", sizeof(chunkHeader.id))) {
        qWarning("First chunk of RIFF isn't format chunk in wave %s.", qPrintable(mFilename));
        return false;
    }

    if (chunkHeader.size != 16) {
        qWarning("Unknown wave format with size %d in %s.", chunkHeader.size, qPrintable(mFilename));
        return false;
    }

    quint16 audioFormat, channels, blockAlign, bitsPerSample;
    quint32 sampleRate, byteRate;
    stream >> audioFormat >> channels >> sampleRate >> byteRate >> blockAlign >> bitsPerSample;

    if (audioFormat != 1) {
        qWarning("Unknown audio format %d for wave %s.", audioFormat, qPrintable(mFilename));
        return false;
    }

    switch (channels) {
    case 1:
        mSoundFormat.setChannels(SoundFormat::Mono);
        break;
    case 2:
        mSoundFormat.setChannels(SoundFormat::Stereo);
        break;
    default:
        qWarning("Cannot handle %d channels for wave file %s.", channels, qPrintable(mFilename));
        return false;
    }

    switch (bitsPerSample) {
    case 8:
        mSoundFormat.setSampleFormat(SoundFormat::UnsignedByte);
        break;
    case 16:
        mSoundFormat.setSampleFormat(SoundFormat::SignedShort);
        break;
    default:
        qWarning("Cannot handle %d bits per sample in wave file %s.", qPrintable(mFilename));
        return false;
    }

    // Assert that the other values are all correct
    // TODO: Convert to normal checks and warn/save errors?
    Q_ASSERT(byteRate == sampleRate * channels * bitsPerSample / 8);
    Q_ASSERT(blockAlign == channels * bitsPerSample / 8);

    mSoundFormat.setSampleRate(sampleRate);
    return true;
}

bool WaveFile::readDataChunk(QDataStream &stream)
{
    RiffChunk chunkHeader;
    if (!readChunkHeader(stream, chunkHeader)) {
        qWarning("Unable to read header of data chunk for wave %s.", qPrintable(mFilename));
        return false;
    }

    if (memcmp(chunkHeader.id, "data", sizeof(chunkHeader.id))) {
        qWarning("Second chunk of RIFF isn't data chunk in wave %s.", qPrintable(mFilename));
        return false;
    }

    mSampleData.resize(chunkHeader.size);
    int read = stream.readRawData(mSampleData.data(), mSampleData.size());
    if (read < mSampleData.size()) {
        qWarning("Could only read %d of %d bytes in the data chunk of wave file %s.",
                 read, chunkHeader.size, qPrintable(mFilename));
        mSampleData.resize(read);
    }

    return true;
}

bool WaveFile::readChunkHeader(QDataStream &stream, RiffChunk &chunk)
{
    if (stream.readRawData(chunk.id, sizeof(chunk.id)) < sizeof(chunk.id)) {
        qWarning("Unable to read the first 4 magic bytes of Wave file.");
        return false;
    }

    stream >> chunk.size;
    return true;
}

}

