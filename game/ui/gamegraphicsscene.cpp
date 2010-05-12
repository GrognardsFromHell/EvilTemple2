
#include <GL/glew.h>

#include <QtOpenGL>
#include <QMainWindow>
#include <QGraphicsWebView>
#include <QPropertyAnimation>
#include <cmath>
#include <limits>

#include "ui/gamegraphicsscene.h"
#include "game.h"
#include "util.h"
#include "camera.h"

#include "qline3d.h"

#include "renderstates.h"
#include "modelfile.h"

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
            qWarning("OpenGL error @ %s:%d: %s", file, line);
        }
    }

    void Draw(Model *model) {
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

                    // Set render states
                    foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                            state->enable();
                    }

                    // Draw the actual model
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceGroup.buffer); HANDLE_GL_ERROR
                    glDrawElements(GL_TRIANGLES, faceGroup.elementCount, GL_UNSIGNED_SHORT, 0); HANDLE_GL_ERROR

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

    class GameGraphicsSceneData
    {
    public:
        GameGraphicsSceneData()
            : dragging(false),
                totalTime(0),
                totalFrames(0),
                objectsDrawn(0),
                initialized(false),
                modelLoaded(false)
        {
        }

        bool initialized;
        Model model;
        bool modelLoaded;

        RenderStates renderStates;

        bool dragging;
        QTime timer;

        int totalTime;
        int totalFrames;

        int objectsDrawn;
        QVector2D lastMousePos;
    };

    GameGraphicsScene::GameGraphicsScene(const Game &game, QObject *parent) :
            QGraphicsScene(parent),
            d_ptr(new GameGraphicsSceneData)
    {
        QTimer *animTimer = new QTimer(this);
        animTimer->setInterval(5);
        animTimer->setSingleShot(false);
        connect(animTimer, SIGNAL(timeout()), this, SLOT(invalidate()));
        animTimer->start();
    }

    GameGraphicsScene::~GameGraphicsScene() {
    }

//    void GameGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
//    {
//        Q_UNUSED(painter); // We use the QGL Context directly in this method

//        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glEnable(GL_ALPHA_TEST);
//        glAlphaFunc(GL_NOTEQUAL, 0);
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//        if (!d_ptr->initialized) {
//            if (!d_ptr->model.open("meshes/monsters/demon/demon.model", d_ptr->renderStates)) {
//                qWarning("Unable to open model file: %s", qPrintable(d_ptr->model.error()));
//            }
//            d_ptr->initialized = true;
//        }

//        glClearColor(0.25, 0.25, 0.25, 0);
//        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//        // TODO: Convert this to Matrix4-only code (don't use the GL matrix stack)
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        //gluPerspective(45, (float)w / (float)h, 1, 200);
//        int w = 800;
//        int h = 600;
//        glOrtho(-w/2, w/2, -h/2, h/2, 1, 3628);
//        Matrix4 projectionMatrix;
//        glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix.data());
//        d_ptr->renderStates.setProjectionMatrix(projectionMatrix);
//        glLoadIdentity();
//        glMatrixMode(GL_MODELVIEW);

//        Vector4 eyeVector(250.0, 500, 500, 0);
//        Vector4 centerVector(0, 10, 0, 0);
//        Vector4 upVector(0, 1, 0, 0);
//        Matrix4 viewMatrix = Matrix4::lookAt(eyeVector, centerVector, upVector);

//        d_ptr->renderStates.setViewMatrix(viewMatrix);

//        Draw(&d_ptr->model);

//        return;

//        d_ptr->timer.start();

//        QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());

//        glMatrixMode(GL_PROJECTION);
//        glPushMatrix();

//        glMatrixMode(GL_MODELVIEW);
//        glPushMatrix();

//        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
//        glPushAttrib(GL_ALL_ATTRIB_BITS); // Save all attrib states so QGraphicsView can do what it wants

//        // Clear screen
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//        glEnable(GL_TEXTURE_2D);

//        // Ensure that the Z-Buffer is used
//        glEnable(GL_DEPTH_TEST);
//        glDepthFunc(GL_LEQUAL);

//        // Enable culling of back faces
//        glEnable(GL_CULL_FACE);

//        // By default, blending is active
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glEnable(GL_BLEND);

//        // Transparent pixels mess up the depth buffer, so discard any fragments
//        // that are entirely transparent.
//        glAlphaFunc(GL_GREATER, 0);
//        glEnable(GL_ALPHA_TEST);

//        // Enable anti aliasing
//        glEnable(GL_LINE_SMOOTH); // For lines
//        glEnable(GL_MULTISAMPLE); // And for everything else

//        // Default blending attributes for textures
//        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
//        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
//        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

//        // This should probably only be set whenever the viewport changes
//        // game.camera()->setViewport(QRectF(- rect.width() / 2, - rect.height() / 2, rect.width(), rect.height()));

//        // QBox3D aabbFrustum = game.camera()->viewFrustum();

//        d_ptr->objectsDrawn = 0; // Reset number of objects drawn.

//        /*GeometryMeshObject *selectedObject = pickObject(_lastMousePos);

//        // Draw the campaign if the campaign is running
//        Campaign *campaign = game.campaign();
//        if (campaign && campaign->currentZone())
//        {
//            const QSharedPointer<ZoneTemplate> &zoneTemplate = campaign->currentZone()->zoneTemplate();

