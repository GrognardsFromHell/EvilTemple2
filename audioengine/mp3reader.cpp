
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

#include "mp3reader.h"
#include "isound.h"
#include "isoundsource.h"

static bool avCodecsRegistered = false;

static void initializeCodecs()
{
    if (avCodecsRegistered)
        return;

    fprintf(stderr, "Registering AV Codecs.\n");
    av_register_all();
    avCodecsRegistered = true;
}

namespace EvilTemple {

class MP3SoundSource : public ISoundSource
{
public:
    MP3SoundSource(const QString &filename);
    ~MP3SoundSource();

    const QString &name() const;
    quint32 length() const;
    const SoundFormat &format() const;
    bool atEnd() const;
    void rewind();
    quint32 desiredBuffer() const;
    void readSamples(void *buffer, quint32 &bufferSize);
private:
    QString mFilename;
    SoundFormat soundFormat;
    AVFormatContext *formatCtx;
    AVCodec *codec;
    AVCodecContext *codecCtx;
    int streamIndex;
    AVPacket nextPacket;
    bool nextPacketValid;
    bool readNextPacket();
    void copySampleFormat();
    QByteArray sampleBuffer;
    QByteArray decodeBuffer;
};

MP3SoundSource::MP3SoundSource(const QString &filename)
    : mFilename(filename), formatCtx(NULL), codec(NULL), codecCtx(NULL), nextPacketValid(false),
    decodeBuffer(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3 / 2, Qt::Uninitialized)
{
    initializeCodecs();

    int error = av_open_input_file(&formatCtx, qPrintable(filename), NULL, 0, NULL);

    if (error != 0) {
        qWarning("Unable to open MP3 file %s: %d.", qPrintable(filename), error);
        return;
    }

    // Read stream information from the opened file
    error = av_find_stream_info(formatCtx);

    if (error != 0) {
        qWarning("Unable to find streams in MP3 file %s: %d.", qPrintable(filename), error);
        return;
    }

    // TODO: Limit this dump to debug mode
    dump_format(formatCtx, 0, qPrintable(filename), false);

    for (quint32 i = 0; i < formatCtx->nb_streams; ++i) {
        AVStream *stream = formatCtx->streams[i];
        if (stream->codec->codec_type == CODEC_TYPE_AUDIO) {
            streamIndex = i;
            codecCtx = stream->codec;
            codec = avcodec_find_decoder(stream->codec->codec_id);
            if (!codec) {
                qWarning("Found stream %d, but couldn't find matching codec. Skipping.", i);
                codecCtx = NULL;
            }
            break;
        }
    }

    if (!codec) {
        qWarning("Couldn't find a suitable audio stream in the MP3 file %s.", qPrintable(filename));
        return;
    }

    // Try to open the codec for the stream.
    error = avcodec_open(codecCtx, codec);

    if (error != 0) {
        qWarning("Unable to open MP3 codec for stream in file %s: %d.", qPrintable(filename), error);
        return;
    }

    // Copy information from codec context to sound format
    copySampleFormat();

    av_init_packet(&nextPacket);
    memset(&nextPacket, 0, sizeof(nextPacket));
    readNextPacket(); // Validate that we have at least one packet
}

quint32 MP3SoundSource::desiredBuffer() const
{
    // around 500ms of sound should be buffered for MP3s
    switch (soundFormat.sampleFormat()) {
    case SoundFormat::UnsignedByte:
        return soundFormat.sampleRate() / 2;
    case SoundFormat::SignedShort:
        return soundFormat.sampleRate() / 2 * 2;
    default:
        qWarning("Unknown sample format: %d", soundFormat.sampleFormat());
        return soundFormat.sampleRate() / 2;
    }
}

void MP3SoundSource::copySampleFormat()
{
    soundFormat.setSampleRate(codecCtx->sample_rate);

    if (codecCtx->channels == 1) {
        soundFormat.setChannels(SoundFormat::Mono);
    } else if (codecCtx->channels == 2) {
        soundFormat.setChannels(SoundFormat::Stereo);
    } else {
        qWarning("Unsupported number of channels for MP3 %s: %d", qPrintable(mFilename), codecCtx->channels);
    }

    if (codecCtx->sample_fmt == SAMPLE_FMT_U8) {
        soundFormat.setSampleFormat(SoundFormat::UnsignedByte);
    } else if (codecCtx->sample_fmt == SAMPLE_FMT_S16) {
        soundFormat.setSampleFormat(SoundFormat::SignedShort);
    } else {
        qWarning("Unsupported sample format in MP3 %s: %d", qPrintable(mFilename), codecCtx->sample_fmt);
    }

    qDebug("Sample Rate: %d, Channels: %d, Format: %d", soundFormat.sampleRate(), (int)soundFormat.channels(),
           (int)soundFormat.sampleFormat());
}

bool MP3SoundSource::readNextPacket()
{
    nextPacketValid = false;

    if (nextPacket.data)
        av_free_packet(&nextPacket);

    forever {
        int error = av_read_frame(formatCtx, &nextPacket);

        if (error < 0) {
            return false; // Unable to read another packet
        }

        if (nextPacket.stream_index == streamIndex) {
            nextPacketValid = true;
            return true;
        }
    }
}

MP3SoundSource::~MP3SoundSource()
{
    if (codec) {
        avcodec_close(codecCtx);
    }
    if (formatCtx) {
        av_close_input_file(formatCtx);
    }
}

const QString &MP3SoundSource::name() const
{
    return mFilename;
}

const SoundFormat &MP3SoundSource::format() const {
    return soundFormat;
}

quint32 MP3SoundSource::length() const
{
    return 0;
}

bool MP3SoundSource::atEnd() const
{
    // We're at the end of the buffered samples and there is no packet following
    return sampleBuffer.size() == 0 && !nextPacketValid;
}

void MP3SoundSource::rewind()
{
    sampleBuffer.clear();
    av_seek_frame(formatCtx, streamIndex, 0, AVSEEK_FLAG_ANY);
    readNextPacket();
}

void MP3SoundSource::readSamples(void *buffer, quint32 &bufferSize)
{
    Q_ASSERT(nextPacketValid);

    // While the size of our buffer is less than the requested size, try to
    // fullfil the request by parsing packets
    while ((quint32)sampleBuffer.size() < bufferSize && nextPacketValid)
    {
        int frameSize = decodeBuffer.size();
        int result = avcodec_decode_audio2(codecCtx,
                              (int16_t*)decodeBuffer.data(),
                              &frameSize,
                              nextPacket.data,
                              nextPacket.size);

        if (result <= 0) {
            qWarning("Unable to decode audio packet @ %d", nextPacket.pos);
        } else if (result < nextPacket.size) {
            qWarning("TODO: Packet decoding is incomplete.");
        }

        readNextPacket(); // Next packet is now invalid, read another one

        if (frameSize > 0) {
            int oldSize = sampleBuffer.size();
            sampleBuffer.resize(sampleBuffer.size() + frameSize);
            memcpy(sampleBuffer.data() + oldSize, decodeBuffer.data(), frameSize);
        }
    }

    // Consume bytes in our sample buffer.
    int consume = qMin(sampleBuffer.size(), (int)bufferSize);
    memcpy(buffer, sampleBuffer.data(), consume);
    bufferSize = consume;

    sampleBuffer = sampleBuffer.right(sampleBuffer.size() - consume);

}

ISound *MP3Reader::read(const QString &filename)
{
    return new LightweightSound<MP3SoundSource>(filename);
}

}
