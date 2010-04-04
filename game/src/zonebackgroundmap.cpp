#include <QtOpenGL>
#include "glext.h"
#include <QPainter>

#include "io/virtualfilesystem.h"
#include "game.h"
#include "zonebackgroundmap.h"
#include "camera.h"

/**
  This local hashing function uses a 16-bit shift. So coordinates
  are uniquely mapped, *but* there is a limited range.
  */
inline uint qHash ( const QPoint & key )
{
    return (key.x() << 16) | (key.y() & 0xFFFF);
}

namespace EvilTemple
{

    const int horizontalMapTiles = 66;
    const int verticalMapTiles = 71;
    const QVector2D centerWorldCoord(480, 481);

    /**
      Internal implementation details of ZoneBackgroundMap.
      */
    class ZoneBackgroundMapData
    {
    public:
        QString directory;

        QHash<QPoint, GLuint> textures;

        QImage loadMapTile(const Game &game, QPoint tile)
        {
            QImage image;

            // Try loading the image from the VFS
            QString filename = QString("%1%2%3")
                               .arg(directory)
                               .arg(tile.y(), 4, 16, QChar('0'))
                               .arg(tile.x(), 4, 16, QChar('0'))
                               .append(".jpg");
            QByteArray imageData = game.virtualFileSystem()->openFile(filename);

            if (imageData.isNull())
            {
                image = QImage(256, 256, QImage::Format_RGB32);

                QString label = QString("%1,%2").arg(tile.x()).arg(tile.y());

                QPainter painter(&image);
                painter.setPen(QColor(0, 0, 0));
                painter.fillRect(QRect(0, 0, 256, 256), QColor(255, 255, 255));
                painter.drawRect(QRect(0, 0, 256, 256));
                painter.drawText(QRect(0, 0, 256, 256), label, QTextOption(Qt::AlignCenter));
                painter.end();
            }
            else
            {
                image.loadFromData(imageData, "jpg");
            }

            return image;
        }

        void removeUnusedTiles(const QRect &visibleTiles)
        {
            // The rectangle is extended by a border of one tile (for precaching purposes)
            QRect adjusted = visibleTiles.adjusted(-1, -1, 1, 1);

            QList<QPoint> points = textures.keys();

            foreach (const QPoint &point, points)
            {
                if (!adjusted.contains(point))
                {
                    GLuint textureId = textures[point];
                    glDeleteTextures(1, &textureId);
                    textures.remove(point);
                }
            }
        }
    };

    ZoneBackgroundMap::ZoneBackgroundMap(const QString &directory, QObject *parent) :
        QObject(parent), d_ptr(new ZoneBackgroundMapData)
    {
        d_ptr->directory = directory;
    }

    ZoneBackgroundMap::~ZoneBackgroundMap()
    {
        foreach (GLuint textureId, d_ptr->textures)
        {
            glDeleteTextures(1, &textureId);
        }
        d_ptr->textures.clear();
    }

    void ZoneBackgroundMap::draw(const Game &game, QGLContext *context)
    {
        QVector2D mapTileCenter = game.camera()->worldToView(centerWorldCoord);

        // Please note: on the y-axis, positive is up, on the x-axis, negative is left.
        // We want the origin to be anchored at the top-left of the background map (since the files
        // are named that way).
        QVector2D mapTileOrigin(mapTileCenter.x() - horizontalMapTiles / 2.f * 256.f,
                                mapTileCenter.y() + verticalMapTiles / 2.f * 256.f);

        // The viewport in screen coordinates
        const QRectF &viewport = game.camera()->viewport();

        int xoffset = viewport.left() - mapTileOrigin.x(); // Viewport should be "right of" origin (>)
        int yoffset = mapTileOrigin.y() - viewport.bottom(); // Viewport should be "below" origin (<)

        int startX = qMin(horizontalMapTiles, qMax(0, xoffset / 256));
        int startY = qMin(verticalMapTiles, qMax(0, yoffset / 256));
        int endX = qMin(horizontalMapTiles, qMax(0, startX + (int)(viewport.width() + 255) / 256));
        int endY = qMin(verticalMapTiles, qMax(0, startY + (int)(viewport.height() + 255) / 256));

        Q_ASSERT(startX <= endX);
        Q_ASSERT(startY <= endY);

        // Draw a map tile for testing
        game.camera()->activate(true);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        QRect visibleTiles(startX, startY, endX - startX + 1, endY - startY + 1);
        d_ptr->removeUnusedTiles(visibleTiles);

        for (int x = startX; x <= endX; ++x)
        {
            for (int y = startY; y <= endY; ++y)
            {
                QVector2D mapTile(mapTileOrigin.x() + x * 256, mapTileOrigin.y() - y * 256);

                QPoint tile(x, y);
                if (!d_ptr->textures.contains(tile))
                {
                    QImage image = d_ptr->loadMapTile(game, tile);

                    GLuint textureId = context->bindTexture(image, GL_TEXTURE_2D, GL_RGB, QGLContext::NoBindOption);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                    d_ptr->textures[tile] = textureId;
                } else {
                    GLuint texture = d_ptr->textures[tile];

                    glBindTexture(GL_TEXTURE_2D, texture);
                }

                glBegin(GL_QUADS);
                glTexCoord2i(0, 0);
                glVertex2f(mapTile.x(), mapTile.y());
                glTexCoord2i(0, 1);
                glVertex2f(mapTile.x(), mapTile.y() - 256);
                glTexCoord2i(1, 1);
                glVertex2f(mapTile.x() + 256, mapTile.y() - 256);
                glTexCoord2i(1, 0);
                glVertex2f(mapTile.x() + 256, mapTile.y());
                glEnd();

                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }

}
