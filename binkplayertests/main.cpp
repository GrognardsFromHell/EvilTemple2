
#include <stdio.h>
#include <QThread>
#include <QApplication>

#include "audioengine.h"
#include "binkplayer.h"
#include "dialog.h"

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}

static void registerCodecs()
{
    fprintf(stderr, "Registering AV Codecs.\n");
    avcodec_register_all();

    fprintf(stderr, "Registering AV Formats.\n");
    av_register_all();
}

class PlayerThread : public QThread
{
public:
    PlayerThread(EvilTemple::BinkPlayer &player) : mPlayer(player) {
    }
protected:
    EvilTemple::BinkPlayer &mPlayer;
    void run()
    {
        mPlayer.play();
    }
};

int main(int argc, char *argv[])
{
    registerCodecs();

    QApplication app(argc, argv);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <bink file>\n", argv[0]);
        return -1;
    }

    QString filename = QString::fromLocal8Bit(argv[1]);

    qDebug("Trying to play \"%s\".", qPrintable(filename));

    EvilTemple::BinkPlayer player;

    if (!player.open(filename)) {
        qDebug("Unable to open bink file: %s", qPrintable(player.errorString()));
    }

    PlayerThread thread(player);

    Dialog dialog;
    dialog.connect(&player, SIGNAL(videoFrame(QImage)),
                   &dialog, SLOT(showFrame(QImage)));
    dialog.show();

    AudioEngine audioEngine;
    audioEngine.open();

    thread.start();

    int result = app.exec();

    thread.wait();

    return result;
}

