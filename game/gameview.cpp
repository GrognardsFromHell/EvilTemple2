
#include <GL/glew.h>

#include <QDesktopServices>
#include <QtCore/QElapsedTimer>
#include <QtDeclarative/QtDeclarative>
#include <QtGui/QResizeEvent>
#include <QtOpenGL/QGLContext>
#include <QtGui/QImage>

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
#include "binkplayer.h"

#include <QPointer>

#include <gamemath.h>
using namespace GameMath;

#ifdef Q_OS_WIN32
#include <windows.h>
static bool initialized = false;
LARGE_INTEGER frequency;

static qint64 getTicks() {
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (now.QuadPart * 1000) / frequency.QuadPart;
}
#endif

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

        qint64 elapsedSinceStart(qint64 msecsSinceReference) const;

        /**
          Call the callback stored in this visual timer.
          */
        void call(qint64 reference);

    private:
        QScriptValue mCallback;
        qint64 mElapseAfter;
        qint64 mStarted;
    };

    inline VisualTimer::VisualTimer(const QScriptValue &callback, uint elapseAfter)
        : mCallback(callback)
    {
#if defined(Q_OS_WIN32)
        mElapseAfter = getTicks() + elapseAfter;
        mStarted = getTicks();
#else
        QElapsedTimer timer;
        timer.start();
        mStarted = timer.msecsSinceReference();
        mElapseAfter = timer.msecsSinceReference() + elapseAfter;
#endif
    }

    inline qint64 VisualTimer::elapsedSinceStart(qint64 msecsSinceReference) const
    {
        return msecsSinceReference - mStarted;
    }

    inline bool VisualTimer::isElapsed(qint64 msecsSinceReference) const
    {
        return msecsSinceReference >= mElapseAfter;
    }

    inline void VisualTimer::call(qint64 reference)
    {
        int realElapsed = (int)elapsedSinceStart(reference);

        mCallback.call(QScriptValue(), QScriptValueList() << QScriptValue(realElapsed));

        QScriptEngine *engine = mCallback.engine();

        if (!engine)
            return;

        if (engine->hasUncaughtException()) {
            qDebug() << engine->uncaughtException().toString() << engine->uncaughtExceptionLineNumber();
            foreach (const QString &line, engine->uncaughtExceptionBacktrace()) {
                qDebug() << "   " << line;
            }

        }
    }

    class VideoPlayerThread : public QThread
    {
    Q_OBJECT
    public:
        VideoPlayerThread(BinkPlayer *player) : mPlayer(player) {}
    protected:
        void run();
        BinkPlayer *mPlayer;
    };

    class GameViewData : public QObject, public AlignedAllocation
    {
    Q_OBJECT
    public:
        GameViewData(GameView *view)
            : q(view), backgroundMap(renderStates),
            clippingGeometry(renderStates), dragging(false), lightDebugger(renderStates),
            materials(renderStates), sectorMap(&scene), models(&materials, renderStates),
            particleSystems(&models, &materials), scene(&materials), lastAudioEnginePosition(0, 0, 0, 1),
            scrollingDisabled(false),
            mPlayingVideo(false),
            mVideoPlayerThread(&mVideoPlayer)
        {
            connect(&uiEngine, SIGNAL(warnings(QList<QDeclarativeError>)), SLOT(uiWarnings(QList<QDeclarativeError>)));

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

            audioEngine.setVolume(0.25f);

            // Set some properties on the audio engine relating to positional audio
            audioEngine.setListenerOrientation(Vector4(-1, 0, -1, 0).normalized(),
                                               Vector4(0, 1, 0, 0));

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

            lightDebugger.loadMaterial();
            Light::setDebugRenderer(&lightDebugger);

            connect(&mVideoPlayer, SIGNAL(videoFrame(QImage)), SLOT(updateVideoFrame(QImage)));
            connect(&mVideoPlayerThread, SIGNAL(finished()), SLOT(stoppedPlayingVideo()));
        }

        ~GameViewData()
        {
            GlobalTextureCache::stop();
        }

        void updateListenerPosition()
        {
            Vector4 worldCenter = q->worldCenter();
            if (!(worldCenter == lastAudioEnginePosition)) {
                lastAudioEnginePosition = worldCenter;
                audioEngine.setListenerPosition(worldCenter);
            }
        }

        void centerOnWorld(float worldX, float worldY)
        {
            Matrix4 matrix = Matrix4::translation(-worldX, 0, -worldY);
            renderStates.setViewMatrix(baseViewMatrix * matrix);

            audioEngine.setListenerPosition(Vector4(worldX, 0, worldY, 1));
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

        Ray3d getPickingRay(float x, float y)
        {
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            float width = viewport[2];
            float height = viewport[3];

            // Construct a picking ray
            Vector4 nearPlanePoint(2 * x / width - 1,
                                   2 * (height - y) / height - 1,
                                   0,
                                   1);

            Vector4 farPlanePoint = nearPlanePoint;
            farPlanePoint.setZ(1);

            Matrix4 matrix = renderStates.viewProjectionMatrix().inverted();

            Vector4 pickingRayOrigin = matrix.mapPosition(nearPlanePoint);
            Vector4 pickingRayDirection = (matrix.mapPosition(farPlanePoint) - pickingRayOrigin).normalized();

            return Ray3d(pickingRayOrigin, pickingRayDirection);
        }

        Vector4 getWorldCenter()
        {
            Ray3d pickingRay = getPickingRay(viewportSize.width() * 0.5f, viewportSize.height() * 0.5f);

            // Get intersection with x,z plane
            if (qFuzzyIsNull(pickingRay.direction().y())) {
                // In this case, the ray is parallel to the x,z axis
                return Vector4(0, 0, 0, 0);
            }

            float d = pickingRay.origin().y() / pickingRay.direction().y();

            return pickingRay.origin() - d * pickingRay.direction();
        }

        Renderable *pickObject(const QPoint &point)
        {
            return scene.pickRenderable(getPickingRay(point.x(), point.y()));
        }

        bool scrollingDisabled;

        QImage mCurrentVideoFrame;
        bool mPlayingVideo;
        BinkPlayer mVideoPlayer;
        VideoPlayerThread mVideoPlayerThread;
        QScriptValue mVideoFinishedCallback;
        QGraphicsScene movieEmptyScene; // We switch to this scene for playback
        QString cursorFilename;

        QDeclarativeEngine uiEngine;
        QGraphicsScene uiScene;
        QPointer<QDeclarativeItem> rootItem;

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

        Vector4 lastAudioEnginePosition;

        void playVideo(const QString &video);

        void resize(int width, int height) {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;
            glViewport(0, 0, width, height);

            viewportSize.setWidth(width);
            viewportSize.setHeight(height);

            uiScene.setSceneRect(0, 0, width, height);
            if (rootItem) {
                rootItem->setWidth(width);
                rootItem->setHeight(height);
            }

            const float zoom = 1;

            Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth / zoom, halfWidth / zoom, -halfHeight / zoom, halfHeight / zoom, 1, 5000);
            renderStates.setProjectionMatrix(projectionMatrix);
        }

        void pollVisualTimers();

    public slots:

        void uiWarnings(const QList<QDeclarativeError> &warnings)
        {
            foreach (const QDeclarativeError &error, warnings) {
                qWarning("%s", qPrintable(error.toString()));
            }
        }

        void updateVideoFrame(const QImage &image)
        {
            mCurrentVideoFrame = image;
        }

        void stoppedPlayingVideo()
        {
            qDebug("Finished playing video. Thread was stopped.");

            q->setScene(&uiScene);
            mPlayingVideo = false;
            mVideoPlayer.close();
            mCurrentVideoFrame = QImage();
            if (mVideoFinishedCallback.isValid()) {
                mVideoFinishedCallback.call();
                mVideoFinishedCallback = QScriptValue();
            }
        }

    private:
        GameView *q;
    };

    void VideoPlayerThread::run()
    {
        mPlayer->play();
    }

    void GameViewData::pollVisualTimers()
    {
#ifdef Q_OS_WIN32
        qint64 reference = getTicks();
#else
        QElapsedTimer timer;
        timer.start();
        qint64 reference = timer.msecsSinceReference();
#endif

        for (int i = 0; i < visualTimers.size(); ++i) {
            VisualTimer timer = visualTimers[i];

            if (timer.isElapsed(reference)) {
                // qDebug("Timer has elapsed.");
                timer.call(reference);
                visualTimers.removeAt(i--);
            }
        }
    }

    GameView::GameView(QWidget *parent) :
            QGraphicsView(parent), d(new GameViewData(this)), mScrollingBorder(0)
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

        while (glGetError() != GL_NO_ERROR);

        if (d->mPlayingVideo) {

            if (d->mCurrentVideoFrame.isNull())
                return;

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());
            Q_ASSERT(context);

            glEnable(GL_TEXTURE_2D);
            GLuint texture = context->bindTexture(d->mCurrentVideoFrame, GL_TEXTURE_2D, GL_RGBA, QGLContext::NoBindOption);

            glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(-1, -1);
            glTexCoord2f(0, 0);
            glVertex2i(-1, 1);
            glTexCoord2f(1, 0);
            glVertex2i(1, 1);
            glTexCoord2f(1, 1);
            glVertex2i(1, -1);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            context->deleteTexture(texture);

            return;
        }

        // Update the audio engine state if necessary
        d->updateListenerPosition();

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
        if (d->mPlayingVideo)
            return;

        QGraphicsView::mouseMoveEvent(evt);

        if (d->dragging) {
            d->mouseMovedDuringDrag = true;

            if (!isScrollingDisabled()) {
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
            }
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
        if (d->mPlayingVideo) {
            d->mVideoPlayer.stop();
            return;
        }

        if (d->uiScene.itemAt(evt->posF())) {
            QGraphicsView::mousePressEvent(evt);
        } else {
            d->dragging = true;
            d->mouseMovedDuringDrag = false;
            evt->accept();
            d->lastPoint = evt->pos();
        }
    }

    void GameView::mouseReleaseEvent(QMouseEvent *evt)
    {
        if (d->mPlayingVideo) {
            d->mVideoPlayer.stop();
            return;
        }

        if (d->dragging) {
            if (!d->mouseMovedDuringDrag) {
                Renderable *renderable = d->pickObject(evt->pos());

                if (renderable) {
                    renderable->mouseReleaseEvent(evt);
                } else {
                    Vector4 worldPosition = d->worldPositionFromScreen(evt->pos());
                    emit worldClicked(evt->button(), evt->buttons(), worldPosition);
                }
            }
            d->dragging = false;
        } else {
            QGraphicsView::mouseReleaseEvent(evt);
        }
    }

    void GameView::mouseDoubleClickEvent(QMouseEvent *evt)
    {
        if (d->mPlayingVideo)
            return;

        // Sadly it's not really possible to detect, whether the double click was
        // really accepted by the scene. Instead we will simply check if there
        // is a QGraphicsItem at the clicked position.
        if (d->uiScene.itemAt(evt->posF())) {
            QGraphicsView::mouseDoubleClickEvent(evt);
        } else {
            Renderable *renderable = d->pickObject(evt->pos());

            if (renderable) {
                renderable->mouseDoubleClickEvent(evt);
            } else {
                Vector4 worldPosition = d->worldPositionFromScreen(evt->pos());
                emit worldDoubleClicked(evt->button(), evt->buttons(), worldPosition);
            }
        }
    }

    void GameView::centerOnWorld(const Vector4 &center)
    {
        d->centerOnWorld(center.x(), center.z());
    }

    Vector4 GameView::worldCenter() const
    {
        return d->getWorldCenter();
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

    void GameView::deleteScreenshot(const QUrl &url)
    {
        QString localFile = url.toLocalFile();

        QDir currentDir = QDir::current();
        if (!currentDir.cd("screenshots"))
            return;

        QFileInfo localFileInfo(localFile);
        currentDir.remove(localFileInfo.fileName());
    }

    void GameView::openBrowser(const QUrl &url)
    {
        qDebug("Opening browser with URL %s.", qPrintable(url.toString()));
        QDesktopServices::openUrl(url);
    }

    void GameView::setScrollingDisabled(bool disabled)
    {
        d->scrollingDisabled = disabled;
    }

    bool GameView::isScrollingDisabled() const
    {
        return d->scrollingDisabled;
    }

    bool GameView::playMovie(const QString &filename, const QScriptValue &callback)
    {
        if (d->mPlayingVideo) {
            qDebug("Already playing video. Movie queue is still todo...");
            return false;
        }

        if (!d->mVideoPlayer.open(filename)) {
            qDebug("Unable to open video: %s: %s", qPrintable(filename), qPrintable(d->mVideoPlayer.errorString()));
            return false;
        }

        setScene(&d->movieEmptyScene);

        d->mPlayingVideo = true;
        d->mVideoFinishedCallback = callback;
        d->mVideoPlayerThread.start();
        return true;
    }

    bool GameView::playUiSound(const QString &filename)
    {
        SharedSoundHandle handle = d->audioEngine.playSoundOnce(filename, SoundCategory_Interface);
        return !handle.isNull();
    }

    const QString &GameView::currentCursor() const
    {
        return d->cursorFilename;
    }

    void GameView::setCurrentCursor(const QString &filename)
    {
        d->cursorFilename = filename;

        QPixmap cursor(filename);
        setCursor(QCursor(cursor, 2, 2));
    }

}

#include "gameview.moc"
