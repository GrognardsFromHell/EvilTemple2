#ifndef ANIMINFO_H
#define ANIMINFO_H

#include <QList>

struct AnimEventInfo {
    QString type;
    QString content;
};

struct AnimationInfo {
    QString name;
    uint frames;
    QString driveType;
    float fps;
    float dps;
    uint affectedBones;
    QList<AnimEventInfo> events;
};

#endif // ANIMINFO_H
