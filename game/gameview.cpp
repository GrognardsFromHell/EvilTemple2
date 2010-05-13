
#include <GL/glew.h>

#include <QtDeclarative/QtDeclarative>
#include <QtGui/QResizeEvent>

#include "gameview.h"

#include "renderstates.h"
#include "modelfile.h"
#include "backgroundmap.h"

namespace EvilTemple {

    // TODO: THIS IS TEMP
    class GMF {
    public:
        GMF() : model(new Model) {}

        Vector4 position;
        Quaternion rotation;
        QSharedPointer<Model> model;
    };

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

    static void Draw(Model *model) {
        for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
            const FaceGroup &faceGroup = model->faceGroups[faceGroupId];
            MaterialState *material = faceGroup.material;

            if (!material)
                continue;

            for (int i = 0; i < material->passCount; ++i) {
                MaterialPassState &pass = material->passes[i];

                pass.program.bind(); HANDLE_GL_ERROR

                        // Bind texture samplers
                        for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                    pass.textureSamplers[j].bind(); HANDLE_GL_ERROR
                        }

                // Bind uniforms
                for (int j = 0; j < pass.uniforms.size(); ++j) {
                    pass.uniforms[j].bind(); HANDLE_GL_ERROR
                        }

                // Bind attributes
                for (int j = 0; j < pass.attributes.size(); ++j) {
                    MaterialPassAttributeState &attribute = pass.attributes[j];

                    // Bind the correct buffer
                    switch (attribute.bufferType) {
                    case 0:
                        glBindBuffer(GL_ARRAY_BUFFER, model->positionBuffer);
                        break;
                    case 1:
                        glBindBuffer(GL_ARRAY_BUFFER, model->normalBuffer);
                        break;
                    case 2:
                        glBindBuffer(GL_ARRAY_BUFFER, model->texcoordBuffer);
                        break;
                    }

                    // Assign the attribute
                    glEnableVertexAttribArray(attribute.location);
                    glVertexAttribPointer(attribute.location, attribute.binding.components(), attribute.binding.type(),
                                          attribute.binding.normalized(), attribute.binding.stride(), (GLvoid*)attribute.binding.offset());
                    HANDLE_GL_ERROR
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind any previously bound buffers

                // Set render states
                foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                    state->enable();
                }

                // Draw the actual model
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceGroup.buffer); HANDLE_GL_ERROR
                glDrawElements(GL_TRIANGLES, faceGroup.elementCount, GL_UNSIGNED_SHORT, 0); HANDLE_GL_ERROR
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); HANDLE_GL_ERROR

                // Reset render states to default
                foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                      state->disable();
                }

                // Unbind textures
                for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                    pass.textureSamplers[j].unbind();
                }

                // Unbind attributes
                for (int j = 0; j < pass.attributes.size(); ++j) {
                    MaterialPassAttributeState &attribute = pass.attributes[j];
                    glDisableVertexAttribArray(attribute.location); HANDLE_GL_ERROR
                }

                pass.program.unbind();
            }
        }
    }

    class GameViewData
    {
    public:
        GameViewData(GameView *view)
            : q(view), rootItem(0), modelLoaded(0), backgroundMap(renderStates),
        dragging(false) {
            if (glewInit() != GLEW_OK) {
                qWarning("Unable to initialize GLEW.");
            }

            Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, deg2rad(44.42700648682643));
            Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

            Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(90-135.0000005619373));
            Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

            renderStates.setViewMatrix(rotate1matrix * rotate2matrix);

            Matrix4 transform = rotate1matrix;

            Vector4 upVector(0, 1, 0, 0);
            Vector4 eyeVector(0, 0, 1, 1);

           // upVector = transform * upVector;
            eyeVector = transform * eyeVector;

            /*
            Vector4 eyeVector(250.0, 500, 500, 0);
            Vector4 centerVector(0, 10, 0, 0);
            Vector4 upVector(0, 1, 0, 0);
            Matrix4 viewMatrix = Matrix4::lookAt(eyeVector, centerVector, upVector);
            renderStates.setViewMatrix(viewMatrix);
            */
            upVector.normalize();

            Matrix4 lookAt = Matrix4::lookAt(eyeVector, Vector4(0, 0, 0, 0), upVector);
            renderStates.setViewMatrix(lookAt);

            Matrix4 id;
            id.setToIdentity();
            id(2,3) = -3000;

            renderStates.setViewMatrix(id * rotate1matrix * rotate2matrix);

            backgroundMap.setMapDirectory("backgroundMaps/hommlet-exterior/");

            QFile gmf("maps/Map-2-Hommlet-Exterior/staticGeometry.dat");

            if (!gmf.open(QIODevice::ReadOnly)) {
                qFatal("Couldn't open GMF.");
            }

            QDataStream stream(&gmf);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

            while (!stream.atEnd()) {
                float x, y, z, w;
                QString modelFilename;
                stream >> z >> y >> x >> modelFilename;

                GMF obj;
                obj.position = Vector4(x, y, z, 0);

                stream >> x >> y >> z >> w;

                obj.rotation = Quaternion(x, y, z, w);

                if (modelCache.contains(modelFilename)) {
                    obj.model = modelCache[modelFilename];
                } else if (!obj.model->open(modelFilename, renderStates)) {
                    qWarning("UNABLE TO LOAD GMF: %s", qPrintable(modelFilename));
                    continue;
                }

                modelCache[modelFilename] = obj.model;
                geometryMeshes.append(obj);
            }
        }

        QDeclarativeEngine uiEngine;
        QGraphicsScene uiScene;
        QDeclarativeItem* rootItem;

        RenderStates renderStates;
        Model model;
        bool modelLoaded;

        bool dragging;
        QPoint lastPoint;

        typedef QPair<Vector4, QSharedPointer<Model> > GeometryMesh;

        QList< GMF > geometryMeshes;

        QHash<QString, QWeakPointer<Model> > modelCache;

        BackgroundMap backgroundMap;

        void resize(int width, int height) {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;

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

        // Ensure constant updates
        QTimer *animTimer = new QTimer(this);
        animTimer->setInterval(5);
        animTimer->setSingleShot(false);
        connect(animTimer, SIGNAL(timeout()), this, SLOT(update()));
        animTimer->start();

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
            if (!d->model.open("meshes/monsters/demon/demon.model", d->renderStates)) {
            //if (!d->model.open("meshes/scenery/portals/stairs-down.model", d->renderStates)) {
                qWarning("Unable to open model file: %s", qPrintable(d->model.error()));
            }
            d->modelLoaded = true;
        }

        glEnable(GL_DEPTH_TEST); HANDLE_GL_ERROR
        glEnable(GL_CULL_FACE); HANDLE_GL_ERROR
        glEnable(GL_ALPHA_TEST); HANDLE_GL_ERROR
        glAlphaFunc(GL_NOTEQUAL, 0); HANDLE_GL_ERROR
        glEnable(GL_BLEND); HANDLE_GL_ERROR

        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        d->backgroundMap.render();

        Matrix4 t;
        t.setToIdentity();;
        t(0, 3) = 480 * 28.2842703f;
        t(2, 3) = 480 * 28.2842703f;
        d->renderStates.setWorldMatrix(t);

        Draw(&d->model);

        t.setToIdentity();
        d->renderStates.setWorldMatrix(t);

        foreach (const GMF &geometryMesh, d->geometryMeshes) {
            Matrix4 positionMatrix = Matrix4::transformation(Vector4(1, 1, 1, 0),
                                                             geometryMesh.rotation,
                                                             geometryMesh.position)
            * Matrix4::transformation(Vector4(1,1,1,0), Quaternion::fromAxisAndAngle(0, 1, 0, Pi), Vector4(0,0,0,0));
            d->renderStates.setWorldMatrix(positionMatrix);

            Draw(geometryMesh.model.data());

            positionMatrix.setToIdentity();
            d->renderStates.setWorldMatrix(positionMatrix);
        }

        glDisable(GL_DEPTH_TEST); HANDLE_GL_ERROR
        glDisable(GL_CULL_FACE); HANDLE_GL_ERROR
        glDisable(GL_ALPHA_TEST); HANDLE_GL_ERROR
        glAlphaFunc(GL_ALWAYS, 0); HANDLE_GL_ERROR
        glDisable(GL_BLEND); HANDLE_GL_ERROR

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(d->renderStates.projectionMatrix().data());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixf(d->renderStates.viewMatrix().data());

        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(100, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 100, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 100);

        glColor3f(1, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(480 * 28.2842703f, 0, 480 * 28.2842703f);
        glEnd();

        glPointSize(50);
        glColor3f(1, 0, 0);
        glBegin(GL_POINTS);
        glVertex3f(19.9999352f, -13454.0, -1);
        glVertex3f(480 * 28.2842703f, 0, 480 * 28.2842703f);

        glEnd();

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
        // Update projection matrix
        d->resize(event->size().width(), event->size().height());

        QGraphicsView::resizeEvent(event);
    }

    void GameView::mouseMoveEvent(QMouseEvent *event)
    {
        if (d->dragging) {
            int diffX = event->pos().x() - d->lastPoint.x();
            int diffY = event->pos().y() - d->lastPoint.y();
            d->lastPoint = event->pos();

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
            event->accept();

            viewport()->repaint();
        }

        QGraphicsView::mouseMoveEvent(event);
    }

    void GameView::mousePressEvent(QMouseEvent *event)
    {
        QGraphicsView::mousePressEvent(event);

        if (!event->isAccepted()) {
            d->dragging = true;
            d->lastPoint = event->pos();
            //grabMouse(QCursor(Qt::ClosedHandCursor));
            event->accept();
        }
    }

    void GameView::mouseReleaseEvent(QMouseEvent *event)
    {
        QGraphicsView::mouseReleaseEvent(event);

        d->dragging = false;
        //releaseMouse();
        event->accept();
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

}