//            // Draw the zone background map
//            ZoneBackgroundMap *map = zoneTemplate->dayBackground();

//            if (map)
//            {
//                map->draw(game, context);
//            }

//            // Draw other static geometry objects
//            game.camera()->activate();

//            glDisable(GL_LIGHTING);
//            glDisable(GL_CULL_FACE);
//            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color buffer writing
//            glDisable(GL_TEXTURE_2D);

//            foreach (GeometryMeshObject *object, zoneTemplate->clippingGeometry())
//            {
//                if (aabbFrustum.contains(object->boundingBox())
//                    || aabbFrustum.intersects(object->boundingBox()))
//                    {
//                    object->draw(game, context);
//                    _objectsDrawn++;
//                }
//            }

//            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//            glEnable(GL_TEXTURE_2D);
//            glEnable(GL_LIGHTING);
//            glEnable(GL_CULL_FACE);

//            // Set lighting state
//            glEnable(GL_LIGHT0);
//            glEnable(GL_LIGHT1);

//            const GLfloat ambientLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
//            glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

//            const GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
//            const GLfloat diffuseDir[] = { 0, -1, 0, 0 };
//            glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
//            glLightfv(GL_LIGHT1, GL_POSITION, diffuseDir);

//            foreach (GeometryMeshObject *object, zoneTemplate->staticGeometry())
//            {
//                if (aabbFrustum.contains(object->boundingBox())
//                    || aabbFrustum.intersects(object->boundingBox()))
//                    {
//                    if (object == selectedObject)
//                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//                    object->draw(game, context);
//                    if (object == selectedObject)
//                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//                    _objectsDrawn++;
//                }
//            }
//        }

//        game.camera()->activate();

//        model->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, roty));
//        model->draw(game, context); */

//        /*
//          Draw the map limits
//         */

//        // game.camera()->activate(true);

//        glDisable(GL_TEXTURE_2D);
//        glDisable(GL_DEPTH_TEST);
//        glDisable(GL_LIGHTING);

//        glBegin(GL_LINE_LOOP);
//        glVertex2i(-8180, -8708);
//        glVertex2i(8180, -8708);
//        glVertex2i(8180, -18172);
//        glVertex2i(-8180, -18172);
//        glEnd();

//        glEnable(GL_TEXTURE_2D);
//        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_LIGHTING);

//        // Reset state

//        glPopAttrib();
//        glPopClientAttrib();

//        glMatrixMode(GL_PROJECTION);
//        glPopMatrix();

//        glMatrixMode(GL_MODELVIEW);
//        glPopMatrix();

//        d_ptr->totalTime += d_ptr->timer.elapsed();
//        d_ptr->totalFrames++;
//    }

//    void GameGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
//    {
//        QGraphicsScene::mousePressEvent(mouseEvent);

//        if (!mouseEvent->isAccepted())
//        {
//            /*GeometryMeshObject *selected = pickObject(_lastMousePos);

//            if (selected) {
//               // MeshDialog *meshDialog = new MeshDialog(selected, NULL);
//              //  meshDialog->show();
//            } else {
//                dragging = true;
//            }*/

//            mouseEvent->accept();
//        }
//    }

//    void GameGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
//    {
//        QGraphicsScene::mouseReleaseEvent(mouseEvent);
//        d_ptr->dragging = false;
//    }

//    void GameGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
//    {
//        QPointF pos = mouseEvent->scenePos(); // Relative to center of scene, y+ is down, x+ is right.

//        d_ptr->lastMousePos = QVector2D(pos.x(), sceneRect().height() - pos.y());

//        if (d_ptr->dragging)
//        {
//            QVector2D diff = QVector2D(mouseEvent->scenePos() - mouseEvent->lastScenePos());
//            // game.camera()->moveView(diff);
//            mouseEvent->accept();
//        }
//        else
//        {
//            QGraphicsScene::mouseMoveEvent(mouseEvent);
//        }
//    }

    double GameGraphicsScene::fps() const {
        if (d_ptr->totalFrames > 0) {
            return d_ptr->totalTime / d_ptr->totalFrames;
        } else {
            return 0;
        }
    }

    /*
    GeometryMeshObject *GameGraphicsScene::pickObject(QVector2D screenPos)
    {
        Camera *camera = game.camera();

        QVector3D nearPoint = camera->unproject(QVector3D(screenPos, -1));
        QVector3D farPoint = camera->unproject(QVector3D(screenPos, 1));

        // Construct a ray through the view frustum
        QLine3D pickRay(nearPoint, (farPoint - nearPoint).normalized());

        Campaign *campaign = game.campaign();
        if (!campaign || !campaign->currentZone())
        {
            return NULL;
        }

        const QSharedPointer<ZoneTemplate> &zoneTemplate = campaign->currentZone()->zoneTemplate();

        GeometryMeshObject *result = NULL;
        qreal resultDistance = std::numeric_limits<qreal>::infinity();

        foreach (GeometryMeshObject *object, zoneTemplate->staticGeometry())
        {
            float distance;
            if (Intersects(pickRay, object->boundingBox(), distance))
            {
                float distance;
                if (object->intersects(pickRay, distance) && distance < resultDistance)
                {
                    resultDistance = distance;
                    result = object;
                }
            }
        }

        return result;
    }*/

    int GameGraphicsScene::objectsDrawn() const
    {
        return d_ptr->objectsDrawn;
    }

}
