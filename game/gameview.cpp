
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

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

#define HANDLE_GL_ERROR handleGlError(__FILE__, __LINE__);
    inline static void handleGlError(const char *file, int line) {
        QString error;

        GLenum glErr = glGetError();
        while (glErr != GL_NO_ERROR) {
            error.append(QString::fromLatin1((char*)gluErrorString(glErr)));
            error.append('\n');
            glErr = glGetError();
        }

        if (error.length() > 0) {
            qWarning("OpenGL error @ %s:%d: %s", file, line, qPrintable(error));
        }
    }
    
    class GameViewData
    {
    public:
        GameViewData(GameView *view)
            : q(view), rootItem(0), modelLoaded(0), backgroundMap(renderStates),
            clippingGeometry(renderStates), dragging(false), lightDebugger(renderStates),
            materials(renderStates), particleSystems(&materials) {

            sceneTimer.invalidate();

            qDebug("Initializing glew...");

            if (glewInit() != GLEW_OK) {
                qWarning("Unable to initialize GLEW.");
            }

            if (!particleSystems.loadTemplates()) {
                qWarning("Unable to load particle system templates: %s.", qPrintable(particleSystems.error()));
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

        void centerOnWorld(float worldX, float worldY)
        {
            Matrix4 matrix = Matrix4::translation(-worldX, 0, -worldY);
            renderStates.setViewMatrix(baseViewMatrix * matrix);
        }

        SharedRenderable pickObject(const QPoint &point) {
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

        RenderStates renderStates;
        ModelInstance model;
        ModelInstance model2;
        bool modelLoaded;

        Materials materials;

        bool dragging;
        QPoint lastPoint;
        Matrix4 baseViewMatrix; // Without translations

        typedef QPair<Vector4, QSharedPointer<Model> > GeometryMesh;

        QHash<QString, QWeakPointer<Model> > modelCache;

        LightDebugRenderer lightDebugger;

        BackgroundMap backgroundMap;

        ClippingGeometry clippingGeometry;

        ParticleSystems particleSystems;
        ParticleSystem *swordParticleSystem;

        QElapsedTimer sceneTimer;
        Scene scene;

        void resize(int width, int height) {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;
			glViewport(0, 0, width, height);

            const float zoom = 1.25f;

            Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth / zoom, halfWidth / zoom, -halfHeight / zoom, halfHeight / zoom, 1, 3628);
            renderStates.setProjectionMatrix(projectionMatrix);
        }

    private:
        GameView *q;
    };

    GameView::GameView(QWidget *parent) :
            QGraphicsView(parent), d(new GameViewData(this))
    {
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

        setMouseTracking(true);
    }

    GameView::~GameView()
    {
    }

    void GameView::objectMousePressed()
    {
        Renderable *renderable = qobject_cast<Renderable*>(sender());

        SceneNode *parent = renderable->parentNode();
        qDebug("Pressed mouse on object @ %f,%f,%f", parent->position().x(), parent->position().y(), parent->position().z());
    }

    void GameView::drawBackground(QPainter *painter, const QRectF &rect)
    {
        Q_UNUSED(painter);
        Q_UNUSED(rect);

        HANDLE_GL_ERROR

        if (!d->modelLoaded) {
            SharedModel model = loadModel("meshes/monsters/demon/demon.model");
            SharedModel model2 = loadModel("meshes/monsters/demon/demon_balor_sword.model");
            d->swordParticleSystem = d->particleSystems.instantiate("ef-Balor Sword");
            d->swordParticleSystem->setModelInstance(&d->model2);
            d->model.setModel(model);
            d->model2.setModel(model2);
            d->modelLoaded = true;
        }               

        glUseProgram(0);

        SAFE_GL(glEnable(GL_DEPTH_TEST));
        SAFE_GL(glEnable(GL_CULL_FACE));
        SAFE_GL(glEnable(GL_BLEND));
        SAFE_GL(glDisable(GL_STENCIL_TEST));

        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClearStencil(1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        d->backgroundMap.render();

        if (!d->sceneTimer.isValid()) {
            d->sceneTimer.start();
        } else {
            qint64 elapsedMs = d->sceneTimer.restart();
            double elapsedSeconds = elapsedMs / 1000.0;

            Profiler::enter(Profiler::SceneElapseTime);
            d->model.elapseTime(elapsedSeconds);
            d->model2.elapseTime(elapsedSeconds);
            d->scene.elapseTime(elapsedSeconds);
            Profiler::leave();
        }

        Matrix4 t;
        t.setToIdentity();
        t(0, 3) = 480 * 28.2842703f;
        t(2, 3) = 480 * 28.2842703f;
        d->renderStates.setWorldMatrix(t * Matrix4::rotation(Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(-120))));

        //d->model.render(d->renderStates);

        Matrix4 flipZ;
        flipZ.setToIdentity();
        flipZ(2,2) *= -1;

        Matrix4 boneTransform = d->model.getBoneSpace("HandL_ref");
        // Remove scaling
        float scaleX = boneTransform.column(0).length();
        float scaleY = boneTransform.column(1).length();
        float scaleZ = boneTransform.column(2).length();
        for (int i = 0; i < 4; ++i) {
            boneTransform(i, 0) /= scaleX;
            boneTransform(i, 1) /= scaleY;
            boneTransform(i, 2) /= scaleZ;
        }

        d->renderStates.setWorldMatrix(d->renderStates.worldMatrix() * flipZ * boneTransform * flipZ);

        d->model2.render(d->renderStates);

        d->renderStates.setWorldMatrix(Matrix4::identity());

        Profiler::enter(Profiler::SceneRender);
        d->scene.render(d->renderStates);
        Profiler::leave();

        SAFE_GL(glDisable(GL_CULL_FACE));
        SAFE_GL(glDisable(GL_DEPTH_TEST));
        SAFE_GL(glDisable(GL_TEXTURE_2D));
        SAFE_GL(glDisable(GL_LIGHTING));

        glDisable(GL_DEPTH_TEST); HANDLE_GL_ERROR
        glAlphaFunc(GL_ALWAYS, 0); HANDLE_GL_ERROR
        glDisable(GL_BLEND); HANDLE_GL_ERROR

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(d->renderStates.projectionMatrix().data());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(d->renderStates.viewMatrix()(0, 3), d->renderStates.viewMatrix()(1, 3), 0);

        // Draw a line-stippled version of the bounding box
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-8180, -8708, -1);
        glVertex3f(8180, -8708, -1);
        glVertex3f(8180, -18712, -1);
        glVertex3f(-8180, -18712, -1);
        glEnd();

        glClear(GL_DEPTH_BUFFER_BIT);HANDLE_GL_ERROR

        glLoadIdentity();HANDLE_GL_ERROR
        glMatrixMode(GL_PROJECTION);HANDLE_GL_ERROR
        glLoadIdentity();HANDLE_GL_ERROR
        glMatrixMode(GL_MODELVIEW);HANDLE_GL_ERROR

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void GameView::showView(const QString &url)
    {
        // Create the console
        QDeclarativeComponent *component = new QDeclarativeComponent(&d->uiEngine, this);
        component->loadUrl(QUrl::fromLocalFile(url));

        QEventLoop eventLoop;
        while (!component->isReady()) {
            if (component->isError()) {
                qFatal("Error creating console: %s.", qPrintable(component->errorString()));
                break;
            }
            eventLoop.processEvents();
        }

        QDeclarativeItem *widget = qobject_cast<QDeclarativeItem*>(component->create());

        d->uiScene.addItem(widget);
        d->rootItem = widget;

        widget->setWidth(width());
        widget->setHeight(height());
    }

    QDeclarativeEngine *GameView::uiEngine()
    {
        return &d->uiEngine;
    }

    void GameView::resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);

        // Update projection matrix
        d->resize(event->size().width(), event->size().height());        
    }

    void GameView::mouseMoveEvent(QMouseEvent *evt)
    {
        QGraphicsView::mouseMoveEvent(evt);
      
        if (d->dragging) {
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

        d->lastPoint = evt->pos();
    }

    void GameView::mousePressEvent(QMouseEvent *evt)
    {
        QGraphicsView::mousePressEvent(evt);
        
        if (!evt->isAccepted()) {
            SharedRenderable renderable = d->pickObject(evt->pos());

            if (renderable) {
                renderable->mousePressEvent();
            } else {
                d->dragging = true;
            }

            evt->accept();
            d->lastPoint = evt->pos();
        }
    }

    void GameView::mouseReleaseEvent(QMouseEvent *evt)
    {
        d->dragging = false;
        QGraphicsView::mouseReleaseEvent(evt);        
    }

    QPoint GameView::screenCenter() const
    {
        Vector4 viewCenter(0,0,0,1);
        Vector4 viewDirection(0,0,1,0);
        viewCenter = d->renderStates.viewMatrix().inverted() * viewCenter;
        viewDirection = d->renderStates.viewMatrix().inverted() * viewDirection;

        viewCenter *= 1/ viewCenter.w();

        return QPoint(viewCenter.x(), viewCenter.y());
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

    SharedModel GameView::loadModel(const QString &filename)
    {
        SharedModel model;

        if (d->modelCache.contains(filename)) {
            model = d->modelCache[filename];
        }

        if (model)
            return model;

        model = SharedModel(new Model);
        if (!model->open(filename, d->renderStates)) {
            qWarning("UNABLE TO LOAD MODEL: %s (%s)", qPrintable(filename), qPrintable(model->error()));
        }

        // This assumes that repeatedly loading a model won't fix the error.
        d->modelCache[filename] = model;

        return model;
    }

}
