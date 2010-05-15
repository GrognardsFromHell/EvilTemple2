#ifndef UTIL_H
#define UTIL_H

#include <QScopedPointer>
#include <QtOpenGL>

#include <limits>

#include "qbox3d.h"

#include "gamemath.h"

namespace EvilTemple
{
    /**
      Custom deleter for QScopedPointer that uses aligned free.
      */
    struct AlignedDeleter
    {
        static inline void cleanup(void *pointer)
        {
            ALIGNED_FREE(pointer);
        }
    };

    const float Pi = 3.14159265358979323846f;

    /**
      The original game applies an additional base rotation to everything in order to align it
      with the isometric grid. This is the radians value of that rotation.
      */
    const float LegacyBaseRotation = 0.77539754f;

    /**
      Converts radians to degree.
      */
    inline float rad2deg(float rad)
    {
        return rad * 180.f / Pi;
    }

    /**
      Converts radians to degree.
      */
    inline float deg2rad(float deg)
    {
        return deg / 180.f * Pi;
    }

    /**
      This class represents an md5 hash that is used to identify a file.
      It is NOT used to actually create md5 hashes, use QCryptographicHash for that purpose
      instead.
      */
    class Md5Hash {
    friend inline uint qHash(const Md5Hash &key);
    public:
        bool operator ==(const Md5Hash &other) const;
        bool operator !=(const Md5Hash &other) const;
    private:
        int mHash[4];
    };

    inline bool Md5Hash::operator ==(const Md5Hash &other) const
    {
        return mHash[0] == other.mHash[0] &&
               mHash[1] == other.mHash[1] &&
               mHash[2] == other.mHash[2] &&
               mHash[3] == other.mHash[3];
    }

    inline uint qHash(const Md5Hash &key)
    {
        return key.mHash[0] ^ key.mHash[1] ^ key.mHash[2] ^ key.mHash[3];
    }

    inline bool Md5Hash::operator !=(const Md5Hash &other) const
    {
        return !this->operator==(other);
    }

    inline static void DrawVertex(const QVector3D &v) {
        glVertex3f(v.x(), v.y(), v.z());
    }

    inline void DrawDebugCoordinateSystem(const QVector3D &origin = QVector3D()) {

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glBegin(GL_LINES);
        // X Axis
        glColor4f(1, 0, 0, .5f);
        DrawVertex(origin);
        DrawVertex(origin + QVector3D(100, 0, 0));

        // Y Axis
        glColor4f(0, 1, 0, .5f);
        DrawVertex(origin);
        DrawVertex(origin + QVector3D(0, 100, 0));

        // Z Axis
        glColor4f(0, 0, 1, .5f);
        DrawVertex(origin);
        DrawVertex(origin + QVector3D(0, 0, 100));
        glEnd();

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }

    /**
      Draws a QBox3D on the screen using lines.
      */
    inline void DrawBox(const QBox3D &box)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glColor4f(1, 1, 1, .75f);

        /*
          The corners of the box. A,B,C,D are on a plane and so are E,F,G,H.
         */
        QVector3D a(box.minimum());
        QVector3D b(box.minimum().x(), box.maximum().y(), box.minimum().z());        
        QVector3D c(box.maximum().x(), box.maximum().y(), box.minimum().z());
        QVector3D d(box.maximum().x(), box.minimum().y(), box.minimum().z());

        QVector3D e(box.maximum());
        QVector3D f(box.minimum().x(), box.maximum().y(), box.maximum().z());        
        QVector3D g(box.minimum().x(), box.minimum().y(), box.maximum().z());
        QVector3D h(box.maximum().x(), box.minimum().y(), box.maximum().z());

        glBegin(GL_LINES);

        // Draw first rect
        DrawVertex(a); DrawVertex(b);
        DrawVertex(b); DrawVertex(c);
        DrawVertex(c); DrawVertex(d);
        DrawVertex(d); DrawVertex(a);

        // Draw second rect
        DrawVertex(e); DrawVertex(f);
        DrawVertex(f); DrawVertex(g);
        DrawVertex(g); DrawVertex(h);
        DrawVertex(h); DrawVertex(e);

        // Connect the two rects
        DrawVertex(a); DrawVertex(g);
        DrawVertex(b); DrawVertex(f);
        DrawVertex(c); DrawVertex(e);
        DrawVertex(d); DrawVertex(h);

