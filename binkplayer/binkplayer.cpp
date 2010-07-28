
#include <QUrl>
#include <QFile>

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
    #include "libswscale/swscale.h"
}

#include "binkplayer.h"

#include <AL/al.h>
#include <AL/alc.h>

static int file_read(URLContext *h, unsigned char *buf, int size)
{
    QFile *file = (QFile*) h->priv_data;

    return file->read(reinterpret_cast<char*>(buf), size);
}

static int file_write(URLContext *h, unsigned char *buf, int size)
{
    qFatal("Writing to QFile from libavformat is not supported.");
    return -1;
}

static int file_open(URLContext *h, const char *rawFilename, int flags)
{
    QString filename = QString::fromLocal8Bit(rawFilename);
    if (filename.startsWith("bfile:"))
        filename = filename.mid(strlen("bfile:"));

    QFile *file = new QFile(filename);

    if (!file->open(QIODevice::ReadOnly)) {
        int error = file->error();
        delete file;
        return AVERROR(error);
    }

    h->priv_data = (void *) file;
    return 0;
}

static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
    QFile *file = (QFile*) h->priv_data;
    if (whence == AVSEEK_SIZE)
        return file->size();

    if (whence == SEEK_SET) {
        if (!file->seek(pos))
            return -1;
    } else if (whence == SEEK_CUR) {
        if (pos == 0)
            return file->pos();
        if (!file->seek(file->pos() + pos))
            return -1;
    } else if (whence == SEEK_END) {
        if (!file->seek(file->size() + pos))
            return -1;
    }

    return file->pos();
}

static int file_close(URLContext *h)
{
    QFile *file = (QFile*) h->priv_data;
    delete file;
    h->priv_data = NULL;
    return 0;
}

static URLProtocol binkfile_protocol = {
    "bfile",
    file_open,
    file_read,
    file_write,
    file_seek,
    file_close,
    0
};

namespace EvilTemple {

static bool avCodecsRegistered = false;

static void initializeCodecs()
{
    if (avCodecsRegistered)
        return;

    fprintf(stderr, "Registering AV Codecs.\n");
    av_register_all();
    av_register_protocol(&binkfile_protocol);
    avCodecsRegistered = true;
}

class BinkPlayerData
{
public:
    QString filename; // Name of the movie to play
    AVFormatContext *formatCtx; // Context for the opened video file
    AVInputFormat *binkFormat; // File format for bink video
    AVCodecContext *videoCodecCtx; // Decoding context for the video stream
    AVCodecContext *audioCodecCtx; // Decoding context for the audio stream
    AVCodec *videoCodec; // Handle to the codec used for the video stream
    AVCodec *audioCodec; // Handle to the codec used for the audio stream
    AVFrame *videoFrame; // Holds a decoded video frame
    AVFrame *audioFrame; // Holds a decoded audio rame
    int audioStreamIndex; // Index of audio stream in file
    int videoStreamIndex; // Index of video stream in file
    QString lastError; // Contains description of the error that occured

    BinkPlayerData()
        : formatCtx(NULL), binkFormat(av_find_input_format("bink")), videoCodecCtx(NULL), audioCodecCtx(NULL)
    {
        if (!binkFormat) {
            qWarning("Unable to retrieve bink input format.");
        } else {
            qDebug("Successfully retrieved BINK input format (%s)", binkFormat->long_name);
        }
    }

    void errorFromCode(const QString &operation, int errorCode)
    {

        QString errorDetails;
        char errorDescription[256];

        switch (errorCode) {
        case AVERROR_INVALIDDATA:
            errorDetails = "Invalid data";
            break;
        case AVERROR_IO:
            errorDetails = "I/O error";
            break;
        case AVERROR_NOENT:
            errorDetails = "No such file or directory";
            break;
        case AVERROR_NOFMT:
            errorDetails = "Unknown format";
            break;
        case AVERROR_NOMEM:
            errorDetails = "Not enough memory";
            break;
        case AVERROR_NUMEXPECTED:
            errorDetails = "Number syntax expected in filename";
            break;
        case AVERROR_EOF:
            errorDetails = "End of file";
            break;
        case AVERROR_NOTSUPP:
            errorDetails = "Operation not supported";
            break;
        default:
#if LIBAVUTIL_VERSION_MAJOR < 50
            errorDetails = QString("Error Code: %1").arg(errorCode);
#else
            av_strerror(errorCode, errorDescription, sizeof(errorDescription));
            errorDescription[sizeof(errorDescription) - 1] = 0; // Always nul-terminate
            errorDetails = QString::fromLocal8Bit(errorDescription);
#endif
        }

        lastError = QString("%1: %2 (File: %3, Error: %4)")
                    .arg(operation).arg(errorDetails).arg(filename).arg(errorCode);
        qWarning("BinkPlayer error: %s", qPrintable(lastError));
    }

