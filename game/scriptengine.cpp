
#include <QtScriptTools>
#include <QElapsedTimer>
#include <QUuid>

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
#include "selectioncircle.h"
#include "imageuploader.h"
#include "fogofwar.h"
#include "modelviewer.h"
#include "geometryrenderables.h"
#include "tileinfo.h"
#include "pathfinder.h"

#include "renderable.h"
#include "modelinstance.h"
#include "lighting.h"

// Fix this, audio engine header name clash
#include "audioengine.h"
#include "scripting.h"

#include <parser.h>

Q_DECLARE_METATYPE(QMouseEvent*)

namespace EvilTemple {

    class ScriptEngineData {
    public:
        ScriptEngineData(ScriptEngine *parent, Game *game);

        QScriptEngine *engine; // Owned by the parent, not this class

        QScriptEngineDebugger *debugger; // Also owned by the parent
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

    void ScriptEngine::callGlobalFunction(const QString &name, const QScriptValueList &arguments)
    {
        QScriptValue func = d->engine->globalObject().property(name);

        if (!func.isNull()) {
            func.call(QScriptValue(), arguments);
        } else {
            qWarning("Global function %s not found.", qPrintable(name));
        }

        handleUncaughtException();
    }

    QScriptEngine *ScriptEngine::engine() const
    {
        return d->engine;
    }

    bool ScriptEngine::loadScripts(const QString &directory)
    {
        QDir scriptDir(QDir::toNativeSeparators(directory));
        qDebug("Loading scripts from %s.", qPrintable(scriptDir.absolutePath()));

        // Subdirectories are loaded first
        QStringList subdirectoryNames = scriptDir.entryList(QStringList(),
                                                          QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot,
                                                          QDir::Name);

        foreach (const QString &subdirectoryName, subdirectoryNames) {
            qDebug("Loading scripts from directory %s.", qPrintable(subdirectoryName));
            loadScripts(scriptDir.absolutePath() + '/' + subdirectoryName);
        }

        QStringList scriptFileNames = scriptDir.entryList(QStringList() << "*.js",
                                                          QDir::Files|QDir::Readable,
                                                          QDir::Name);

        foreach (QString scriptFileName, scriptFileNames) {
            QString fullFilename = scriptDir.absolutePath() + '/' + scriptFileName;

            QFile scriptFile(fullFilename);

            qDebug("Loading script file %s (%ld byte).", qPrintable(scriptFileName), (long int)scriptFile.size());

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

    static QScriptValue writeFile(QScriptContext *context, QScriptEngine *engine)
    {
        QString filename = context->argument(0).toString();
        QString content = context->argument(1).toString();

        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)) {
            return QScriptValue(false);
        }

        file.write(content.toUtf8());
        return QScriptValue(true);
    }

    static QScriptValue fileExists(QScriptContext *context, QScriptEngine *engine)
    {
        QScriptValue filename = context->argument(0);

        return QFile::exists(filename.toString());
    }

    static QScriptValue timerReference(QScriptContext*, QScriptEngine*)
    {
        QElapsedTimer timer;
        timer.start();
        return QScriptValue((uint)timer.msecsSinceReference());
    }

