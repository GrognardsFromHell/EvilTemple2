/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplane3d.h"

#include <QtCore/qmath.h>

/*!
  \class QPlane3D
  \brief The QPlane3D class models the mathematics of planes in 3D space.
  \since 4.6
  \ingroup qt3d
  \ingroup qt3d::math

  A plane is defined by:

  \list
    \o a point, which can be thought of as an origin point, lying on the plane
    \o a normal vector, which is perpendicular to the surface of the plane
  \endlist

  The normal vector is normalized to be a unit length vector.

  The QPlane3D is an infinite surface, from which the unit long normal vector points
  out perpendicular from the origin point.

  Methods are provided for setting and returning the defining values of the plane.
  \list
    \o QVector3D QPlane3D::origin() const
    \o void QPlane3D::setOrigin(const QVector3D &point)
    \o QVector3D QPlane3D::normal() const
    \o void QPlane3D::setNormal(const QVector3D &vec)
  \endlist

  Determine if a given point or line is contained wholly within the plane using
  the contains() methods.
  \list
    \o bool QPlane3D::contains(const QVector3D &point) const
    \o bool QPlane3D::contains(const QLine3D &line) const
  \endlist

  \image qplane3d.png

  Find a point where a line intersects with the plane using the intersection() method.
  If a mere test of whether or not the line intersects is all that is required, use
  the intersects() method which is much cheaper.

  The project() methods project a point, vector or a line onto this plane.

  In this diagram a QLine3D intsects with the Z-X plane:
  \code
  QVector3D planeOrigin(1.0f, 0.0f, 1.0f);
  QVector3D planeNormal(0.0f, 1.0f, 0.0f);
  QPlane3D zxPlane(planeOrigin, planeNormal);
  QVector3D lineOrigin(24.0f, 32.0f, -22.0f);
  QVector3D lineDirection(2.0f, 1.0f, -1.5f);
  QLine3D ray(lineOrigin, lineDirection);
  QVector3D intersection = zxPlane.intersection(ray);
  QVector3D projection = zxPlane.project(ray);
  \endcode

  The projection is useful for forming coordinate systems and translating other geometry
  onto the plane.
 */

/*!
  \fn QPlane3D::QPlane3D()
  Construct a default QPlane3D object.
  The defining point of the plane is set to the origin of the world co-ordinate system
  (0, 0, 0) and the normal vector (which is not allowed to be null) is set arbitrarily
  to a unit vector lying on the x-axis (1, 0, 0).
 */

/*!
  \fn QPlane3D::QPlane3D(const QVector3D &point, const QVector3D &normal)
  Construct a new QPlane3D.
  The \a point lies on the new plane, and the \a normal vector is perpendicular to it.
  Together the point and the normal vector completely define the plane.  The vector is
  normalized to a unit length vector before setting it as the normal.
 */

/*!
  \fn QPlane3D::QPlane3D(const QVector3D &p, const QVector3D &q, const QVector3D &r)
  Construct a new QPlane3D containing the three given points.
  The point \a p is the planes origin point, and a normal is constructed from the
  other points \a q and \a r using the vector cross-product.  If P-Q-R forms a
  counter-clockwise triangle as viewed, then the normal vector will point out toward
  the viewer.  The vector is normalized to a unit length vector before setting it as
  the normal.

  This method effectively discards information in that the values of the points
  \a q and \a r cannot be reconstructed from the plane.
 */

/*!
  \fn QVector3D QPlane3D::origin() const
  Return this plane's defining point.
  \sa setOrigin()
 */

/*!
  \fn void QPlane3D::setOrigin(const QVector3D& value)
  Set this plane's defining point to be \a value.
  \sa origin()
 */

/*!
  \fn QVector3D QPlane3D::normal() const
  Return this plane's normal vector.
  The normal returned is guaranteed to be normalized to a unit vector.
  \sa setNormal()
 */

/*!
  \fn void QPlane3D::setNormal(const QVector3D& value)
  Set this plane's defining point to be \a value
  This method ensures that the \a value is normalized to a unit vector
  before setting it as the normal.
 */

/*!
  \fn bool QPlane3D::contains(const QVector3D &point) const
  Return true if the \a point lies in this plane.
 */

/*!
  \fn bool QPlane3D::contains(const QLine3D &line) const
  Return true if the \a line lies in this plane.  Note that this is
  only true if all points on the line are contained in the plane.
 */

/*!
  \fn bool QPlane3D::intersects(const QLine3D &line) const
  Return true if an intersection of the \a line with this plane exists,
  and false otherwise.  When the actual intersection point is not required,
  this method is cheaper than calling QPlane3D::intersection.  However the
  intermediate result is not retained, so if the result is needed on an
  intersection being present, use intersection() instead.
  \sa intersection()
 */

/*!
  Find the point at which the \a line intersects this plane.
  The result is returned as a QResult<QVector3D> instance.

  When the line intersects this plane, a QResult<QVector3D> class instance is
  returned where QResult::validity() returns QResult<QVector3D>::ResultExists.
  QResult<QVector3D>::value() on the instance will return the intersection.

  There are a number of failure cases where no single intersection point
  exists.  The cases are:
  \list
     \o when the line is paralell to the plane (but does not lie on it)
     \o the line lies entirely in the plane (thus every point "intersects")
  \endlist

  This method does not distinguish between these two failure cases and
  simply returns QResult<QVector3D>::NoResult in both cases.
 */
QResult<QVector3D> QPlane3D::intersection(const QLine3D& line) const
{
    qreal dotLineAndPlane = QVector3D::dotProduct(m_normal, line.direction());
    if (qIsNull(dotLineAndPlane))
    {
        // degenerate case - line & plane-normal vectors are at
        // 90 degrees to each other, so either plane and line never meet
        // or the line lies in the plane - return failure case.
        return QResult<QVector3D>();
    }
    QVector3D ppVec = m_origin - line.origin();
    if (ppVec.isNull())
        return m_origin;
    qreal t =  QVector3D::dotProduct(ppVec, m_normal) / dotLineAndPlane;
    return line.origin() + (t * line.direction());
}

/*!
  \fn QLine3D QPlane3D::project(const QLine3D &line) const
  Returns the projection of the \a line onto this plane.
 */

/*!
  Returns the projection of the \a point onto this plane.
  If the plane is visualized as a platform in 3D space, then the projection
  of the \a point above the plane is the shadow of the point, cast on the plane cast
  by an infinitely distant light source with its rays parallel to the normal.
 */
QVector3D QPlane3D::project(const QVector3D &point) const
{
    QVector3D ppVec = m_origin - point;
    if (ppVec.isNull())
        return point;

    // the component/projection of the point-to-point vector on the normal vector
    QVector3D normalComponent = QVector3D::dotProduct(m_normal, ppVec) * m_normal;

    // the component/projection of the point-to-point vector on the plane
    QVector3D planeComponent = ppVec - normalComponent;

    return m_origin + planeComponent;
}

/*!
  \fn bool QPlane3D::sameSide(const QVector3D &pointA, const QVector3D &pointB) const
  Returns true if \a pointA and \a pointB lie on the same side of this plane.  If either one
  lies in the plane, or they are on opposite sides of the plane returns false.
 */
