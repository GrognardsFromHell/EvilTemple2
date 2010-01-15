#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QScriptEngine>

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

}

#endif // SCRIPTENGINE_H