    static QScriptValue generateGuid(QScriptContext*, QScriptEngine*)
    {
        return QUuid::createUuid().toString();
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

    template<typename T>
    static QScriptValue renderableWithMaterialsCtor(QScriptContext *context, QScriptEngine *engine)
    {
        if (!context->isCalledAsConstructor()) {
            return context->throwError(QScriptContext::SyntaxError, "Please call this function as a "
                                       "constructor using the 'new' keyword.");
        }

        Scene *scene = qobject_cast<Scene*>(context->argument(0).toQObject());
        Materials *materials = qobject_cast<Materials*>(context->argument(1).toQObject());

        if (!scene || !materials) {
            return context->throwError(QScriptContext::SyntaxError, "This renderable constructor requires the "
                                       "scene as its first argument and materials as its second");
        }

        T *result = new T(materials);
        result->setParent(scene);
        return engine->newQObject(result);
    }

    QScriptValue qMouseEventToScriptable(QScriptEngine *engine, QMouseEvent* const &in)
    {
        QScriptValue result = engine->newObject();

        result.setProperty("button", in->button());
        result.setProperty("buttons", (int)in->buttons());
        result.setProperty("modifiers", (int)in->modifiers());
        result.setProperty("x", in->x());
        result.setProperty("y", in->y());

        return result;
    }

    void qMouseEventFromScriptable(const QScriptValue &object, QMouseEvent* &out)
    {
        qFatal("Trying to convert a QScriptValue to QMoueEvent. This is not supported.");
    }


    QScriptValue qByteArrayToScriptable(QScriptEngine *engine, QByteArray const &in)
    {
        return QScriptValue(QString::fromLatin1(in));
    }

    void qByteArrayFromScriptable(const QScriptValue &object, QByteArray &out)
    {
        out = object.toString().toLatin1();
    }

    ScriptEngineData::ScriptEngineData(ScriptEngine *parent, Game *game)
        : engine(new QScriptEngine(parent)),
        debugger(NULL)
    {
        engine->installTranslatorFunctions();

        // Debug the script engine if requested by the user
        foreach (const QString &argument, QCoreApplication::instance()->arguments()) {
            if (argument == "-scriptdebugger") {
                debugger = new QScriptEngineDebugger(game);
                debugger->attachTo(engine);
                break;
            }
        }

        qScriptRegisterMetaType<QMouseEvent*>(engine, qMouseEventToScriptable, qMouseEventFromScriptable);
        qScriptRegisterMetaType<QByteArray>(engine, qByteArrayToScriptable, qByteArrayFromScriptable);

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
        registerQObject<EvilTemple::SelectionCircle>(engine, "SelectionCircle*");
        registerQObject<EvilTemple::ModelInstance>(engine, "ModelInstance*");
        registerQObject<EvilTemple::ModelViewer>(engine, "ModelViewer*");
        registerQObject<EvilTemple::TileInfo>(engine, "TileInfo*");
        registerQObject<EvilTemple::Pathfinder>(engine, "Pathfinder*");

        // Add a function to read files
        QScriptValue readFileFn = engine->newFunction(readFile, 1);
        global.setProperty("readFile", readFileFn);
        QScriptValue writeFileFn = engine->newFunction(writeFile, 2);
        global.setProperty("writeFile", writeFileFn);

        QScriptValue fileExistsFn = engine->newFunction(fileExists, 1);
        global.setProperty("fileExists", fileExistsFn);

        QScriptValue timerReferenceFn = engine->newFunction(timerReference, 0);
        global.setProperty("timerReference", timerReferenceFn);

        global.setProperty("generateGuid", engine->newFunction(generateGuid, 0));

        global.setProperty("TileInfo", engine->newQMetaObject(&TileInfo::staticMetaObject));
        global.setProperty("Pathfinder", engine->newQMetaObject(&Pathfinder::staticMetaObject));

        global.setProperty("ModelInstance", engine->newFunction(renderableCtor<ModelInstance>));
        global.setProperty("Light", engine->newFunction(renderableCtor<Light>));
        global.setProperty("LineRenderable", engine->newFunction(renderableCtor<LineRenderable>));
        global.setProperty("BackgroundMap", engine->newFunction(renderableCtor<BackgroundMap>));
        global.setProperty("FogOfWar", engine->newFunction(renderableCtor<FogOfWar>));
        global.setProperty("SelectionCircle", engine->newFunction(renderableWithMaterialsCtor<SelectionCircle>));
        global.setProperty("DecoratedLineRenderable", engine->newFunction(renderableWithMaterialsCtor<DecoratedLineRenderable>));
        global.setProperty("MovementIndicator", engine->newFunction(renderableWithMaterialsCtor<MovementIndicator>));

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
