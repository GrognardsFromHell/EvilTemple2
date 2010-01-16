#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QtScript/QtScript>
class QVector2D;

namespace EvilTemple {
    class Game;
    class ScriptEngineData;

    class ScriptEngine : public QObject
    {
    Q_OBJECT
    public:
        explicit ScriptEngine(Game *parent = 0);
        ~ScriptEngine();

        QScriptEngine *engine() const;

    private:
        QScopedPointer<ScriptEngineData> d_ptr;

        Q_DISABLE_COPY(ScriptEngine)
    };

    class QVector2DClass : public QObject, public QScriptClass
    {
        Q_OBJECT
    public:
        QVector2DClass(QScriptEngine *engine);
        ~QVector2DClass();

        QScriptValue constructor();

        QScriptValue newInstance(qreal x, qreal y);
        QScriptValue newInstance(const QVector2D &vector);

        QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id);
        QScriptValue property(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id);

        void setProperty(QScriptValue &object, const QScriptString &name,
                         uint id, const QScriptValue &value);

        QScriptValue::PropertyFlags propertyFlags(
                const QScriptValue &object, const QScriptString &name, uint id);

        QString name() const;

        QScriptValue prototype() const;

    private:
        static QScriptValue construct(QScriptContext *ctx, QScriptEngine *eng);

        static QScriptValue toScriptValue(QScriptEngine *eng, const QVector2D &ba);
        static void fromScriptValue(const QScriptValue &obj, QVector2D &ba);

        QScriptString x;
        QScriptString y;
        QScriptValue ctor;
        QScriptValue proto;
    };

}

#endif // SCRIPTENGINE_H
