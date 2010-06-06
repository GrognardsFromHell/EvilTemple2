
#include <GL/glew.h>

#include <QEventLoop>
#include <QGraphicsView>
#include <QGLWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QTimer>
#include <QDir>
#include <QtDeclarative>

#include "mainwindow.h"
#include "game.h"
#include "savegames.h"
#include "gameview.h"
#include "scriptengine.h"
#include "profilerdialog.h"
#include "scene.h"

// Used to display the memory usage in the title bar
#if defined(Q_OS_WIN32)
#include <Psapi.h>
#endif

namespace EvilTemple {


    void loadFont(const QString &filename)
    {
        int handle = QFontDatabase::addApplicationFont(filename);

        if (handle == -1) {
            qWarning("Unable to load font %s.", qPrintable(filename));
            return;
        }

        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(handle);

        qDebug("Loaded font families: %s.", qPrintable(fontFamilies.join(", ")));
    }

    class MainWindowData {
    public:
        MainWindowData(const Game &_game) : game(_game), gameView(0), consoleWidget(0), profilerDialog(0) {}

        const Game &game;
        GameView *gameView;
        QDeclarativeItem *consoleWidget;
        ProfilerDialog *profilerDialog;
    };

    MainWindow *currentMainWindow;

    void consoleMessageHandler(QtMsgType type, const char *message) {
        if (currentMainWindow) {
            currentMainWindow->consoleMessage(type, message);
        }

        fprintf(stderr, "%s\n", message);
        fflush(stderr);
    }

    MainWindow::MainWindow(const Game &game, QWidget *parent)
        : QMainWindow(parent),
        d_ptr(new MainWindowData(game))
    {

        loadFont(":/fonts/5inq_-_Handserif.ttf");
        loadFont(":/fonts/ArtNoveauDecadente.ttf");
        loadFont(":/fonts/Fontin-Bold.ttf");
        loadFont(":/fonts/Fontin-Italic.ttf");
        loadFont(":/fonts/Fontin-Regular.ttf");
        loadFont(":/fonts/Fontin-SmallCaps.ttf");

        // Set up the OpenGL format for our view
        QGLFormat format(QGL::DoubleBuffer|QGL::DepthBuffer|QGL::DirectRendering);
        format.setDepthBufferSize(24);
        format.setSampleBuffers(true);
        format.setSamples(4);

        QGLWidget *glViewport = new QGLWidget(format, this);
        glViewport->makeCurrent();

        if (glewInit() != GLEW_OK) {
            qWarning("GLEWinit failed.");
        }

        while (glGetError() != GL_NO_ERROR) {}

        // Create the actual OpenGL widget
        d_ptr->gameView = new GameView();
        d_ptr->gameView->setViewport(glViewport);
        d_ptr->gameView->setMouseTracking(true);
        setCentralWidget(d_ptr->gameView);

        QScriptEngine *engine = game.scriptEngine()->engine();
        engine->globalObject().setProperty("gameView", engine->newQObject(d_ptr->gameView));

        // Ensure constant updates
        QTimer *animTimer = new QTimer(this);
        animTimer->setInterval(20);
        animTimer->setSingleShot(false);
        connect(animTimer, SIGNAL(timeout()), glViewport, SLOT(repaint()), Qt::DirectConnection);
        animTimer->start();

        QTimer *timer = new QTimer(this);
        timer->setInterval(1000);
        timer->setSingleShot(false);
        connect(timer, SIGNAL(timeout()), SLOT(updateTitle()));
        timer->start();

        // Create the console
        QDeclarativeComponent *component = new QDeclarativeComponent(d_ptr->gameView->uiEngine(), this);
        component->loadUrl(QUrl::fromLocalFile("interface/Console.qml")); // This is asynchronous

        QEventLoop eventLoop;
        while (!component->isReady()) {
            if (component->isError()) {
                qFatal("Error creating console: %s.", qPrintable(component->errorString()));
                break;
            }
            eventLoop.processEvents();
        }

        d_ptr->consoleWidget = qobject_cast<QDeclarativeItem*>(component->create());
        if (!d_ptr->consoleWidget) {
            qFatal("Console widget doesn't inherit from QDeclarativeItem.");
        } else {
            connect(this, SIGNAL(consoleToggled()), d_ptr->consoleWidget, SLOT(toggle()));
            connect(this, SIGNAL(logMessage(QVariant,QVariant)), d_ptr->consoleWidget, SLOT(addMessage(QVariant,QVariant)));
        }

        currentMainWindow = this;
        qInstallMsgHandler(consoleMessageHandler);

        // d_ptr->guiView->rootContext()->setContextProperty("savegames", new SaveGames("C:/", this));
    }

