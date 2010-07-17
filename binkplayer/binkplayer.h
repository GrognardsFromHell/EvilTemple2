#ifndef BINKPLAYER_H
#define BINKPLAYER_H

#include "binkplayerglobal.h"

#include <QObject>
#include <QImage>
#include <QScopedPointer>

namespace EvilTemple {

class BinkPlayerData;

class BINKPLAYER_EXPORT BinkPlayer : public QObject
{
Q_OBJECT
public:
    explicit BinkPlayer(QObject *parent = 0);
    ~BinkPlayer();

signals:

    void videoFrame(const QImage &frame);

public slots:

    bool open(const QString &filename); // Opens the movie file
    void close();
    void play();
    void stop();

    const QString &errorString();

private:
    QScopedPointer<BinkPlayerData> d_ptr;
    volatile bool mStopped;
};

}

#endif // BINKPLAYER_H
