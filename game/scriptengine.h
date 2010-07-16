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

    public slots:
        void handleException(const QScriptValue &exception);

        void callGlobalFunction(const QString &name);

        void callGlobalFunction(const QString &name, const QScriptValueList &arguments);

    private:
        void handleUncaughtException();

        QScopedPointer<ScriptEngineData> d;

        Q_DISABLE_COPY(ScriptEngine)
    };

}

#endif // SCRIPTENGINE_H
