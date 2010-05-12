
#include <QtCore/QFile>
#include <QtOpenGL/QGLBuffer>

#include <QtCore/QHash>
#include <QtCore/QPoint>
#include <QtGui/QImage>

#include "backgroundmap.h"
#include "renderstates.h"
#include "materialstate.h"

inline uint qHash(const QPoint &key)
{
    return qHash((key.x() << 16) & 0xFFFF0000 | (key.y() & 0xFFFF));
}

namespace EvilTemple {

class BackgroundMapData
{
public:
    BackgroundMapData(const RenderStates &states) : renderStates(states), positionBuffer(QGLBuffer::VertexBuffer),
        texCoordBuffer(QGLBuffer::VertexBuffer), indexBuffer(QGLBuffer::IndexBuffer)
    {

        QFile materialFile(":/material/map_material.xml");

        if (!materialFile.open(QIODevice::ReadOnly)) {
            qWarning("Unable to open map material.");
            return;
        }

        Material material;

        if (!material.loadFromData(materialFile.readAll())) {
            qWarning("Unable to parse material file.");
            return;
        }

        materialFile.close();

        if (!materialState.createFrom(material, renderStates, NULL)) {
            qWarning("Unable to create material state for background map: %s", qPrintable(materialState.error()));
            return;
        }

        if (!positionBuffer.create()) {
            qWarning("Unableto create position buffer.");
        }
        if (!positionBuffer.bind()) {
            qWarning("Unable to bind position buffer.");
        }

        Vector4 positions[4];
        positions[0] = Vector4(0, -256, -1, 1);
        positions[1] = Vector4(256, -256, -1, 1);
        positions[2] = Vector4(0, 0, -1, 1);
        positions[3] = Vector4(256, 0, -1, 1);
        positionBuffer.allocate(positions, sizeof(positions));
        positionBuffer.release();

        texCoordBuffer.create();
        texCoordBuffer.bind();

        static const float texCoords[8] = {0, 0, 1, 0, 0, 1, 1, 1};
        texCoordBuffer.allocate(texCoords, sizeof(texCoords));
        texCoordBuffer.release();

        static const short indices[6] = {0, 1, 3, 3, 2, 0};
        indexBuffer.create();
        indexBuffer.bind();
        indexBuffer.allocate(indices, sizeof(indices));
        indexBuffer.release();
    }

    bool setMapDirectory(const QString &mapDirectory)
    {
        this->mapDirectory = mapDirectory;

        // Clear all textures
        textures.clear();
        tilesPresent.clear();

        QFile indexFile(mapDirectory + "index.dat");
        indexFile.open(QIODevice::ReadOnly);
        QDataStream stream(&indexFile);
        stream.setByteOrder(QDataStream::LittleEndian);

        uint count;
        stream >> count;

        quint16 x, y;
        for (uint i = 0; i < count; ++i) {
            stream >> x >> y;
            tilesPresent.insert(QPoint(x,y), true);
        }

        return true;
    }

