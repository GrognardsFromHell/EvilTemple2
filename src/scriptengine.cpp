
#include <QtScript/QtScript>
#include <QVector2D>

#include "scriptengine.h"
#include "game.h"

namespace EvilTemple {



    static QScriptValue constructQVector2D(QScriptContext *context, QScriptEngine *engine)
    {
        if (!context->isCalledAsConstructor())
            return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator.");

        if (context->argumentCount() != 2) {
            return context->throwError(QScriptContext::SyntaxError, "takes 2 arguments: x, y");
        }

        qsreal x = context->argument(0).toNumber();
        qsreal y = context->argument(1).toNumber();

        return engine->newVariant(context->thisObject(), qVariantFromValue( QVector2D(x, y) ) );
    }

    QScriptValue qvector2d_x(QScriptContext *context, QScriptEngine *)
    {
        QVector2D vector = qscriptvalue_cast<QVector2D>(context->thisObject());
        return vector.x();
    }

    QScriptValue qvector2d_y(QScriptContext *context, QScriptEngine *)
    {
        QVector2D vector = qscriptvalue_cast<QVector2D>(context->thisObject());
        return vector.y();
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
            QScriptValue qvector2dPrototype = engine->newObject();
            qvector2dPrototype.setProperty("x", engine->newFunction(qvector2d_x));
            qvector2dPrototype.setProperty("y", engine->newFunction(qvector2d_y));
            QScriptValue qvector2d_ctor = engine->newFunction(constructQVector2D, qvector2dPrototype);
            global.setProperty("QVector2D", qvector2d_ctor);
            engine->setDefaultPrototype(qMetaTypeId<QVector2D>(), qvector2dPrototype);
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
