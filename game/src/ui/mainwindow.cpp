#include <QGraphicsView>
#include <QGLWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QTimer>

#include "ui/mainwindow.h"
#include "ui/gamegraphicsscene.h"
#include "ui/gamegraphicsview.h"
#include "camera.h"
#include "game.h"
#include "glext.h"

// Used to display the memory usage in the title bar
#if defined(Q_OS_WIN32)
#include <Psapi.h>
#endif

namespace EvilTemple {

    inline void dumpgl(const QGLFormat &format)
    {
        qWarning("Depth buffer size: %d", format.depthBufferSize());

    }

    MainWindow::MainWindow(const Game &game, QWidget *parent)
        : QMainWindow(parent),
        game(game),
        scene(new GameGraphicsScene(game, this)),
        view(new GameGraphicsView(game, this))
    {
        setCentralWidget(view);

        QGLFormat format(QGL::DoubleBuffer|QGL::DepthBuffer|QGL::DirectRendering);
        format.setDepthBufferSize(24);

        QGLWidget *viewport = new QGLWidget(format);
        viewport->setMouseTracking(true); // This is important for hover events

        dumpgl(viewport->format());

        view->setViewport(viewport);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setScene(scene);

        connect(game.camera(), SIGNAL(positionChanged()), SLOT(updateTitle()));

        QTimer *timer = new QTimer(this);
        timer->setInterval(1000);
        timer->setSingleShot(false);
        connect(timer, SIGNAL(timeout()), SLOT(updateTitle()));
        timer->start();
    }

    MainWindow::~MainWindow()
    {

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
            scene->toggleConsole();
        }
    }

}