        glEnd();

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
    }

    inline bool Intersects( const QLine3D &ray, const QBox3D &box, float &distance )
    {
        float d = 0.0f;
        float maxValue = std::numeric_limits<qreal>::infinity();

        const QVector3D &position = ray.origin();
        const QVector3D &direction = ray.direction().normalized();
        const QVector3D &minimum = box.minimum();
        const QVector3D &maximum = box.maximum();

        if( fabs( direction.x() ) < 0.0000001 )
        {
            if( position.x() < minimum.x() || position.x() > maximum.x() )
            {
                distance = 0.0f;
                return false;
            }
        }
        else
        {
            float inv = 1.0f / direction.x();
            float min = (minimum.x() - position.x()) * inv;
            float max = (maximum.x() - position.x()) * inv;

            if( min > max )
            {
                float temp = min;
                min = max;
                max = temp;
            }

            d = qMax( min, d );
            maxValue = qMin( max, maxValue );

            if( d > maxValue )
            {
                distance = 0.0f;
                return false;
            }
        }

        if( fabs( direction.y() ) < 0.0000001 )
        {
            if( position.y() < minimum.y() || position.y() > maximum.y() )
            {
                distance = 0.0f;
                return false;
            }
        }
        else
        {
            float inv = 1.0f / direction.y();
            float min = (minimum.y() - position.y()) * inv;
            float max = (maximum.y() - position.y()) * inv;

            if( min > max )
            {
                float temp = min;
                min = max;
                max = temp;
            }

            d = qMax( min, d );
            maxValue = qMin( max, maxValue );

            if( d > maxValue )
            {
                distance = 0.0f;
                return false;
            }
        }

        if( fabs( direction.z() ) < 0.0000001 )
        {
            if( position.z() < minimum.z() || position.z() > maximum.z() )
            {
                distance = 0.0f;
                return false;
            }
        }
        else
        {
            float inv = 1.0f / direction.z();
            float min = (minimum.z() - position.z()) * inv;
            float max = (maximum.z() - position.z()) * inv;

            if( min > max )
            {
                float temp = min;
                min = max;
                max = temp;
            }

            d = qMax( min, d );
            maxValue = qMin( max, maxValue );

            if( d > maxValue )
            {
                distance = 0.0f;
                return false;
            }
        }

        distance = d;
        return true;
    }

    /**
      Intersects the ray with a triangle.

      This algorithm is equivalent to the algorithm presented in Realtime Rendering p.750
      as RayTriIntersect.

      @param uOut If not null, this pointer receives the barycentric weight of
                    p1 for the point of intersection. But only if there is an intersection.
      @param vOut If not null, this pointer receives the barycentric weight of
                    p2 for the point of intersection. But only if there is an intersection.

      @returns True if the ray shoots through the triangle.
      */
    inline bool Intersects(const QLine3D &ray,
                           const QVector3D &p0,
                           const QVector3D &p1,
                           const QVector3D &p2,
                           float &distance,
                           float *uOut = 0,
                           float *vOut = 0) {

        QVector3D e1 = p1 - p0;
        QVector3D e2 = p2 - p0;

        QVector3D q = QVector3D::crossProduct(ray.direction(), e2);
        float determinant = QVector3D::dotProduct(e1, q);

        /**
          If the determinant is close to zero, the ray lies in the plane of the triangle and thus
          is very unlikely to intersect it.
          */
        if (qFuzzyIsNull(determinant))
            return false;

        float invertedDeterminant = 1 / determinant;

        // Distance from vertex 0 to ray origin
        QVector3D s = ray.origin() - p0;

        // Calculate the first barycentric coordinate
        float u = invertedDeterminant * QVector3D::dotProduct(s, q);

        if (u < 0)
            return false; // Definetly outside the triangle

        QVector3D r = QVector3D::crossProduct(s, e1);

        // Calcaulate the second barycentric coordinate
        float v = invertedDeterminant * QVector3D::dotProduct(ray.direction(), r);

        if (v < 0 || u + v > 1)
            return false; // Definetly outside the triangle

        // Store u + v for further use
        if (uOut)
            *uOut = u;
        if (vOut)
            *vOut = v;

        // Calculate the exact point of intersection and then the distance to the ray's origin
        QVector3D point = (1 - u - v) * p0 + u * p1 + v * p2;
        distance = ray.distanceFromOrigin(point);
        return true;
    }

}

#endif // UTIL_H
