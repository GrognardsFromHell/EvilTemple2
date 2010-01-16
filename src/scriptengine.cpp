
#include <QVector2D>

#include "scriptengine.h"
#include "game.h"

namespace EvilTemple {   

    QVector2DClass::QVector2DClass(QScriptEngine *engine) : QObject(engine), QScriptClass(engine)
    {
        qScriptRegisterMetaType<QVector2D>(engine, toScriptValue, fromScriptValue);

        x = engine->toStringHandle(QLatin1String("x"));
        y = engine->toStringHandle(QLatin1String("y"));

        proto = engine->newObject(); // Vector has no prototype

        QScriptValue global = engine->globalObject();        
        proto.setPrototype(global.property("Object").property("prototype"));

        ctor = engine->newFunction(construct, proto);
        ctor.setData(qScriptValueFromValue(engine, this));
    }

    QVector2DClass::~QVector2DClass()
    {
    }

    QScriptValue QVector2DClass::newInstance(const QVector2D &vector)
    {
        QScriptValue data = engine()->newVariant(qVariantFromValue(vector));
        return engine()->newObject(this, data);
    }

    QScriptValue QVector2DClass::newInstance(qreal x, qreal y)
    {
        return newInstance(QVector2D(x, y));
    }

    QScriptValue QVector2DClass::construct(QScriptContext *ctx, QScriptEngine *)
    {
        QVector2DClass *cls = qscriptvalue_cast<QVector2DClass*>(ctx->callee().data());
        if (!cls)
            return QScriptValue();
        QScriptValue arg = ctx->argument(0);
        if (arg.instanceOf(ctx->callee()))
            return cls->newInstance(qscriptvalue_cast<QVector2D>(arg));                
        qsreal x = arg.toNumber();
        qsreal y = ctx->argument(1).toNumber();
        return cls->newInstance(x, y);
    }

    QScriptClass::QueryFlags QVector2DClass::queryProperty(const QScriptValue &object,
                                                           const QScriptString &name,
                                                           QueryFlags flags, uint *id)
    {
        Q_UNUSED(object)
        Q_UNUSED(id)

        if (name == x || name == y) {
            return flags;
        } else {
            if (flags & HandlesReadAccess)
                flags &= ~HandlesReadAccess;
            return flags;
        }
    }

    QScriptValue QVector2DClass::property(const QScriptValue &object, const QScriptString &name, QueryFlags flags,
                                          uint *id)
    {
        Q_UNUSED(id)
        Q_UNUSED(flags)

        QVector2D vector = qscriptvalue_cast<QVector2D>(object.data());
        if (name == x) {
            return vector.x();
        } else if (name == y) {
            return vector.y();
        }
        return QScriptValue();
    }

    void QVector2DClass::setProperty(QScriptValue &object,
                                     const QScriptString &name,
                                     uint id, const QScriptValue &value)
    {
        QVector2D vector = qscriptvalue_cast<QVector2D>(object.data());

        if (name == x) {
            vector.setX(value.toNumber());
        } else if (name == y) {
            vector.setY(value.toNumber());
        }

        object.setData(engine()->newVariant(qVariantFromValue(vector)));
    }

    QScriptValue::PropertyFlags QVector2DClass::propertyFlags(
            const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/)
    {
        if (name == x || name == y) {
            return QScriptValue::Undeletable
                    | QScriptValue::SkipInEnumeration;
        }
        return QScriptValue::Undeletable;
    }

    QString QVector2DClass::name() const
    {
        return QLatin1String("QVector2D");
    }

    QScriptValue QVector2DClass::prototype() const
    {
        return proto;
    }

    QScriptValue QVector2DClass::constructor()
    {
        return ctor;
    }

    QScriptValue QVector2DClass::toScriptValue(QScriptEngine *eng, const QVector2D &ba)
    {
        QScriptValue ctor = eng->globalObject().property("QVector2D");
        QVector2DClass *cls = qscriptvalue_cast<QVector2DClass*>(ctor.data());
        if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
        return cls->newInstance(ba);
    }

    void QVector2DClass::fromScriptValue(const QScriptValue &obj, QVector2D &ba)
    {
        ba = qvariant_cast<QVector2D>(obj.data().toVariant());
    }

    class ScriptEngineData {
    public:
        QScriptEngine *engine; // Owned by the parent, not this class

        ScriptEngineData(ScriptEngine *parent, Game *game) {
            engine = new QScriptEngine(parent);

            QScriptValue global = engine->globalObject();

            // Expose default objects
            QScriptValue gameObject = engine->newQObject(game);
            global.setProperty("game", gameObject);

            // Register conversion functions for frequently used data types
            QVector2DClass *qv2Class = new QVector2DClass(engine);
            global.setProperty("QVector2D", qv2Class->constructor());
        }
    };

    ScriptEngine::ScriptEngine(Game *parent) :
            QObject(parent), d_ptr(new ScriptEngineData(this, parent))
    {
    }

    ScriptEngine::~ScriptEngine()
    {
    }

    QScriptEngine *ScriptEngine::engine() const
    {
        return d_ptr->engine;
    }

}

Q_DECLARE_METATYPE(EvilTemple::QVector2DClass*)
