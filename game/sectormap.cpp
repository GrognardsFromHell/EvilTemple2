#include "sectormap.h"
#include "texture.h"
#include "scenenode.h"
#include "scene.h"

#include "navigationmesh.h"
#include "navigationmeshbuilder.h"

#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDataStream>
#include <QElapsedTimer>

namespace EvilTemple {

    class SectorMapData
    {
    public:
        SectorMapData();

        Scene *scene;
        QWeakPointer<Sector> sector;

        SharedNavigationMesh mesh;
    };

    SectorMapData::SectorMapData() : mesh(0), scene(0)
    {

    }

    SectorMap::SectorMap(Scene *scene) : d(new SectorMapData)
    {
        d->scene = scene;
    }

    SectorMap::~SectorMap()
    {
    }

    Sector::Sector()
        : mVertexBuffer(QGLBuffer::VertexBuffer),
        mColorBuffer(QGLBuffer::VertexBuffer),
        mIndexBuffer(QGLBuffer::IndexBuffer),
        mPortalVertexBuffer(QGLBuffer::VertexBuffer),
        mBuffersInvalid(true)
    {
        mRenderCategory = RenderQueue::DebugOverlay;
        mBoundingBox.setToInfinity();
    }

    const Box3d &Sector::boundingBox()
    {
        return mBoundingBox;
    }

    void Sector::setNavigationMesh(const SharedNavigationMesh &navigationMesh)
    {
        mBuffersInvalid = true;
        mNavigationMesh = navigationMesh;
    }

    void Sector::render(RenderStates &renderStates)
    {
        if (!mNavigationMesh)
            return;

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(renderStates.projectionMatrix().data());
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(renderStates.worldViewMatrix().data());

        if (mBuffersInvalid) {
            buildBuffers();
            mBuffersInvalid = false;
        }

        mVertexBuffer.bind();
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        mColorBuffer.bind();
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        mIndexBuffer.bind();
        glDrawElements(GL_QUADS, 4 * mNavigationMesh->rectangles().size(), GL_UNSIGNED_INT, 0);
        mIndexBuffer.release();

        glDisableClientState(GL_COLOR_ARRAY);

        mPortalVertexBuffer.bind();
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glPointSize(2);
        glColor3f(0.1, 0.1f, 0.8f);
        glDrawArrays(GL_POINTS, 0, mNavigationMesh->portals().size());

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    void Sector::buildBuffers()
    {
        if (!mIndexBuffer.isCreated())
            mIndexBuffer.create();
        if (!mVertexBuffer.isCreated())
            mVertexBuffer.create();
        if (!mColorBuffer.isCreated())
            mColorBuffer.create();
        if (!mPortalVertexBuffer.isCreated())
            mPortalVertexBuffer.create();

        srand(1234656812);

        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        stream.setByteOrder(QDataStream::BigEndian);
#else
        stream.setByteOrder(QDataStream::LittleEndian);
#endif
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        foreach (const NavMeshRect *rect, mNavigationMesh->rectangles()) {
            float left = rect->left;
            float top = rect->top;
            float right = rect->right;
            float bottom = rect->bottom;

            const float zero = 0;

            stream << left << zero << top
                    << left << zero << bottom
                    << right << zero << bottom
                    << right << zero << top;
        }

        mVertexBuffer.bind();
        mVertexBuffer.allocate(data.constData(), data.size());
        mVertexBuffer.release();

        stream.device()->seek(0);
        data.clear();

        for (int i = 0; i < mNavigationMesh->rectangles().size(); ++i) {
            float r = 0.5f + 0.5f * (rand() / (float)RAND_MAX);
            float g = 0.5f + 0.5f * (rand() / (float)RAND_MAX);
            float b = 0.5f + 0.5f * (rand() / (float)RAND_MAX);

            uint color = qRgba(r * 255, g * 255, b * 255, 127);

            stream << color << color << color << color;
        }

        mColorBuffer.bind();
        mColorBuffer.allocate(data.constData(), data.size());
        mColorBuffer.release();

        stream.device()->seek(0);
        data.clear();

        for (int i = 0; i < mNavigationMesh->rectangles().size() * 4; i += 4) {
            stream << i << (i + 1) << (i + 2) << (i + 3);
        }

        mIndexBuffer.bind();
        mIndexBuffer.allocate(data.constData(), data.size());
        mIndexBuffer.release();

        stream.device()->seek(0);
        data.clear();

        foreach (const NavMeshPortal *portal, mNavigationMesh->portals()) {
            stream << portal->center.x() << (float)0 << portal->center.z();
        }

        mPortalVertexBuffer.bind();
        mPortalVertexBuffer.allocate(data.constData(), data.size());
        mPortalVertexBuffer.release();
    }

    QVector<Vector4> SectorMap::findPath(const Vector4 &start, const Vector4 &end) const
    {
        if (d->mesh) {
            QElapsedTimer timer;
            timer.start();
            QVector<Vector4> result = d->mesh->findPath(start, end);
            qint64 elapsed = timer.elapsed();
            qDebug("Total time for pathfinding: %ld ms.", elapsed);

            return result;
        }
        else
            return QVector<Vector4>();
    }

    bool SectorMap::hasLineOfSight(const Vector4 &from, const Vector4 &to) const
    {
        if (d->mesh)
            return d->mesh->hasLineOfSight(from, to);
        else
            return false;
    }

    bool SectorMap::load(const QVector<Vector4> &startPositions, const QString &filename) const
    {
        NavigationMeshBuilder builder;
        SharedNavigationMesh mesh(builder.build(filename, startPositions));

        if (!mesh) {
            qWarning("Unable to open %s.", qPrintable(filename));
            return false;
        }

        d->mesh = mesh;

        qDebug("Using mesh with %d rectangles and %d portals.", mesh->rectangles().size(), mesh->portals().size());

        if (d->scene) {
            Sector *sector = new Sector;
            sector->setNavigationMesh(mesh);

            SceneNode *node = new SceneNode;
            node->attachObject(SharedRenderable(sector));

            d->scene->addNode(SharedSceneNode(node));
        }

        return true;
    }

}
