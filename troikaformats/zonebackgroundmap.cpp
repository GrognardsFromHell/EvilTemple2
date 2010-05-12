
#include <GL/glew.h>

#include <QtOpenGL>
#include <QPainter>

#include "virtualfilesystem.h"
#include "zonebackgroundmap.h"

/**
  This local hashing function uses a 16-bit shift. So coordinates
  are uniquely mapped, *but* there is a limited range.
  */
inline uint qHash ( const QPoint & key )
{
    return (key.x() << 16) | (key.y() & 0xFFFF);
}

namespace Troika
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

        QImage loadMapTile(VirtualFileSystem *vfs, QPoint tile)
        {
            QImage image;

            // Try loading the image from the VFS
            QString filename = QString("%1%2%3")
                               .arg(directory)
                               .arg(tile.y(), 4, 16, QChar('0'))
                               .arg(tile.x(), 4, 16, QChar('0'))
                               .append(".jpg");
            QByteArray imageData = vfs->openFile(filename);

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

    const QString &ZoneBackgroundMap::directory() const
    {
        return d_ptr->directory;
    }

}
