
#include <GL/glew.h>

#include <QCoreApplication>
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
#include "gameview.h"
#include "scriptengine.h"
#include "profilerdialog.h"
#include "scene.h"

#include "materialstate.h"
#include "modelinstance.h"
#include "texture.h"
#include "navigationmesh.h"
#include "renderable.h"

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
        MainWindowData(const Game &_game) : game(_game), gameView(0), profilerDialog(0) {}

        const Game &game;
        GameView *gameView;
        ProfilerDialog *profilerDialog;
    };

    MainWindow::MainWindow(const Game &game, QWidget *parent)
        : QMainWindow(parent),
        d_ptr(new MainWindowData(game))
    {
        setWindowIcon(QIcon(":/images/application.ico"));

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
        engine->globalObject().setProperty("translations", engine->newQObject(d_ptr->gameView->translations()));

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
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::viewStatusChanged(QDeclarativeView::Status status)
    {
        /*if (status == QDeclarativeView::Error) {
            foreach (QDeclarativeError error, d_ptr->guiView->errors()) {
                qWarning("QML Error: %s", qPrintable(error.description()));
            }
        }*/
    }

    void MainWindow::updateTitle()
    {
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

        windowTitle.append(QString(" Models: %1").arg(getActiveModels()));
        windowTitle.append(QString(" Textures: %1").arg(getActiveTextures()));
        windowTitle.append(QString(" Mat-States: %1").arg(getActiveMaterialStates()));
        windowTitle.append(QString(" NavMeshes: %1").arg(getActiveNavigationMeshes()));
        windowTitle.append(QString(" Renderables: %1").arg(getActiveRenderables()));

        Vector4 worldPosition = d_ptr->gameView->worldCenter();
        windowTitle.append(QString(" (%1,%2,%3)").arg((int)worldPosition.x())
                           .arg((int)worldPosition.y())
                           .arg((int)worldPosition.z()));

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
        } else {
            QScriptValue scriptEvent = d_ptr->game.scriptEngine()->engine()->newObject();
            scriptEvent.setProperty("key", QScriptValue(e->key()));
            scriptEvent.setProperty("text", QScriptValue(e->text()));

            QScriptValueList args;
            args << scriptEvent;

            qDebug("Delegating shortcut to JavaScript.");
            d_ptr->game.scriptEngine()->callGlobalFunction("keyPressed", args);
        }
    }


}