    /**
      For every texture we have in our cache, a metric is computed, stating how far away
      the texture is from the screen viewport. This metric is used to determine whether
      the texture should be culled from the cache.

      @param xMin The first visible tile.
      @param yMin The first visible tile.
      @param xMax The last visible tile.
      @param yMax The last visible tile.
      */
    void cleanCache(int xMin, int yMin, int xMax, int yMax) {
        TextureCache::iterator it = textures.begin();

        while (it != textures.end()) {
            // Compute the manhatten distance metric for the texture
            int xDist = xMin - it.key().x();
            if (xDist < 0) {
                xDist = qMax<int>(0, it.key().x() - xMax);
            }

            int yDist = yMin - it.key().y();
            if (yDist < 0) {
                yDist = qMax<int>(0, it.key().y() - yMax);
            }

            if (qMax<int>(xDist, yDist) > 3) {
                qDebug("Removing %d,%d from texture cache.", it.key().x(), it.key().y());
                it = textures.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool isTilePresent(const QPoint &point)
    {
        return tilesPresent.contains(point);
    }

    SharedTexture loadTexture(const QPoint &point)
    {
        TextureCache::iterator it = textures.find(point);

        if (it != textures.end()) {
            return *it;
        }

        QString filename = QString("%1%3-%2.jpg").arg(mapDirectory).arg(point.x()).arg(point.y());

        QFile backgroundTexture(filename);

        SharedTexture result(new Texture);
        result->setMagFilter(GL_NEAREST);
        result->setMinFilter(GL_NEAREST);
        textures.insert(point, result);

        if (backgroundTexture.open(QIODevice::ReadOnly)) {
            QImage image;
            image.loadFromData(backgroundTexture.readAll(), "JPG");

            if (!result->load(image)) {
                qWarning("Unable to load background image %s.", qPrintable(filename));
            }
        } else {
            qWarning("Unable to find background image %s.", qPrintable(filename));
        }

        return result;
    }

    const RenderStates &renderStates;

    QHash<QPoint, bool> tilesPresent;
    typedef QHash<QPoint, SharedTexture> TextureCache;

    TextureCache textures;
    QString mapDirectory;
    MaterialState materialState;
    QGLBuffer positionBuffer;
    QGLBuffer texCoordBuffer;
    QGLBuffer indexBuffer;
};

BackgroundMap::BackgroundMap(const RenderStates &states) : d(new BackgroundMapData(states))
{
}

BackgroundMap::~BackgroundMap()
{

}

bool BackgroundMap::setMapDirectory(const QString &directory)
{
    return d->setMapDirectory(directory);
}

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

void BackgroundMap::render()
{
    MaterialState *material = &d->materialState;

    for (int i = 0; i < material->passCount; ++i) {
        MaterialPassState &pass = material->passes[i];

        pass.program.bind(); HANDLE_GL_ERROR

        glActiveTexture(GL_TEXTURE0);

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
                d->positionBuffer.bind();
                break;
            case 2:
                d->texCoordBuffer.bind();
                break;
            }

            // Assign the attribute
            glEnableVertexAttribArray(attribute.location);
            glVertexAttribPointer(attribute.location, attribute.binding.components(), attribute.binding.type(),
                                  attribute.binding.normalized(), attribute.binding.stride(), (GLvoid*)attribute.binding.offset());
            HANDLE_GL_ERROR
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind any previously bound buffers

        d->indexBuffer.bind(); HANDLE_GL_ERROR

        int tileLocation = pass.program.uniformLocation("tilePos");

        // Draw the actual model
        int tilePosition[2] = {0, 0};

        /*
            To deduce which map-tiles need to be painted, we retrieve the screen-space viewport
            (without translation, meaning -> absolute coordinates), and relate it to the origin
            of the current map in the same coordinate space.
         */
        const Box2d &screenViewport = d->renderStates.screenViewport();

        // Negative map tile coordinates are not considered
        int firstVisibleX = qMax<int>(0, (int)screenViewport.left() / 256);
        int firstVisibleY = qMax<int>(0, (int)screenViewport.top() / 256);
        int lastVisibleX = qMax<int>(0, (int)screenViewport.right() / 256);
        int lastVisibleY = qMax<int>(0, (int)screenViewport.bottom() / 256);

        for (int x = firstVisibleX; x <= lastVisibleX; ++x) {
            for (int y = firstVisibleY; y <= lastVisibleY; ++y) {
                if (!d->isTilePresent(QPoint(x, y)))
                    continue;

                tilePosition[0] = x;
                tilePosition[1] = y;

                const SharedTexture &texture = d->loadTexture(QPoint(x,y));
                texture->bind();
                glUniform1iv(tileLocation, 2, tilePosition); HANDLE_GL_ERROR
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0); HANDLE_GL_ERROR
            }
        }

        d->indexBuffer.release(); HANDLE_GL_ERROR

        // Unbind attributes
        for (int j = 0; j < pass.attributes.size(); ++j) {
            MaterialPassAttributeState &attribute = pass.attributes[j];
            glDisableVertexAttribArray(attribute.location); HANDLE_GL_ERROR
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        pass.program.unbind();

        // Clean the cache
        d->cleanCache(firstVisibleX, firstVisibleY, lastVisibleX, lastVisibleY);
    }

}

}
