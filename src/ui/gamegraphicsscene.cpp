
#include <QtOpenGL>
#include <QMainWindow>
#include <QGraphicsWebView>
#include <QPropertyAnimation>
#include <cmath>
#include <limits>

#include "glext.h"
#include "ui/gamegraphicsscene.h"
#include "io/virtualfilesystem.h"
#include "game.h"
#include "model.h"
#include "util.h"
#include "geometrymeshobject.h"
#include "camera.h"
#include "io/skmreader.h"
#include "campaign/campaign.h"
#include "campaign/zone.h"
#include "zonetemplate.h"
#include "zonebackgroundmap.h"
#include "ui/meshdialog.h"
#include "ui/consolewidget.h"

#include "qline3d.h"

namespace EvilTemple {

    extern QTime TransformTimer; // TODO: Refactor this. VERY bad design.

    GameGraphicsScene::GameGraphicsScene(const Game &game, QObject *parent) :
            QGraphicsScene(parent),            
            dragging(false),
            totalTime(0),
            totalFrames(0),
            game(game),
            roty(0),
            _objectsDrawn(0)
    {
        // Whenever the view changes, update the scene
        connect(game.camera(), SIGNAL(positionChanged()), this, SLOT(update()));

        console = new ConsoleWidget(game);
        console->setVisible(false);
        addWidget(console);

        model.reset(new GeometryMeshObject());
        model->setModelSource(new SkmReader(game.virtualFileSystem(),
                                            game.materials(),
                                            "art/meshes/monsters/"
                                            "demon/demon.skm"));
        model->setPosition(QVector3D(480 * PixelPerWorldTile, 0, 480 * PixelPerWorldTile));

        QTimer *animTimer = new QTimer(this);
        animTimer->setInterval(5);
        animTimer->setSingleShot(false);
        connect(animTimer, SIGNAL(timeout()), this, SLOT(update()));
        animTimer->start();

        // Update the console whenever the scene rect changes
        connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(resizeConsole(QRectF)));

