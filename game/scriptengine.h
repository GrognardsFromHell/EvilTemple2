#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "gameglobal.h"

#include <QObject>
#include <QtScript/QtScript>

namespace EvilTemple {
    class Game;
    class ScriptEngineData;

    class GAME_EXPORT ScriptEngine : public QObject
    {
    Q_OBJECT
    public:
        explicit ScriptEngine(Game *parent = 0);
        ~ScriptEngine();

        /**
          Loads user scripts from the scripts directory.
          */
        bool loadScripts();

        QScriptEngine *engine() const;

        template<typename T>
        void exposeQObject(const QString &name, T *object);

    public slots:
        void handleException(const QScriptValue &exception);

        void callGlobalFunction(const QString &name);

        void callGlobalFunction(const QString &name, const QScriptValueList &arguments);

    private:
        void handleUncaughtException();

        QScopedPointer<ScriptEngineData> d;

        Q_DISABLE_COPY(ScriptEngine)
    };

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
    void ScriptEngine::exposeQObject(const QString &name, T *object)
    {
        qScriptRegisterMetaType(engine(), qobjectToScriptValue<T>, qobjectFromScriptValue<T>);
        engine()->globalObject().setProperty(name, engine()->newQObject(object));
    }

}

#endif // SCRIPTENGINE_H
