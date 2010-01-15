#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>

namespace EvilTemple {

    class Game;

    class ConsoleWidget : public QWidget
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

        QLineEdit *inputField;
        QTextEdit *log;
    };

}

#endif // CONSOLEWIDGET_H