        roty = 180;
    }

    GameGraphicsScene::~GameGraphicsScene() {
    }

    void GameGraphicsScene::rotate()
    {
        roty += 1;

        invalidate(sceneRect(), AllLayers);
    }

    void GameGraphicsScene::toggleConsole()
    {
        if (console->isVisible()) {
            QPropertyAnimation *hideAnim = new QPropertyAnimation(console, "size");
            hideAnim->setDuration(250);
            hideAnim->setStartValue(QSize(width(), console->height()));
            hideAnim->setEndValue(QSize(width(), 0));
            hideAnim->setEasingCurve(QEasingCurve::OutQuad);
            hideAnim->start(QAbstractAnimation::DeleteWhenStopped);

            connect(hideAnim, SIGNAL(finished()), console, SLOT(hide()));
        } else {
            console->show();

            QPropertyAnimation *showAnim = new QPropertyAnimation(console, "size");
            showAnim->setDuration(250);
            showAnim->setStartValue(QSize(width(), 0));
            showAnim->setEndValue(QSize(width(), height() * 0.4));
            showAnim->setEasingCurve(QEasingCurve::OutQuad);
            showAnim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    void GameGraphicsScene::resizeConsole(const QRectF &viewport)
    {
        //console->resize(viewport.width(), 0.4 * viewport.height());
    }

    void GameGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
    {
        Q_UNUSED(painter); // We use the QGL Context directly in this method

        timer.start();

        QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glPushAttrib(GL_ALL_ATTRIB_BITS); // Save all attrib states so QGraphicsView can do what it wants        

        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);

        // Ensure that the Z-Buffer is used
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Enable culling of back faces
        glEnable(GL_CULL_FACE);

        // By default, blending is active
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Transparent pixels mess up the depth buffer, so discard any fragments
        // that are entirely transparent.
        glAlphaFunc(GL_GREATER, 0);
        glEnable(GL_ALPHA_TEST);

        // Enable anti aliasing
        glEnable(GL_LINE_SMOOTH); // For lines
        glEnable(GL_MULTISAMPLE); // And for everything else

        // Default blending attributes for textures
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

        // This should probably only be set whenever the viewport changes
        game.camera()->setViewport(QRectF(- rect.width() / 2, - rect.height() / 2, rect.width(), rect.height()));       

        QBox3D aabbFrustum = game.camera()->viewFrustum();

        _objectsDrawn = 0; // Reset number of objects drawn.

        GeometryMeshObject *selectedObject = pickObject(_lastMousePos);

        // Draw the campaign if the campaign is running
        Campaign *campaign = game.campaign();
        if (campaign && campaign->currentZone())
        {
            const QSharedPointer<ZoneTemplate> &zoneTemplate = campaign->currentZone()->zoneTemplate();

            // Draw the zone background map
            ZoneBackgroundMap *map = zoneTemplate->dayBackground();

            if (map)
            {
                map->draw(game, context);
            }

            // Draw other static geometry objects
            game.camera()->activate();

            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color buffer writing
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_TEXTURE_2D);

            foreach (GeometryMeshObject *object, zoneTemplate->clippingGeometry())
            {
                if (aabbFrustum.contains(object->boundingBox())
                    || aabbFrustum.intersects(object->boundingBox()))
                    {
                    object->draw(game, context);
                    _objectsDrawn++;
                }
            }

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
            glEnable(GL_CULL_FACE);

            // Set lighting state
            glEnable(GL_LIGHT0);
            glEnable(GL_LIGHT1);

            const GLfloat ambientLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

            const GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
            const GLfloat diffuseDir[] = { 0, -1, 0, 0 };
            glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
            glLightfv(GL_LIGHT1, GL_POSITION, diffuseDir);           

            foreach (GeometryMeshObject *object, zoneTemplate->staticGeometry())
            {
                if (aabbFrustum.contains(object->boundingBox())
                    || aabbFrustum.intersects(object->boundingBox()))
                    {
                    if (object == selectedObject)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    object->draw(game, context);
                    if (object == selectedObject)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    _objectsDrawn++;
                }
            }
        }

        game.camera()->activate();

        model->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, roty));
        model->draw(game, context);

        /*
          Draw the map limits
         */

        game.camera()->activate(true);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glLineWidth(5);

        glBegin(GL_LINE_LOOP);
        glVertex2i(-8180, -8708);
        glVertex2i(8180, -8708);
        glVertex2i(8180, -18172);
        glVertex2i(-8180, -18172);
        glEnd();

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);        

        // Reset state

        glPopAttrib();
        glPopClientAttrib();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        totalTime += timer.elapsed();
        totalFrames++;
        TransformTimer.restart();
    }

    void GameGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
    {
        QGraphicsScene::mousePressEvent(mouseEvent);

        if (!mouseEvent->isAccepted())
        {
            GeometryMeshObject *selected = pickObject(_lastMousePos);

            if (selected) {
                MeshDialog *meshDialog = new MeshDialog(selected, NULL);
                meshDialog->show();
            } else {
                dragging = true;
            }

            mouseEvent->accept();            
        }
    }

    void GameGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
    {
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
        dragging = false;
    }

    void GameGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
    {
        QPointF pos = mouseEvent->scenePos(); // Relative to center of scene, y+ is down, x+ is right.

        _lastMousePos = QVector2D(pos.x(), sceneRect().height() - pos.y());

        if (dragging)
        {
            QVector2D diff = QVector2D(mouseEvent->scenePos() - mouseEvent->lastScenePos());
            game.camera()->moveView(diff);
            mouseEvent->accept();
        }
        else
        {
            QGraphicsScene::mouseMoveEvent(mouseEvent);
        }
    }

    double GameGraphicsScene::fps() const {
        return totalTime / totalFrames;
    }

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
    }

}