    MainWindow::~MainWindow()
    {
        currentMainWindow = NULL;
        qInstallMsgHandler(NULL);
    }

    void MainWindow::viewStatusChanged(QDeclarativeView::Status status)
    {
        /*if (status == QDeclarativeView::Error) {
            foreach (QDeclarativeError error, d_ptr->guiView->errors()) {
                qWarning("QML Error: %s", qPrintable(error.description()));
            }
        }*/
    }

    void MainWindow::consoleMessage(QtMsgType type, const char *message)
    {
        QVariant varType;
        switch (type) {
        case QtDebugMsg:
            varType = QString("debug");
            break;
        case QtWarningMsg:
            varType = QString("warning");
            break;
        case QtCriticalMsg:
            varType = QString("critical");
            break;
        case QtFatalMsg:
            varType = QString("fatal");
            break;
        }

        QVariant varMessage(QString::fromLocal8Bit(message));

        emit logMessage(varMessage, varType);
    }

    void MainWindow::updateTitle()
    {
        QPoint centeredOn = d_ptr->gameView->screenCenter();

        QString windowTitle = QString("Evil Temple");

        windowTitle.append(QString(" (Drawn: %1)").arg(d_ptr->gameView->objectsDrawn()));

#if defined(Q_OS_WIN32)
        // Open a handle to the current process
        HANDLE hProcess = GetCurrentProcess();

        PROCESS_MEMORY_COUNTERS pmc;

        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            windowTitle.append(QString(" %1 MB").arg(int(pmc.WorkingSetSize / (1024 * 1024))));
        }
#endif

        setWindowTitle(windowTitle);
    }

    void MainWindow::readSettings() {
        QSettings settings;

        settings.beginGroup("MainWindow");

        if (settings.contains("size"))
            resize(settings.value("size").toSize());
        if (settings.contains("pos"))
            move(settings.value("pos").toPoint());
        settings.endGroup();
    }

    void MainWindow::writeSettings() {
        QSettings settings;

        settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.setValue("maximized", isMaximized());
        settings.setValue("fullscreen", isFullScreen());
        settings.endGroup();
    }

    void MainWindow::closeEvent(QCloseEvent *e) {
        writeSettings();
        QMainWindow::closeEvent(e);
    }

    void MainWindow::showFromSettings() {
        QSettings settings;

        settings.beginGroup("MainWindow");
        bool fullscreen = settings.value("fullscreen", true).toBool();
        bool maximized = settings.value("maximized", false).toBool();
        settings.endGroup();

        if (fullscreen) {
            showFullScreen();
        } else if (maximized) {
            showMaximized();
        } else {
            show();
        }

        qobject_cast<QGLWidget*>( d_ptr->gameView->viewport() )->makeCurrent();

        GLenum err = glGetError();
        while (err != GL_NO_ERROR)
            err = glGetError();

        d_ptr->game.scriptEngine()->callGlobalFunction("startup");

        glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO: necessity?

        //d_ptr->guiView->setSource(QUrl("interface/Startup.qml"));

        // TODO: Wait for the loading state here

        // Is it safe to assume that root object is a declarative item?
        //QDeclarativeItem *root = qobject_cast<QDeclarativeItem*>(d_ptr->guiView->rootObject());
        //if (root) {
        //    d_ptr->consoleWidget->setParentItem(root);
        //}
    }

    void MainWindow::keyPressEvent(QKeyEvent *e) {
        if (e->key() == Qt::Key_Return && (e->modifiers() & Qt::AltModifier) == Qt::AltModifier) {
            if (isFullScreen()) {
                hide();
                showMaximized();
            } else {
                hide();
                showFullScreen();
            }
            e->accept();
        } else if (e->key() == Qt::Key_F11) {
            if (d_ptr->profilerDialog) {
                d_ptr->profilerDialog->close();
                delete d_ptr->profilerDialog;
                d_ptr->profilerDialog = 0;
            } else {
                d_ptr->profilerDialog = new ProfilerDialog;
                d_ptr->profilerDialog->show();
            }

        } else if (e->key() == Qt::Key_F12) {
            emit consoleToggled();
        }
    }


}
