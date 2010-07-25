#ifndef SCRIPTABLES_H
#define SCRIPTABLES_H

#include "modelfile.h"

#include <QObject>
#include <QScriptable>
#include <QScriptValue>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

    class ModelScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(const Box3d &boundingBox READ boundingBox);
    Q_PROPERTY(float radius READ radius);
    Q_PROPERTY(float radiusSquared READ radiusSquared)
    Q_PROPERTY(QScriptValue animations READ animations)
    public:
        static void registerWith(QScriptEngine *engine);

        const Box3d &boundingBox() const;
        float radius() const;
        float radiusSquared() const;

        QScriptValue animations() const;

    public slots:
        bool hasAnimation(const QString &name) const;
        QScriptValue animationDps(const QString &name) const;
        QScriptValue animationFrames(const QString &name) const;

    private:
        SharedModel data() const;
    };

    class MaterialStateScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    public:
        static void registerWith(QScriptEngine *engine);
    };

    class Vector4Scriptable {
    public:
        static void registerWith(QScriptEngine *engine);
    };

    class QuaternionScriptable {
    public:
        static void registerWith(QScriptEngine *engine);
    };

    class Box3dScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(Vector4 minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(Vector4 maximum READ maximum WRITE setMaximum)
    public:
        Vector4 minimum() const;
        Vector4 maximum() const;
        void setMinimum(const Vector4 &minimum);
        void setMaximum(const Vector4 &maximum);

        static void registerWith(QScriptEngine *engine);
    };

}

#endif // SCRIPTABLES_H
