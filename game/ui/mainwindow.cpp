#include <QGraphicsView>
#include <QGLWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QTimer>
#include <QDir>
#include <QtDeclarative>

#include "ui/mainwindow.h"
#include "ui/gamegraphicsscene.h"
#include "ui/gamegraphicsview.h"
#include "camera.h"
#include "game.h"
#include "glext.h"
#include "savegames.h"

// Used to display the memory usage in the title bar
#if defined(Q_OS_WIN32)
#include <Psapi.h>
#endif

namespace EvilTemple {

    inline void dumpgl(const QGLFormat &format)
    {
        qWarning("Depth buffer size: %d", format.depthBufferSize());
    }

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
        game(game),
        scene(new GameGraphicsScene(game, this)),
        // view(new GameGraphicsView(game, this))
        view(new QDeclarativeView(this))
    {
        setCentralWidget(view);

        QGLFormat format(QGL::DoubleBuffer|QGL::DepthBuffer|QGL::DirectRendering);
        format.setDepthBufferSize(24);

        QGLWidget *viewport = new QGLWidget(format);
        viewport->setMouseTracking(true); // This is important for hover events

        view->setViewport(viewport);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
        connect(view, SIGNAL(statusChanged(QDeclarativeView::Status)),
                SLOT(viewStatusChanged(QDeclarativeView::Status)));
        //view->setScene(scene);
        QUrl baseUrl = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "data" + QDir::separator());
        view->engine()->setBaseUrl(baseUrl);

        connect(game.camera(), SIGNAL(positionChanged()), SLOT(updateTitle()));

        QTimer *timer = new QTimer(this);
        timer->setInterval(1000);
        timer->setSingleShot(false);
        connect(timer, SIGNAL(timeout()), SLOT(updateTitle()));
        timer->start();

        // Create the console
        QDeclarativeComponent *component = new QDeclarativeComponent(view->engine(), this);
        component->loadUrl(QUrl("interface/Console.qml"));
        consoleWidget = qobject_cast<QDeclarativeItem*>(component->create());
        if (!consoleWidget) {
            qWarning("Console widget doesn't inherit from QDeclarativeItem.");
        } else {
            connect(this, SIGNAL(consoleToggled()), consoleWidget, SLOT(toggle()));
        }

        connect(this, SIGNAL(logMessage(QVariant,QVariant)), consoleWidget, SLOT(addMessage(QVariant,QVariant)));
        currentMainWindow = this;
        qInstallMsgHandler(consoleMessageHandler);

        dumpgl(viewport->format());

        view->rootContext()->setContextProperty("savegames", new SaveGames("C:/", this));
    }

    MainWindow::~MainWindow()
    {
        currentMainWindow = NULL;
        qInstallMsgHandler(NULL);
    }


    void MainWindow::viewStatusChanged(QDeclarativeView::Status status)
    {
        if (status == QDeclarativeView::Error) {
            foreach (QDeclarativeError error, view->errors()) {
                qWarning("QML Error: %s", qPrintable(error.description()));
            }
        }
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
        QVector2D centeredOn = game.camera()->centeredOn();
        QString windowTitle = QString("EvilTemple (Pos: %1,%2 | Objects: %3) %4 fps").arg((int)centeredOn.x())
                               .arg((int)centeredOn.y())
                               .arg(scene->objectsDrawn())
                               .arg((int)scene->fps());

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
        e->accept();
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

        view->setSource(QUrl("interface/Startup.qml"));

        // TODO: Wait for the loading state here

        // Is it safe to assume that root object is a declarative item?
        QDeclarativeItem *root = qobject_cast<QDeclarativeItem*>(view->rootObject());
        if (root) {
            consoleWidget->setParentItem(root);
        }
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
        } else if (e->key() == Qt::Key_F12) {
            emit consoleToggled();
        }
    }

}
