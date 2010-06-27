
#include <QVector2D>
#include <QtScriptTools>
#include <QElapsedTimer>

#include "scriptengine.h"
#include "game.h"
#include "backgroundmap.h"
#include "scene.h"
#include "scriptables.h"
#include "clippinggeometry.h"
#include "materials.h"
#include "particlesystem.h"
#include "sectormap.h"

// Fix this, audio engine header name clash
#include "audioengine.h"
#include "scripting.h"

#include <parser.h>

namespace EvilTemple {

    class ScriptEngineData {
    public:
        QScriptEngine *engine; // Owned by the parent, not this class

        ScriptEngineData(ScriptEngine *parent, Game *game);
    };

    ScriptEngine::ScriptEngine(Game *game) :
            QObject(game), d(new ScriptEngineData(this, game))
    {
        connect(d->engine, SIGNAL(signalHandlerException(QScriptValue)), SLOT(handleException(QScriptValue)));
    }

    ScriptEngine::~ScriptEngine()
    {
    }

    void ScriptEngine::handleException(const QScriptValue &exception)
    {
        QScriptValueIterator it(exception);

        qWarning("Unhandled scripting exception: %s.", qPrintable(exception.toString()));

        while (it.hasNext()) {
            it.next();
            qWarning("  %s = %s", qPrintable(it.name()), qPrintable(it.value().toString()));
        }
    }

    void ScriptEngine::callGlobalFunction(const QString &name)
    {
        d->engine->evaluate(name + "()");
        handleUncaughtException();
    }

    void ScriptEngine::handleUncaughtException()
    {
        if (d->engine->hasUncaughtException()) {
            qWarning("Uncaught scripting exception: %s", qPrintable(d->engine->uncaughtException().toString()));
            QStringList backtrace = d->engine->uncaughtExceptionBacktrace();
            foreach (QString line, backtrace) {
                qWarning("    %s", qPrintable(line));
            }
        }
    }

    void ScriptEngine::callGlobalFunction(const QString &name, const QList<QVariant> &arguments)
    {
    }

    QScriptEngine *ScriptEngine::engine() const
    {
        return d->engine;
    }

    bool ScriptEngine::loadScripts()
    {
        QDir scriptDir("data/scripts");

        QStringList scriptFileNames = scriptDir.entryList(QStringList() << "*.js",
                                                          QDir::Files|QDir::Readable,
                                                          QDir::Name);

        foreach (QString scriptFileName, scriptFileNames) {
            QString fullFilename = "data/scripts/" + scriptFileName;

            QFile scriptFile(fullFilename);

            qDebug("Loading script file %s (%d byte).", qPrintable(fullFilename), scriptFile.size());

            if (!scriptFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
                qWarning("Unable to read script file %s: %s", qPrintable(scriptFile.fileName()),
                         qPrintable(scriptFile.errorString()));
                continue;
            }

            QString scriptCode(scriptFile.readAll());
            scriptFile.close();

            d->engine->evaluate(scriptCode, scriptFileName);

            handleUncaughtException();
        }

        return true;
    }

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

    template<typename T>
    QScriptValue qobjectToScriptValue(QScriptEngine *engine, T* const &in)
    {
        return engine->newQObject(in);
    }

    template<typename T>
    void qobjectFromScriptValue(const QScriptValue &object, T* &out)
    {
        out = qobject_cast<T*>(object.toQObject());
    }

    template<typename T>
    void registerQObject(QScriptEngine *engine, const char *name)
    {
        qRegisterMetaType<T*>(name);
        qScriptRegisterMetaType(engine, qobjectToScriptValue<T>, qobjectFromScriptValue<T>);
    }

     static QScriptValue readFile(QScriptContext *context, QScriptEngine *engine)
     {
        QScriptValue filename = context->argument(0);
        QFile file(filename.toString());

        if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            return engine->undefinedValue();
        }

        return QScriptValue(QString::fromUtf8(file.readAll()));
     }

     static QScriptValue timerReference(QScriptContext *context, QScriptEngine *engine)
     {
        QElapsedTimer timer;
        timer.start();
        return QScriptValue((uint)timer.msecsSinceReference());
     }

     ScriptEngineData::ScriptEngineData(ScriptEngine *parent, Game *game)
     {
        engine = new QScriptEngine(parent);

        QScriptValue global = engine->globalObject();

        // Expose default objects
        QScriptValue gameObject = engine->newQObject(game);
        global.setProperty("game", gameObject);

        // Register conversion functions for frequently used data types
        QVector2DClass *qv2Class = new QVector2DClass(engine);
        global.setProperty("QVector2D", qv2Class->constructor());

        // Some standard meta types used as arguments throughout the code
        registerQObject<EvilTemple::Game>(engine, "Game*");
        registerQObject<EvilTemple::BackgroundMap>(engine, "BackgroundMap*");
        registerQObject<EvilTemple::ClippingGeometry>(engine, "ClippingGeometry*");
        registerQObject<EvilTemple::Scene>(engine, "Scene*");
        registerQObject<EvilTemple::Materials>(engine, "Materials*");
        registerQObject<EvilTemple::ParticleSystems>(engine, "ParticleSystems*");
        registerQObject<EvilTemple::SectorMap>(engine, "SectorMap*");

        // Add a function to read files
        QScriptValue readFileFn = engine->newFunction(readFile, 1);
        global.setProperty("readFile", readFileFn);

        QScriptValue timerReferenceFn = engine->newFunction(timerReference, 0);
        global.setProperty("timerReference", timerReferenceFn);

        // Register scriptable objects
        Vector4Scriptable::registerWith(engine);
        Box3dScriptable::registerWith(engine);
        QuaternionScriptable::registerWith(engine);
        SceneNodeScriptable::registerWith(engine);
        ModelScriptable::registerWith(engine);
        ModelInstanceScriptable::registerWith(engine);
        LightScriptable::registerWith(engine);
        registerRenderableScriptable(engine);
        MaterialStateScriptable::registerWith(engine);
        ParticleSystemScriptable::registerWith(engine);
        LineRenderableScriptable::registerWith(engine);

        registerQObject<EvilTemple::AudioEngine>(engine, "AudioEngine*");
        registerAudioEngine(engine);
    }

}

Q_DECLARE_METATYPE(EvilTemple::QVector2DClass*)