    bool findStreams()
    {
        // Retrieve the video & audio stream// Find the first video stream
        for (unsigned int i = 0; i < formatCtx->nb_streams; ++i) {
            AVStream *stream = formatCtx->streams[i];

            if (stream->codec->codec_type == CODEC_TYPE_VIDEO) {
                if (!videoCodecCtx) {
                    videoCodecCtx = stream->codec;
                    videoFrame = avcodec_alloc_frame();
                    videoStreamIndex = i;
                } else {
                    qWarning("Video file %s contains additional video streams.", qPrintable(filename));
                }
            } else if (stream->codec->codec_type == CODEC_TYPE_AUDIO) {
                if (!audioCodecCtx) {
                    audioCodecCtx = stream->codec;
                    audioFrame = avcodec_alloc_frame();
                    audioStreamIndex = i;
                } else {
                    qWarning("Video file %s contains additional audio streams.", qPrintable(filename));
                }
            }
        }

        if (!videoCodecCtx) {
            lastError = "Couldn't find a video stream";
            return false;
        }

        return true;
    }

    bool openCodecs()
    {
        Q_ASSERT(videoCodecCtx);

        if (!openCodec(videoCodecCtx, videoCodec)) {
            return false;
        }

        if (audioCodecCtx) {
            return openCodec(audioCodecCtx, audioCodec);
        } else {
            return true;
        }
    }

    bool openCodec(AVCodecContext *codecCtx, AVCodec *&codec)
    {
        codec = avcodec_find_decoder(codecCtx->codec_id);

        if (!codec) {
            lastError = QString("Unable to find codec %1 for file %2.")
                        .arg(QString::fromLocal8Bit(codecCtx->codec_name))
                        .arg(filename);
            return false;
        }

        // This "random" piece of code comes from the tutorial
        if (codec->capabilities & CODEC_CAP_TRUNCATED)
            codecCtx->flags |= CODEC_FLAG_TRUNCATED;

        int error = avcodec_open(codecCtx, codec);

        if (error < 0) {
            errorFromCode(QString("Opening codec %1").arg(QString::fromLatin1(codecCtx->codec_name)), error);
            return false;
        }

        return true;
    }
};

BinkPlayer::BinkPlayer(QObject *parent) :
    QObject(parent), d_ptr(new BinkPlayerData)
{
}

BinkPlayer::~BinkPlayer()
{
}

// A small macro used to simplify error handling in the following method
#define ASSERT_ERROR(message) if (error < 0) { \
    d_ptr->errorFromCode(message, error); \
    close(); \
    return false; \
    }

bool BinkPlayer::open(const QString &filename)
{
    initializeCodecs();

    if (d_ptr->formatCtx) {
        close();
    }

    d_ptr->filename = filename;

    int error = av_open_input_file(&d_ptr->formatCtx, qPrintable("bfile:" + filename), d_ptr->binkFormat, 0, NULL);
    if (error != 0) {
        d_ptr->errorFromCode("Couldn't open file.", error);
        return false;
    }

    // Read stream information from the opened file
    error = av_find_stream_info(d_ptr->formatCtx);
    ASSERT_ERROR("Couldn't find streams")

    // TODO: Limit this to debug mode
    dump_format(d_ptr->formatCtx, 0, qPrintable(d_ptr->filename), false);

    // Try to find the two streams we actually want to play
    if (!d_ptr->findStreams()) {
        return false;
    }

    // Try to open the codecs used by the streams
    if (!d_ptr->openCodecs()) {
        return false;
    }

    qDebug("Frame dimensions: %dx%d", d_ptr->videoCodecCtx->width, d_ptr->videoCodecCtx->height);

    return true;
}

template <typename T>
struct AvFree : QScopedPointerDeleter<T>
{
    static inline void cleanup(T *pointer)
    {
        av_free(pointer);
    }
};

