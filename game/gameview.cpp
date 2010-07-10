
#include <GL/glew.h>

#include <QtCore/QElapsedTimer>
#include <QtDeclarative/QtDeclarative>
#include <QtGui/QResizeEvent>

#include "gameview.h"

#include "renderstates.h"
#include "modelfile.h"
#include "modelinstance.h"
#include "backgroundmap.h"

#include "clippinggeometry.h"
#include "particlesystem.h"
#include "lighting.h"
#include "lighting_debug.h"

#include "scene.h"
#include "boxrenderable.h"
#include "profiler.h"
#include "materials.h"
#include "translations.h"
#include "audioengine.h"
#include "sectormap.h"
#include "models.h"
#include "imageuploader.h"

#include <QPointer>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

    class VisualTimer {
    public:

        /**
          Constructs a new visual timer, that starts now, which calls the given callback
          after elapseAfter miliseconds.
          */
        VisualTimer(const QScriptValue &value, uint elapseAfter);

        /**
          Returns true if this timer elapsed given a reference time.
          */
        bool isElapsed(qint64 msecsSinceReference) const;

        /**
          Call the callback stored in this visual timer.
          */
        void call();

    private:
        QScriptValue mCallback;
        qint64 mElapseAfter;
    };

    inline VisualTimer::VisualTimer(const QScriptValue &callback, uint elapseAfter)
        : mCallback(callback)
    {
        QElapsedTimer timer;
        timer.start();
        mElapseAfter = timer.msecsSinceReference() + elapseAfter;
    }

    inline bool VisualTimer::isElapsed(qint64 msecsSinceReference) const
    {
        return msecsSinceReference >= mElapseAfter;
    }

    inline void VisualTimer::call()
    {
        mCallback.call();

        QScriptEngine *engine = mCallback.engine();

        if (!engine)
            return;

        if (engine->hasUncaughtException()) {
            qDebug() << engine->uncaughtException().toString() << engine->uncaughtExceptionLineNumber();
        }
    }

    class GameViewData : public AlignedAllocation
    {
    public:
        GameViewData(GameView *view)
            : q(view), rootItem(0), backgroundMap(renderStates),
            clippingGeometry(renderStates), dragging(false), lightDebugger(renderStates),
            materials(renderStates), sectorMap(&scene), models(&materials, renderStates),
            particleSystems(&models, &materials), scene(&materials)
        {
            sceneTimer.invalidate();

            qDebug("Initializing glew...");

            if (glewInit() != GLEW_OK) {
                qWarning("Unable to initialize GLEW.");
            }

            GlobalTextureCache::start();

            if (!particleSystems.loadTemplates()) {
                qWarning("Unable to load particle system templates: %s.", qPrintable(particleSystems.error()));
            }

            if (!translations.load("translation.dat")) {
                qFatal("Unable to load translations.");
            }

            if (!audioEngine.open()) {
                qWarning("Unable to initialize audio engine.");
            }

            // Old: -44
            Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, deg2rad(-44.42700648682643));
            Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

            // Old: 90-135
            Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(135.0000005619373));
            Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

            Matrix4 flipZMatrix;
            flipZMatrix.setToIdentity();
            flipZMatrix(2, 2) = -1;

            Matrix4 id;
            id.setToIdentity();
            id(2,3) = -3000;

            baseViewMatrix = id * flipZMatrix * rotate1matrix * rotate2matrix;

            renderStates.setViewMatrix(baseViewMatrix);
            centerOnWorld(480 * 28.2842703f, 480 * 28.2842703f);

            lightDebugger.loadMaterial();
            Light::setDebugRenderer(&lightDebugger);
        }

        ~GameViewData()
        {
            GlobalTextureCache::stop();
        }

        void centerOnWorld(float worldX, float worldY)
        {
            Matrix4 matrix = Matrix4::translation(-worldX, 0, -worldY);
            renderStates.setViewMatrix(baseViewMatrix * matrix);
        }

        Vector4 worldPositionFromScreen(const QPoint &point) {
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            float width = viewport[2];
            float height = viewport[3];

            // Construct a picking ray
            Vector4 nearPlanePoint(2 * point.x() / width - 1,
                                   2 * (height - point.y()) / height - 1,
                                   0,
                                   1);

            Vector4 farPlanePoint = nearPlanePoint;
            farPlanePoint.setZ(1);

            Matrix4 matrix = renderStates.viewProjectionMatrix().inverted();

            Vector4 pickingRayOrigin = matrix.mapPosition(nearPlanePoint);
            Vector4 pickingRayDirection = (matrix.mapPosition(farPlanePoint) - pickingRayOrigin).normalized();

            // Using the picking ray direction, project the picking ray's origin onto the x,z plane.

            Q_ASSERT(pickingRayDirection.y() < 0); // The assumption is that the picking ray goes *into* the scene

            float t = pickingRayOrigin.y() / pickingRayDirection.y();

            return pickingRayOrigin - t * pickingRayDirection;
        }

        Renderable *pickObject(const QPoint &point)
        {
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            float width = viewport[2];
            float height = viewport[3];

            // Construct a picking ray
            Vector4 nearPlanePoint(2 * point.x() / width - 1,
                                   2 * (height - point.y()) / height - 1,
                                   0,
                                   1);

            Vector4 farPlanePoint = nearPlanePoint;
            farPlanePoint.setZ(1);

            Matrix4 matrix = renderStates.viewProjectionMatrix().inverted();

            Vector4 pickingRayOrigin = matrix.mapPosition(nearPlanePoint);
            Vector4 pickingRayDirection = (matrix.mapPosition(farPlanePoint) - pickingRayOrigin).normalized();

            Ray3d pickingRay(pickingRayOrigin, pickingRayDirection);
            return scene.pickRenderable(pickingRay);
        }

        QDeclarativeEngine uiEngine;
        QGraphicsScene uiScene;
        QDeclarativeItem* rootItem;

        QSize viewportSize;

        RenderStates renderStates;

        Materials materials;

        bool dragging;
        QPoint lastPoint;
        bool mouseMovedDuringDrag;
        Matrix4 baseViewMatrix; // Without translations

        typedef QList<VisualTimer> VisualTimers;
        VisualTimers visualTimers;

        QPointer<Renderable> lastMouseOverRenderable;

        LightDebugRenderer lightDebugger;

        BackgroundMap backgroundMap;

        ClippingGeometry clippingGeometry;

        Translations translations;

        AudioEngine audioEngine;

        SectorMap sectorMap;

        Models models;

        ParticleSystems particleSystems;

        QElapsedTimer sceneTimer;
        Scene scene;

        void resize(int width, int height) {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;
            glViewport(0, 0, width, height);

            viewportSize.setWidth(width);
            viewportSize.setHeight(height);

            const float zoom = 1;

            Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth / zoom, halfWidth / zoom, -halfHeight / zoom, halfHeight / zoom, 1, 5000);
            renderStates.setProjectionMatrix(projectionMatrix);
        }

        void pollVisualTimers();

    private:
        GameView *q;
    };

    void GameViewData::pollVisualTimers()
    {
        QElapsedTimer timer;
        timer.start();
        qint64 reference = timer.msecsSinceReference();

        for (int i = 0; i < visualTimers.size(); ++i) {
            VisualTimer timer = visualTimers[i];

            if (timer.isElapsed(reference)) {
                // qDebug("Timer has elapsed.");
                timer.call();
                visualTimers.removeAt(i--);
            }
        }
    }

    GameView::GameView(QWidget *parent) :
            QGraphicsView(parent), d(new GameViewData(this))
    {
        QPixmap defaultCursor("art/interface/cursors/MainCursor.png");
        setCursor(QCursor(defaultCursor, 2, 2));

        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);
        setCacheMode(CacheNone);
        setFrameStyle(0);
        setScene(&d->uiScene);

        // Sets several Qt Quick related options
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        d->uiScene.setItemIndexMethod(QGraphicsScene::NoIndex);
        viewport()->setFocusPolicy(Qt::NoFocus);
        setFocusPolicy(Qt::StrongFocus);

        QUrl baseUrl = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "data" + QDir::separator());
        d->uiEngine.setBaseUrl(baseUrl);

        d->uiScene.setStickyFocus(true);

        d->uiEngine.rootContext()->setContextProperty("gameView", this);
        d->uiEngine.rootContext()->setContextProperty("translations", &d->translations);
        d->uiEngine.rootContext()->setContextProperty("imageUploader", new ImageUploader);

        setMouseTracking(true);
    }

    GameView::~GameView()
    {
    }

    void GameView::drawBackground(QPainter *painter, const QRectF &rect)
    {
        Q_UNUSED(painter);
        Q_UNUSED(rect);

        Profiler::newFrame();

        SAFE_GL(;); // Clears existing OpenGL errors

        // Evaluate visual script timers
        d->pollVisualTimers();

        SAFE_GL(glEnable(GL_MULTISAMPLE));

        SAFE_GL(glEnable(GL_DEPTH_TEST));
        SAFE_GL(glEnable(GL_CULL_FACE));
        SAFE_GL(glEnable(GL_BLEND));
        SAFE_GL(glDisable(GL_STENCIL_TEST));

        glClearColor(0, 0, 0, 0);
        glClearStencil(1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        d->backgroundMap.render();

        if (!d->sceneTimer.isValid()) {
            d->sceneTimer.start();
        } else {
            qint64 elapsedMs = d->sceneTimer.restart();
            double elapsedSeconds = elapsedMs / 1000.0;

            // A single frame may never advance time more than 1/10th of a second
            if (elapsedSeconds > 0.1)
                elapsedSeconds = 0.1;

            float texAnimTime = d->renderStates.textureAnimationTime() + elapsedSeconds;
            while (texAnimTime > 60)
                texAnimTime -= 60;

            d->renderStates.setTextureAnimationTime(texAnimTime);

            d->scene.elapseTime(elapsedSeconds);
        }

        d->scene.render(d->renderStates);

        SAFE_GL(glDisable(GL_CULL_FACE));
        SAFE_GL(glDisable(GL_DEPTH_TEST));
        SAFE_GL(glDisable(GL_TEXTURE_2D));
        SAFE_GL(glDisable(GL_LIGHTING));

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(d->renderStates.projectionMatrix().data());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(d->renderStates.viewMatrix()(0, 3), d->renderStates.viewMatrix()(1, 3), 0);

        // Draw a line-stippled version of the bounding box
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
        glVertex3f(mScrollBoxMinX, mScrollBoxMinY, -1);
        glVertex3f(mScrollBoxMaxX, mScrollBoxMinY, -1);
        glVertex3f(mScrollBoxMaxX, mScrollBoxMaxY, -1);
        glVertex3f(mScrollBoxMinX, mScrollBoxMaxY, -1);
        glEnd();

        glClear(GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        SAFE_GL(glDisable(GL_MULTISAMPLE));
    }

    QObject *GameView::showView(const QString &url)
    {
        QDeclarativeItem *widget = qobject_cast<QDeclarativeItem*>(addGuiItem(url));

        d->rootItem = widget;
        widget->setWidth(width());
        widget->setHeight(height());

        return widget;
    }

    QObject *GameView::addGuiItem(const QString &url)
    {
        // Create the console
        QDeclarativeComponent *component = new QDeclarativeComponent(&d->uiEngine, this);
        component->loadUrl(QUrl::fromLocalFile(url));

        QEventLoop eventLoop;
        while (!component->isReady()) {
            if (component->isError()) {
                qFatal("Error creating widget: %s.", qPrintable(component->errorString()));
                break;
            }
            eventLoop.processEvents();
        }

        QDeclarativeItem *widget = qobject_cast<QDeclarativeItem*>(component->create());

        d->uiScene.addItem(widget);

        return widget;
    }

    QDeclarativeEngine *GameView::uiEngine()
    {
        return &d->uiEngine;
    }

    void GameView::resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);

        if (event->size() == d->viewportSize)
            return;

        // Update projection matrix
        d->resize(event->size().width(), event->size().height());

        emit viewportChanged();
    }

    void GameView::mouseMoveEvent(QMouseEvent *evt)
    {
        QGraphicsView::mouseMoveEvent(evt);

        if (d->dragging) {
            d->mouseMovedDuringDrag = true;

            int diffX = evt->pos().x() - d->lastPoint.x();
            int diffY = evt->pos().y() - d->lastPoint.y();

            Vector4 diff(diffX, -diffY, 0, 0);

            Matrix4 transform = d->renderStates.viewMatrix().transposed();
            // Clear last column
            transform(3, 0) = 0;
            transform(3, 1) = 0;
            transform(3, 2) = 0;
            transform(3, 3) = 1;

            // This should now be the inverse of the funky view matrix.
            diff = transform * diff;

            transform.setToIdentity();
            transform(0,3) = diff.x() - diff.y();
            transform(2,3) = diff.z() - diff.y();
            // TODO: We should instead project it to the x,z plane.

            Matrix4 viewMatrix = d->renderStates.viewMatrix();
            d->renderStates.setViewMatrix(viewMatrix * transform);
            evt->accept();
        } else {
            Renderable *renderable = d->pickObject(evt->pos());
            Renderable *lastMouseOver = d->lastMouseOverRenderable;

            if (renderable != lastMouseOver) {
                if (lastMouseOver)
                    lastMouseOver->mouseLeaveEvent(evt);
                if (renderable)
                    renderable->mouseEnterEvent(evt);
                d->lastMouseOverRenderable = renderable;
            }
        }

        d->lastPoint = evt->pos();
    }

    void GameView::mousePressEvent(QMouseEvent *evt)
    {
        QGraphicsView::mousePressEvent(evt);

        if (!evt->isAccepted()) {
            d->dragging = true;
            d->mouseMovedDuringDrag = false;
            evt->accept();
            d->lastPoint = evt->pos();
        }
    }

    void GameView::mouseReleaseEvent(QMouseEvent *evt)
    {
        if (d->dragging && !d->mouseMovedDuringDrag) {
            Renderable *renderable = d->pickObject(evt->pos());

            if (renderable) {
                renderable->mouseReleaseEvent(evt);
            } else {
                Vector4 worldPosition = d->worldPositionFromScreen(evt->pos());
                emit worldClicked(evt->button(), evt->buttons(), worldPosition);
            }
        }
        d->dragging = false;
        QGraphicsView::mouseReleaseEvent(evt);
    }

    void GameView::mouseDoubleClickEvent(QMouseEvent *evt)
    {
        // Sadly it's not really possible to detect, whether the double click was
        // really accepted by the scene. Instead we will simply check if there
        // is a QGraphicsItem at the clicked position.
        if (d->uiScene.itemAt(evt->posF())) {
            QGraphicsView::mouseDoubleClickEvent(evt);
        } else {
            qDebug("Mouse double click event.");

            Renderable *renderable = d->pickObject(evt->pos());

            if (renderable) {
                renderable->mouseDoubleClickEvent(evt);
            } else {
                Vector4 worldPosition = d->worldPositionFromScreen(evt->pos());
                emit worldDoubleClicked(evt->button(), evt->buttons(), worldPosition);
            }
        }
    }

    QPoint GameView::screenCenter() const
    {
        Vector4 viewCenter(0,0,0,1);
        Vector4 viewDirection(0,0,1,0);
        viewCenter = d->renderStates.viewMatrix().inverted() * viewCenter;
        viewDirection = d->renderStates.viewMatrix().inverted() * viewDirection;

        viewCenter *= 1 / viewCenter.w();

        return QPoint(viewCenter.x(), viewCenter.z());
    }

    void GameView::centerOnWorld(float worldX, float worldY)
    {
        d->centerOnWorld(worldX, worldY);
    }

    int GameView::objectsDrawn() const
    {
        return d->scene.objectsDrawn();
    }

    Scene *GameView::scene() const
    {
        return &d->scene;
    }

    BackgroundMap *GameView::backgroundMap() const
    {
        return &d->backgroundMap;
    }

    ClippingGeometry *GameView::clippingGeometry() const
    {
        return &d->clippingGeometry;
    }

    Materials *GameView::materials() const
    {
        return &d->materials;
    }

    ParticleSystems* GameView::particleSystems() const
    {
        return &d->particleSystems;
    }

    const QSize &GameView::viewportSize() const
    {
        return d->viewportSize;
    }

    Translations *GameView::translations() const
    {
        return &d->translations;
    }

    AudioEngine *GameView::audioEngine() const
    {
        return &d->audioEngine;
    }

    SectorMap *GameView::sectorMap() const
    {
        return &d->sectorMap;
    }

    Models *GameView::models() const
    {
        return &d->models;
    }

    void GameView::addVisualTimer(uint elapseAfter, const QScriptValue &callback)
    {
        // qDebug("Adding visual timer that'll elapse after %d ms.", elapseAfter);
        d->visualTimers.append(VisualTimer(callback, elapseAfter));
    }

    QUrl GameView::takeScreenshot()
    {
        QGLWidget *widget = qobject_cast<QGLWidget*>(viewport());
        Q_ASSERT(widget);

        QImage screenshot = widget->grabFrameBuffer();

        QDir currentDir = QDir::current();
        if (!currentDir.exists("screenshots"))
            currentDir.mkdir("screenshots");

        QDateTime now = QDateTime::currentDateTime();

        QString currentDateTime = "screenshot-" + now.toString("yyyy-MM-dd-hh-mm-ss");
        uint suffix = 0;
        QString currentFilename = "screenshots/" + currentDateTime + ".jpg";

        while (currentDir.exists(currentFilename)) {
            suffix++;
            currentFilename = QString("screenshots/%1-%2.jpg").arg(currentDateTime).arg(suffix);
        }

        screenshot.save(currentFilename, "jpg", 85);

        return QUrl::fromLocalFile(currentDir.absoluteFilePath(currentFilename));
    }

    QString GameView::readBase64(const QUrl &filename)
    {
        QFile file(filename.toLocalFile());

        if (!file.open(QIODevice::ReadOnly)) {
            return QByteArray();
        }

        QByteArray content = file.readAll();

        file.close();

        QString result = QString::fromLatin1(content.toBase64());

        return result;
    }

}
