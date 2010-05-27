
#include <GL/glew.h>

#include <QtCore/QTime>
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
            clippingGeometry(renderStates), particleSystems(renderStates), dragging(false), lightDebugger(renderStates) {

            qDebug("Initializing glew...");

            if (glewInit() != GLEW_OK) {
                qWarning("Unable to initialize GLEW.");
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

            qDebug("Loading background map...");

            backgroundMap.setMapDirectory("backgroundMaps/hommlet-exterior-night/");
            //backgroundMap.setMapDirectory("backgroundMaps/moathouse_interior/");

            QString mapDir = "maps/Map-2-Hommlet-Exterior/";

            if (!clippingGeometry.load(mapDir + "clippingGeometry.dat", &scene)) {
            //if (!clippingGeometry.load("maps/Map-7-Moathouse_Interior/clippingGeometry.dat")) {
                qWarning("Loading clipping geometry failed.");
            }

            qDebug("Loading static geometry...");

            QFile gmf(mapDir + "staticGeometry.txt");
            //QFile gmf("maps/Map-7-Moathouse_Interior/staticGeometry.txt");

            if (!gmf.open(QIODevice::ReadOnly)) {
                qFatal("Couldn't open GMF: %s", qPrintable(gmf.fileName()));
            }

            QTextStream stream(&gmf);

            while (!stream.atEnd()) {
                float x, y, z, w;
                QString modelFilename;
                stream >> x >> y >> z >> modelFilename;

                Vector4 position(x, y, z, 0);

                stream >> x >> y >> z >> w;

                Quaternion rotation(x, y, z, w);

                stream >> x >> y >> z;
                Vector4 scale(x, y, z, 1);

                int staticObject, rotationFromPrototype, customRotation;
                stream >> staticObject >> rotationFromPrototype >> customRotation;

                SharedModel model(new Model);

                if (modelCache.contains(modelFilename)) {
                    model = modelCache[modelFilename];
                } else if (!model->open(modelFilename, renderStates)) {
                    qWarning("UNABLE TO LOAD GMF: %s (%s)", qPrintable(modelFilename), qPrintable(model->error()));
                    continue;
                } else {
                    modelCache[modelFilename] = model;
                }

                QSharedPointer<ModelInstance> modelInstance(new ModelInstance());
                modelInstance->setModel(model);

                SharedSceneNode node(new SceneNode);
                node->setPosition(position);
                node->setRotation(rotation);
                node->setScale(scale);
                scene.addNode(node);

                node->attachObject(modelInstance);

                SharedRenderable bbRenderable(new BoxRenderable);
                //node->attachObject(bbRenderable);
            }

            qDebug("Loading particle systems for map...");

            QFile partSys(mapDir + "particleSystems.txt");

            if (!partSys.open(QIODevice::ReadOnly)) {
                qWarning("Unable to find particle system file.");
            }                       

            QDataStream partSysStream(&partSys);
            partSysStream.setByteOrder(QDataStream::LittleEndian);
            partSysStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

            while (!partSysStream.atEnd()) {
                float x, y, z, radius;
                QString name;

                partSysStream >> x >> y >> z >> radius >> name;
				
				particleSystems.create(name, Vector4(x, y, z, 1));
            }

            qDebug("Loading lighting...");

            QFile lightingFile(mapDir + "lighting.xml");
            if (!lightingFile.open(QIODevice::ReadOnly)) {
                qWarning("Missing lighting model.");
            }           

            loadLighting(&lightingFile);

            lightDebugger.loadMaterial();
        }

        void loadLighting(QIODevice *lightingSrc)
        {
            QDomDocument document;

            QString errorMsg;
            int errorLine, errorColumn;
            if (!document.setContent(lightingSrc, false, &errorMsg, &errorLine, &errorColumn)) {
                qWarning("Couldn't parse lighting XML file: %s (%d:%d)", qPrintable(errorMsg), errorLine, errorColumn);
                return;
            }

            QDomElement root = document.documentElement();

            QDomElement global = root.firstChildElement("global");
            // Not used yet

            QDomElement lightSources = root.firstChildElement("lightSources");
            for (QDomElement element = lightSources.firstChildElement(); 
                !element.isNull(); 
                element = element.nextSiblingElement()) {
                    Light light;
                    if (light.load(element))
                        lights.append(light);
                        
            }
        }

        void centerOnWorld(float worldX, float worldY)
        {
            Matrix4 matrix = Matrix4::translation(-worldY, 0, -worldX);
            renderStates.setViewMatrix(baseViewMatrix * matrix);
        }

        QDeclarativeEngine uiEngine;
        QGraphicsScene uiScene;
        QDeclarativeItem* rootItem;

        RenderStates renderStates;
        ModelInstance model;
        ModelInstance model2;
        bool modelLoaded;

        bool dragging;
        QPoint lastPoint;
        Matrix4 baseViewMatrix; // Without translations

        typedef QPair<Vector4, QSharedPointer<Model> > GeometryMesh;

        QList< Light > lights;

        QHash<QString, QWeakPointer<Model> > modelCache;

        LightDebugRenderer lightDebugger;

        BackgroundMap backgroundMap;

        ClippingGeometry clippingGeometry;

        ParticleSystems particleSystems;
        ParticleSystem *swordParticleSystem;

        QTime animationTimer;

        Scene scene;

        void resize(int width, int height) {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;
			glViewport(0, 0, width, height);
            Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 1, 3628);
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

    void GameView::drawBackground(QPainter *painter, const QRectF &rect)
    {
        Q_UNUSED(painter);
        Q_UNUSED(rect);

        HANDLE_GL_ERROR

        if (!d->modelLoaded) {
            SharedModel model(new Model);
            if (!model->open("meshes/monsters/demon/demon.model", d->renderStates)) {
                qWarning("Unable to open model file: %s", qPrintable(model->error()));
            }
            SharedModel model2(new Model);
            if (!model2->open("meshes/monsters/demon/demon_balor_sword.model", d->renderStates)) {
                qWarning("Unable to open model file: %s", qPrintable(model->error()));
            }
            d->swordParticleSystem = d->particleSystems.instantiate("ef-Balor Sword");
            d->swordParticleSystem->setModelInstance(&d->model2);
            d->model.setModel(model);
            d->model2.setModel(model2);
            d->modelLoaded = true;
        }

        glUseProgram(0);

        SAFE_GL(glEnable(GL_DEPTH_TEST));
        SAFE_GL(glEnable(GL_CULL_FACE));
        SAFE_GL(glEnable(GL_ALPHA_TEST));
        SAFE_GL(glAlphaFunc(GL_NOTEQUAL, 0));
        SAFE_GL(glEnable(GL_BLEND));
        SAFE_GL(glDisable(GL_STENCIL_TEST));

        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        d->backgroundMap.render();

        Matrix4 t;
        t.setToIdentity();
        t(0, 3) = 480 * 28.2842703f;
        t(2, 3) = 480 * 28.2842703f;
		d->renderStates.setWorldMatrix(t * Matrix4::rotation(Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(-120))));

		float elapsed = 0;

        if (!d->animationTimer.isNull()) {
            elapsed = d->animationTimer.restart() / 1000.0f;
        } else {
            d->animationTimer.start();
        }

		if (elapsed > 0) {
			d->model.elapseTime(elapsed);
            d->model2.elapseTime(elapsed);
            d->swordParticleSystem->setPosition(Vector4(482 * 28.2842703f, 10, 489 * 28.2842703f, 1));
            d->swordParticleSystem->elapseSeconds(elapsed);
            d->scene.elapseTime(elapsed);
		}

        d->model.render(d->renderStates);

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
        
        d->swordParticleSystem->setPosition(Vector4(480 * 28.2842703f, 20, 480 * 28.2842703f, 1));
        d->swordParticleSystem->render(d->renderStates, d->particleSystems.spriteMaterial());

        d->renderStates.setWorldMatrix(Matrix4::identity());

        d->scene.render(d->renderStates);
        
		SAFE_GL(glDisable(GL_CULL_FACE));
		SAFE_GL(glDisable(GL_DEPTH_TEST));
		SAFE_GL(glDisable(GL_TEXTURE_2D));
		SAFE_GL(glDisable(GL_LIGHTING));
				
		glDisable(GL_ALPHA_TEST); HANDLE_GL_ERROR
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

        glLoadIdentity();HANDLE_GL_ERROR
        glMatrixMode(GL_PROJECTION);HANDLE_GL_ERROR
        glLoadIdentity();HANDLE_GL_ERROR
        glMatrixMode(GL_MODELVIEW);HANDLE_GL_ERROR

        glClear(GL_DEPTH_BUFFER_BIT);HANDLE_GL_ERROR
    }

    void GameView::showView(const QString &url)
    {
        // Create the console
        QDeclarativeComponent *component = new QDeclarativeComponent(&d->uiEngine, this);
        component->loadUrl(url);
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
            d->lastPoint = evt->pos();

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
    }

    void GameView::mousePressEvent(QMouseEvent *evt)
    {
        QGraphicsView::mousePressEvent(evt);
        
        if (!evt->isAccepted()) {
            d->dragging = true;
            d->lastPoint = evt->pos();
            evt->accept();
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

}
