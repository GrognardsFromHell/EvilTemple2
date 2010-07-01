
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
#include "models.h"
#include "scenenode.h"

#include "renderable.h"
#include "modelinstance.h"
#include "lighting.h"

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

     static QScriptValue timerReference(QScriptContext*, QScriptEngine*)
     {
        QElapsedTimer timer;
        timer.start();
        return QScriptValue((uint)timer.msecsSinceReference());
     }

     template<typename T>
     static QScriptValue renderableCtor(QScriptContext *context, QScriptEngine *engine)
     {
         if (!context->isCalledAsConstructor()) {
             return context->throwError(QScriptContext::SyntaxError, "Please call this function as a "
                                        "constructor using the 'new' keyword.");
         }

         Scene *scene = qobject_cast<Scene*>(context->argument(0).toQObject());

         if (!scene) {
             return context->throwError(QScriptContext::SyntaxError, "A renderable constructor requires the scene as "
                                        "it's first argument");
         }

         T *result = new T;
         result->setParent(scene);
         return engine->newQObject(result);
     }

     ScriptEngineData::ScriptEngineData(ScriptEngine *parent, Game *game)
     {
        engine = new QScriptEngine(parent);

        QScriptValue global = engine->globalObject();

        // Expose default objects
        QScriptValue gameObject = engine->newQObject(game);
        global.setProperty("game", gameObject);

        // Some standard meta types used as arguments throughout the code
        registerQObject<EvilTemple::Game>(engine, "Game*");
        registerQObject<EvilTemple::BackgroundMap>(engine, "BackgroundMap*");
        registerQObject<EvilTemple::ClippingGeometry>(engine, "ClippingGeometry*");
        registerQObject<EvilTemple::Scene>(engine, "Scene*");
        registerQObject<EvilTemple::Materials>(engine, "Materials*");
        registerQObject<EvilTemple::ParticleSystems>(engine, "ParticleSystems*");
        registerQObject<EvilTemple::SectorMap>(engine, "SectorMap*");
        registerQObject<EvilTemple::Models>(engine, "Models*");
        registerQObject<EvilTemple::SceneNode>(engine, "SceneNode*");
        registerQObject<EvilTemple::Renderable>(engine, "Renderable*");
        registerQObject<EvilTemple::ParticleSystem>(engine, "ParticleSystem*");
        registerQObject<EvilTemple::ModelInstance>(engine, "ModelInstance*");

        // Add a function to read files
        QScriptValue readFileFn = engine->newFunction(readFile, 1);
        global.setProperty("readFile", readFileFn);

        QScriptValue timerReferenceFn = engine->newFunction(timerReference, 0);
        global.setProperty("timerReference", timerReferenceFn);

        global.setProperty("ModelInstance", engine->newFunction(renderableCtor<ModelInstance>));
        global.setProperty("Light", engine->newFunction(renderableCtor<Light>));
        global.setProperty("LineRenderable", engine->newFunction(renderableCtor<LineRenderable>));

        // Register scriptable objects
        Vector4Scriptable::registerWith(engine);
        Box3dScriptable::registerWith(engine);
        QuaternionScriptable::registerWith(engine);
        ModelScriptable::registerWith(engine);
        MaterialStateScriptable::registerWith(engine);

        registerQObject<EvilTemple::AudioEngine>(engine, "AudioEngine*");
        registerAudioEngine(engine);
    }

}

Q_DECLARE_METATYPE(EvilTemple::QVector2DClass*)

