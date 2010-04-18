#include <QPainter>
#include <QResizeEvent>
#include <QResource>

#include "ui/consolewidget.h"
#include "game.h"
#include "scriptengine.h"

namespace EvilTemple {

    ConsoleLineEdit::ConsoleLineEdit(QWidget *parent) : QLineEdit(parent), historyPosition(-1) {
    }

    ConsoleLineEdit::~ConsoleLineEdit() {
    }

    void ConsoleLineEdit::keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Up) {
            historyPosition = qMin(history.size(), historyPosition + 1);
            setText(history[historyPosition]);
            event->accept();
        } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            historyPosition = -1;
            QString command(text());
            if (!command.isEmpty())
                history.prepend(command);
        } else {
            historyPosition = -1;
            QLineEdit::keyPressEvent(event);
        }
    }

    static ConsoleWidget *outputWidget = NULL;

    static void consoleOutput(QtMsgType type, const char *msg)
    {
        // Still route the message to the console. In debug mode we still have a text mode console
        fprintf(stderr, "%s\n", msg);
        fflush(stderr);

        if (!outputWidget)
            return;

        outputWidget->addQtMessage(type, msg);
    }

    ConsoleWidget::ConsoleWidget(const Game &_game, QWidget *parent) :
            QWidget(parent),
            game(_game),
            background(0, 0, 0, 128),
            border(0, 0, 127, 128),
            inputField(new ConsoleLineEdit(this)),
            log(new QTextEdit(this))
    {
        setAttribute(Qt::WA_OpaquePaintEvent, true);

        QResource stylesheet("/console/style.css");
        if (stylesheet.isValid()) {
            setStyleSheet(reinterpret_cast<const char*>(stylesheet.data()));
        }

        log->setReadOnly(true);
        connect(log, SIGNAL(textChanged()), SLOT(scrollDown()));

        connect(inputField, SIGNAL(returnPressed()), SLOT(performCommand()));

        outputWidget = this;
        qInstallMsgHandler(consoleOutput);
    }

    ConsoleWidget::~ConsoleWidget()
    {
        qInstallMsgHandler(0);
        outputWidget = NULL;
    }

    void ConsoleWidget::addQtMessage(QtMsgType type, const char *message)
    {
        QTextCursor cursor(log->document());
        cursor.movePosition(QTextCursor::End);

        QTextCharFormat old = cursor.charFormat();
        QTextCharFormat bold(old);
        bold.setFontWeight(QFont::Bold);

        cursor.beginEditBlock();
        cursor.insertBlock();
        cursor.setCharFormat(bold);

        switch (type) {
        case QtDebugMsg:
            break;
        case QtWarningMsg:
            bold.setForeground(QColor(255, 255, 0));
            cursor.setCharFormat(bold);
            cursor.insertText("WARNING: ");
            break;
        case QtCriticalMsg:
            bold.setForeground(QColor(255, 0, 0));
            cursor.setCharFormat(bold);
            cursor.insertText("CRITICAL: ");
            break;
        case QtFatalMsg:
            bold.setForeground(QColor(255, 0, 0));
            cursor.setCharFormat(bold);
            cursor.insertText("FATAL: ");
            break;
        }

        cursor.setCharFormat(old);
        cursor.insertText(message);
        cursor.endEditBlock();
    }

    void ConsoleWidget::performCommand()
    {
        QString command = inputField->text();
        inputField->clear();

        QScriptValue result = game.scriptEngine()->engine()->evaluate(command, "console");
        log->append(result.toString());
    }

    void ConsoleWidget::scrollDown()
    {
        log->scroll(0, 999999);
    }

    void ConsoleWidget::paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.fillRect(0, 0, width(), height(), background);

        painter.setPen(QPen(border, 3));
        QPointF left(0, height() - 1);
        QPointF right(width() - 1, height() - 1);
        painter.drawLine(left, right);
    }

    void ConsoleWidget::resizeEvent(QResizeEvent *event)
    {
        inputField->resize(event->size().width(), inputField->height());
        inputField->move(0, event->size().height() - inputField->height());

        log->resize(event->size().width(), event->size().height() - inputField->height());
    }

    void ConsoleWidget::showEvent(QShowEvent *)
    {
        inputField->setFocus(Qt::PopupFocusReason);
    }

}
