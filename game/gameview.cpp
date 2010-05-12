
#include <GL/glew.h>

#include <QtDeclarative/QtDeclarative>
#include <QtGui/QResizeEvent>

#include "gameview.h"

#include "renderstates.h"
#include "modelfile.h"
#include "backgroundmap.h"

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

            Vector4 eyeVector(250.0, 500, 500, 0);
            Vector4 centerVector(0, 10, 0, 0);
            Vector4 upVector(0, 1, 0, 0);
            Matrix4 viewMatrix = Matrix4::lookAt(eyeVector, centerVector, upVector);
            renderStates.setViewMatrix(viewMatrix);

            backgroundMap.setMapDirectory("backgroundMaps/hommlet-exterior/");
        }

        QDeclarativeEngine uiEngine;
        QGraphicsScene uiScene;
        QDeclarativeItem* rootItem;

        RenderStates renderStates;
        Model model;
        bool modelLoaded;

        bool dragging;
        QPoint lastPoint;

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

        Draw(&d->model);

        glDisable(GL_DEPTH_TEST); HANDLE_GL_ERROR
        glDisable(GL_CULL_FACE); HANDLE_GL_ERROR
        glDisable(GL_ALPHA_TEST); HANDLE_GL_ERROR
        glAlphaFunc(GL_ALWAYS, 0); HANDLE_GL_ERROR
        glDisable(GL_BLEND); HANDLE_GL_ERROR

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(d->renderStates.projectionMatrix().data());
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
       // glTranslatef(d->renderStates.viewMatrix()(0, 3), d->renderStates.viewMatrix()(1, 3), 0);

        glDisable(GL_TEXTURE_2D);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);

        // Draw a line-stippled version of the bounding box
        glLineWidth(10);
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-8180, 8708, -1);
        glVertex3f(8180, 8708, -1);
        glVertex3f(8180, 18712, -1);
        glVertex3f(-8180, 18712, -1);

        //        glVertex2i(-8180, -8708);
        //        glVertex2i(8180, -8708);
        //        glVertex2i(8180, -18172);
        //        glVertex2i(-8180, -18172);
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

            Matrix4 viewMatrix = d->renderStates.viewMatrix();
            viewMatrix(0, 3) += diffX;
            viewMatrix(1, 3) -= diffY;

            d->renderStates.setViewMatrix(viewMatrix);
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
