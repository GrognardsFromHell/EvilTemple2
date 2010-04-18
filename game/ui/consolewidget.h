#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include "gameglobal.h"

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QKeyEvent>

namespace EvilTemple {

    class Game;

    class GAME_EXPORT ConsoleLineEdit : public QLineEdit
    {
        Q_OBJECT
    public:
        explicit ConsoleLineEdit(QWidget *parent = 0);
        ~ConsoleLineEdit();

    protected:
        void keyPressEvent(QKeyEvent *event);

    private:
        QStringList history;
        int historyPosition;
    };

    class GAME_EXPORT ConsoleWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit ConsoleWidget(const Game &game, QWidget *parent = 0);
        ~ConsoleWidget();

        void addQtMessage(QtMsgType type, const char *message);

    signals:

    public slots:

    protected slots:
        void performCommand();
        void scrollDown();

    protected:
         void paintEvent(QPaintEvent *event);
         void resizeEvent(QResizeEvent *);
         void showEvent(QShowEvent *);

    private:
        const Game &game;
        QColor background;
        QColor border;
        ConsoleLineEdit *inputField;
        QTextEdit *log;
    };

}

#endif // CONSOLEWIDGET_H