void BinkPlayer::play()
{
    mStopped = false;

    int error;
    AVPacket packet;
    int frameFinished = 0;

    int width = d_ptr->videoCodecCtx->width;
    int height = d_ptr->videoCodecCtx->height;

    SwsContext *scalingCtx = sws_getContext(width, height,
                                            d_ptr->videoCodecCtx->pix_fmt,
                                            width, height,
                                            PIX_FMT_RGB24,
                                            SWS_FAST_BILINEAR|SWS_PRINT_INFO|SWS_CPU_CAPS_MMX|SWS_CPU_CAPS_MMX2,
                                            NULL, NULL, NULL);

    int bufferSize = avpicture_get_size(PIX_FMT_RGB24, width, height);
    QByteArray convertedFrameBuffer(bufferSize, Qt::Uninitialized);
    AVPicture picture;
    avpicture_fill(&picture, (uint8_t*)convertedFrameBuffer.data(), PIX_FMT_RGB24, width, height);

    int64_t nextFrame = 0;

    int64_t frameTime = av_q2d(d_ptr->videoCodecCtx->time_base) * 1000000;

    ALuint source;
    alGenSources(1, &source);

    while (!mStopped && (error = av_read_frame(d_ptr->formatCtx, &packet)) >= 0) {
        // Audio frame?
        if (packet.stream_index == d_ptr->audioStreamIndex) {
            int frameSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * 3 / 2;
            QScopedPointer< int16_t, AvFree<int16_t> > audioBuffer((int16_t*)av_malloc(frameSize));
            int result = avcodec_decode_audio3(d_ptr->audioCodecCtx,
                                  audioBuffer.data(),
                                  &frameSize,
                                  &packet);

            if (result <= 0) {
                qWarning("Unable to decode audio packet @ %ld.", (long int)packet.pos);
            }

            if (frameSize == 0)
                continue;

            // Unqueue buffers
            ALint sourceType;
            alGetSourcei(source, AL_SOURCE_TYPE, &sourceType);

            if (sourceType == AL_STREAMING) {
                ALint processedBuffers;
                alGetSourcei(source, AL_BUFFERS_PROCESSED, &processedBuffers);
                if (processedBuffers > 0) {
                    QVector<ALuint> buffers;
                    buffers.resize(processedBuffers);
                    alSourceUnqueueBuffers(source, processedBuffers, buffers.data());
                    alDeleteBuffers(processedBuffers, buffers.data());
                }
            }

            ALuint buffer;
            alGenBuffers(1, &buffer);

            alBufferData(buffer, AL_FORMAT_STEREO16, audioBuffer.data(), frameSize, d_ptr->audioCodecCtx->sample_rate);
            alSourceQueueBuffers(source, 1, &buffer);
            ALint sourceState;
            alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
            if (sourceState != AL_PLAYING)
                alSourcePlay(source);
        }
        if (packet.stream_index == d_ptr->videoStreamIndex) {
            error = avcodec_decode_video2(d_ptr->videoCodecCtx,
                                         d_ptr->videoFrame,
                                         &frameFinished,
                                         &packet);

            if (error <= 0) {
                qWarning("Unable to decode video packet @ %ld.", (long int)packet.pos);
            }

            if (frameFinished) {
                AVFrame *frame = d_ptr->videoFrame;

                sws_scale(scalingCtx, frame->data, frame->linesize, 0, d_ptr->videoCodecCtx->height,
                          picture.data, picture.linesize);

                QImage convertedFrame(width, height, QImage::Format_RGB888);

                uchar *sourceData = (uchar*)convertedFrameBuffer.data();
                for (int y = 0; y < height; ++y) {
                    memcpy(convertedFrame.scanLine(y), sourceData, convertedFrame.bytesPerLine());
                    sourceData += picture.linesize[0];
                }

                int64_t sleep = nextFrame - av_gettime();
                while (sleep > 0) {
                    // TODO: Fix this busy waiting....
                    sleep = nextFrame - av_gettime();
                }

                nextFrame = av_gettime() + frameTime;

                emit videoFrame(convertedFrame);
            }
        }

        if (packet.data) {
            av_free_packet(&packet);
            packet.data = NULL;
        }
    }

    if (packet.data)
        av_free_packet(&packet);

    sws_freeContext(scalingCtx);
}

void BinkPlayer::close()
{
    if (d_ptr->formatCtx) {
       if (d_ptr->audioCodec && d_ptr->audioCodecCtx) {
            avcodec_close(d_ptr->audioCodecCtx);
            d_ptr->audioCodec = NULL;
            d_ptr->audioCodecCtx = NULL;
        }

        if (d_ptr->videoCodec && d_ptr->videoCodecCtx) {
            avcodec_close(d_ptr->videoCodecCtx);
            d_ptr->videoCodec = NULL;
            d_ptr->videoCodecCtx = NULL;
        }

        if (d_ptr->videoFrame) {
            av_free(d_ptr->videoFrame);
            d_ptr->videoFrame = NULL;
        }

        if (d_ptr->audioFrame) {
            av_free(d_ptr->audioFrame);
            d_ptr->audioFrame = NULL;
        }

        av_close_input_file(d_ptr->formatCtx);
        d_ptr->formatCtx = NULL;
    }
    d_ptr->filename = QString::null;
}

const QString &BinkPlayer::errorString()
{
    return d_ptr->lastError;
}

void BinkPlayer::stop()
{
    mStopped = true;
}

}
