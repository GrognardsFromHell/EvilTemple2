#ifndef SCRIPTING_P_H
#define SCRIPTING_P_H

#include "audioengineglobal.h"

#include <QMetaType>
#include <QObject>
#include <QScriptable>
#include <QScriptValue>
#include <QScriptEngine>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class ISoundPrototype : public QObject, protected QScriptable {
Q_OBJECT
Q_PROPERTY(QString name READ name)
public slots:
    QString name() const;
};

class ISoundHandlePrototype : public QObject, protected QScriptable {
Q_OBJECT
Q_PROPERTY(bool looping READ looping WRITE setLooping)
Q_PROPERTY(qreal volume READ volume WRITE setVolume)
Q_PROPERTY(SoundCategory category READ category WRITE setCategory)
public:
    bool looping() const;
    void setLooping(bool);
    qreal volume() const;
    void setVolume(qreal);
    SoundCategory category() const;
    void setCategory(SoundCategory);

public slots:
    void stop();

    void setPosition(const Vector4 &position);
    void setMaxDistance(float maxDistance);
    void setReferenceDistance(float refDistance);
};

}

#endif // SCRIPTING_P_H
