
#include "texture.h"
#include "sectormap.h"
#include "scenenode.h"
#include "scene.h"

#include "navigationmesh.h"

#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDataStream>
#include <QElapsedTimer>
#include <QVariant>

namespace EvilTemple {

    class SectorMapData
    {
    public:
        SectorMapData();

        Scene *scene;
        QWeakPointer<Sector> sector;

        SharedNavigationMesh walkableMesh;
        SharedNavigationMesh flyableMesh;

        RegionLayers regionLayers;
    };

    SectorMapData::SectorMapData() : flyableMesh(0), walkableMesh(0), scene(0)
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
        mBuffersInvalid(true),
        mBaseColor(1, 0, 0, 0.5f)
    {
        mRenderCategory = Renderable::DebugOverlay;
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

    void Sector::render(RenderStates &renderStates, MaterialState *overrideMaterial)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(renderStates.projectionMatrix().data());
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(renderStates.worldViewMatrix().data());

        if (mNavigationMesh) {
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
            glDisableClientState(GL_VERTEX_ARRAY);
        }

        if (!mLayer.isEmpty()) {
            glBegin(GL_QUADS);
            foreach (const TaggedRegion &region, mLayer) {
                QString type = region.tag.toString();

                if (type == "dirt") {
                    glColor4f(89 / 255.0, 82 / 255.0, 49 / 255.0, 0.5f);
                } else if (type == "grass") {
                    glColor4f(98 / 255.0, 164 / 255.0, 50 / 255.0, 0.5f);
                } else if (type == "water") {
                    glColor4f(44 / 255.0, 89 / 255.0, 255 / 255.0, 0.5f);
                } else if (type == "deepWater") {
                    glColor4f(32 / 255.0, 57 / 255.0, 153 / 255.0, 0.5f);
                } else if (type == "ice") {
                    glColor4f(132 / 255.0, 251 / 255.0, 255 / 255.0, 0.5f);
                } else if (type == "fire") {
                    glColor4f(252 / 255.0, 130 / 255.0, 0 / 255.0, 0.5f);
                } else if (type == "wood") {
                    glColor4f(166 / 255.0, 139 / 255.0, 102 / 255.0, 0.5f);
                } else if (type == "stone") {
                    glColor4f(80 / 255.0, 80 / 255.0, 80 / 255.0, 0.5f);
                } else if (type == "metal") {
                    glColor4f(179 / 255.0, 179 / 255.0, 179 / 255.0, 0.5f);
                } else {
                    glColor4fv(mBaseColor.data());
                }

                glVertex3f(region.left, 0, region.top);
                glVertex3f(region.left, 0, region.bottom);
                glVertex3f(region.right, 0, region.bottom);
                glVertex3f(region.right, 0, region.top);
            }
            glEnd();
        }

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

        foreach (const NavMeshRect &rect, mNavigationMesh->rectangles()) {
            // Convert to float for the stream-writing below
            float left = rect.left;
            float top = rect.top;
            float right = rect.right;
            float bottom = rect.bottom;

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

        foreach (const NavMeshPortal &portal, mNavigationMesh->portals()) {
            stream << portal.center.x() << (float)0 << portal.center.z();
        }

        mPortalVertexBuffer.bind();
        mPortalVertexBuffer.allocate(data.constData(), data.size());
        mPortalVertexBuffer.release();
    }

    QVector<Vector4> SectorMap::findPath(const Vector4 &start, const Vector4 &end) const
    {
        if (d->walkableMesh) {
            QElapsedTimer timer;
            timer.start();
            QVector<Vector4> result = d->walkableMesh->findPath(start, end);
            qint64 elapsed = timer.elapsed();
            if (elapsed > 10)
                qDebug("Total time for pathfinding: %ld ms.", elapsed);

            return result;
        }
        else
            return QVector<Vector4>();
    }

    bool SectorMap::hasLineOfSight(const Vector4 &from, const Vector4 &to) const
    {
        if (d->flyableMesh)
            return d->flyableMesh->hasLineOfSight(from, to);
        else
            return false;
    }

    bool SectorMap::load(const QString &filename) const
    {
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly))
            return false;

        d->regionLayers.clear();
        d->walkableMesh.clear();
        d->flyableMesh.clear();

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        d->walkableMesh = SharedNavigationMesh(new NavigationMesh);
        d->flyableMesh = SharedNavigationMesh(new NavigationMesh);

        stream >> *(d->walkableMesh.data()) >> *(d->flyableMesh.data());

        while (!stream.atEnd()) {
            QString layerName;
            uint count;
            RegionLayer layer;
            stream >> layerName >> count;
            layer.resize(count);

            for (int i = 0; i < count; ++i) {
                TaggedRegion &region = layer[i];
                stream >> region;
            }

            qDebug("Loaded region layer %s.", qPrintable(layerName));
            d->regionLayers[layerName] = layer;
        }

        qDebug("Using walkable mesh with %d rectangles and %d portals.", d->walkableMesh->rectangles().size(),
               d->walkableMesh->portals().size());
        qDebug("Using flyable mesh with %d rectangles and %d portals.", d->flyableMesh->rectangles().size(),
               d->flyableMesh->portals().size());

        return true;
    }

    bool SectorMap::createDebugView() const
    {
        if (d->scene && d->walkableMesh) {
            Sector *sector = new Sector;
            sector->setNavigationMesh(d->walkableMesh);

            SceneNode *node = d->scene->createNode();
            node->attachObject(sector);
        }

        return true;
    }

    bool SectorMap::createFlyableDebugView() const
    {
        if (d->scene && d->flyableMesh) {
            Sector *sector = new Sector;
            sector->setNavigationMesh(d->flyableMesh);

            SceneNode *node = d->scene->createNode();
            node->attachObject(sector);
        }

        return true;
    }

    bool SectorMap::createDebugLayer(const QString &layerName) const
    {
        return createDebugLayer(layerName, Vector4(1, 0, 0, 0.5f));
    }

    bool SectorMap::createDebugLayer(const QString &layerName, const Vector4 &baseColor) const
    {
        if (d->scene && d->regionLayers.contains(layerName)) {
            Sector *sector = new Sector;
            sector->setLayer(d->regionLayers[layerName]);
            sector->setBaseColor(baseColor);

            SceneNode *node = d->scene->createNode();
            node->attachObject(sector);
        }

        return true;
    }

    QVariant SectorMap::regionTag(const QString &layerName, const Vector4 &at) const
    {
        RegionLayers::const_iterator it = d->regionLayers.find(layerName);

        if (it == d->regionLayers.end()) {
            qWarning("Unknown region layer: %s.", qPrintable(layerName));
            return QVariant();
        }

        // Find the layer
        const RegionLayer &layer = it.value();

        // Find the rectangle that contains the point
        foreach (const TaggedRegion &region, layer) {
            if (at.x() >= region.left
                && at.z() >= region.top
                && at.x() <= region.right
                && at.z() <= region.bottom)
                return region.tag;
        }

        return QVariant();
    }

}
