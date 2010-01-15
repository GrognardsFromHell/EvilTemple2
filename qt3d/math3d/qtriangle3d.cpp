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

#include "qtriangle3d.h"

/*!
  \class QTriangle3D
  \brief The QTriangle3D class models 3 points in 3D space.
  \since 4.6
  \ingroup qt3d
  \ingroup qt3d::math

  A triangle is defined by 3 points in 3D space.  Since any 3 points define a plane,
  the triangle can be thought of as defining a plane, and forming a geometric region
  in that plane.

  If a simple plane, with no particular geometry is required the QPlane3D class is a
  more compact and mathematically sufficient definition of a plane.

  The three points are labelled P, Q and R for consistency with textbook treatments.
  For compatibility with counter-clockwise winding on 3D rendering systems, the three
  points should be thought of as producing an "outward facing" triangle, when P, Q
  and R are counter-clockwise vertices.

  The class provides a default constructor which produces a null triangle; and also
  a constructor taking initial assignments for P, Q and R.  The three defining points
  can be individually read or set.

  The width() function simply returns the length of P - Q, the base of the triangle
  (given its counter-clockwise winding), while the height() function returns the
  distance of the point R from the line PQ.  See the QLine3D class for definition of
  the distance from a line.

  \image triangle3d-dimensions.png

  A null triangle is one where all the three vertices P, Q and R are (0, 0, 0).  Test
  for this condition withe the isNull() function.

  A degenerate triangle is one with an area of zero.  This implies that either the
  width or the height is zero.  Apart from the case where at least two of P, Q and R
  are equal, the triangle is also degenerate if P, Q and R lie on the same line.
  A null triangle is a special case of a degenerate triangle.

 */

/*!
  \fn QTriangle3D::QTriangle3D()
  Construct a default triangle which lies in the x-z plane, and has P at the origin,
  Q at x = 1, y = 0; and R at x = 0, y = 1.
 */

/*!
  \fn QTriangle3D::QTriangle3D(const QVector3D &p, const QVector3D &q, const QVector3D &r)
  Construct a triangle with the supplied \a p, \a q and \a r vertices.
 */

/*!
  \fn QVector3D QTriangle3D::p() const
  Returns the value of the P vertex on the triangle.
 */

/*!
  \fn void QTriangle3D::setP(const QVector3D &point)
  Sets the value of the P vertex on the triangle to \a point.
 */


/*!
  \fn QVector3D QTriangle3D::q() const
  Returns the value of the Q vertex on the triangle.
 */

/*!
  \fn void QTriangle3D::setQ(const QVector3D &point)
  Sets the value of the Q vertex on the triangle \a point.
 */


/*!
  \fn QVector3D QTriangle3D::r() const
  Returns the value of the R vertex on the triangle.
 */

/*!
  \fn void QTriangle3D::setR(const QVector3D &point)
  Sets the value of the R vertex on the triangle \a point.
 */

/*!
  \fn qreal QTriangle3D::width() const
  Returns the width of the triangle.
  This is the distance from P to Q.
 */

/*!
  \fn qreal QTriangle3D::height() const
  Returns the height of the triangle.  This is the distance from the point R,
  to the line P-Q.  Here distance has the same definition as for QLine3D: the minimum
  distance of R from the P-Q, or equivalently, the length of a line perpendicular
  to the line P-Q which passes through R.
 */

/*!
  \fn qreal QTriangle3D::area() const
  Returns the area of the triangle.  This is calculated from the formulae
  \c {width() * height() * 0.5f}.
*/

/*!
  \fn bool QTriangle3D::isDegenerate() const
  Returns true if the triangle is degenerate, that is if its area is zero.

  A degenerate triangle cannot contain any point or intersect with any line.
 */

/*!
  \fn bool QTriangle3D::isNull() const
  Returns true if the triangle is null, that is if its vertices all coincide.  In this
  case width, height and area are all zero.  A null triangle is a special case of a
  degenerate triangle, in the case where it collapses to a point.
 */

/*!
  \fn QPlane3D QTriangle3D::plane() const
  Returns the plane in which the triangle lies.
 */

/*!
  \fn QVector3D QTriangle3D::center() const
  Returns the center of the triangle.  This point is the geometric average of the
  vertices, and is a point guaranteed to lie inside the triangle.

  \image triangle3d-center.png
 */

/*!
    \fn QVector3D QTriangle3D::faceNormal() const
    Returns a vector normal to this triangle.  The returned normal vector is based
    on a right-handed coordinate system, so that if P, Q and R are specified in a
    counter-clockwise rotation around the triangle as the viewer faces it, then the
    normal vector projects perpendicular out of the triangle toward the viewer.

    The vector is not normalized.  If unit length normal vectors are required, apply
    the QVector3D::normalize() function.
*/

/*!
  Returns true if this triange contains the \a point, false otherwise.  To contain
  the \a point means that:
  \list
  \o the point lies on the same plane as the triangle, and
  \o the point
    \list
      \o lies either wholly within the triangle, or
      \o lies on one of the sides, or
      \o coincides with one of the 3 vertices
    \endlist
  No check is made for a degenerate triangle.
  \endlist
 */
bool QTriangle3D::contains(const QVector3D &point) const
{
    if (qFuzzyCompare(point, m_p) || qFuzzyCompare(point, m_q) || qFuzzyCompare(point, m_r))
        return true;
    QLine3D apexToPoint(m_r, point - m_r);
    QLineSegment3D base(m_p, m_q);
    QResult<QVector3D> res = base.intersection(apexToPoint);
    if (!res.isValid())
        return false;
    return apexToPoint.distanceFromOrigin(point) <= apexToPoint.distanceFromOrigin(res.value());
}

/*!
  \fn bool QTriangle3D::intersects(const QLine3D &line) const
  Returns true if the \a line intersects with this triangle, false otherwise.
  This test constructs a point which is the intersection of the \a line with the
  plane this triangle lies in and tests that for containment.  Therefore the conditions
  that apply for containment apply for intersection as well.
  \sa contains(), intersection()
 */

/*!
  \fn QResult<QVector3D> QTriangle3D::intersection(const QLine3D &line) const
  Returns the intersection point of the \a line on this triangle.  Call
  QResult::resultExists on the returned value to check if an intersection exists.
  This test constructs a point which is the intersection of the \a line with the
  plane this triangle lies in and tests that for containment.  Therefore the
  conditions that apply for containment apply for intersection as well.
 */

/*!
  \fn QVector3D QTriangle3D::interpolate(qreal s, qreal t) const
  Returns a point on the plane defined by the triangle.  The properties of the point are:
  \list
  \o if \a s and \a t are between 0.0 and 1.0 inclusive then the point return lies in the triangle.
  \o as \a s approaches 1.0 the point is closer to point R
  \o as \a s approaches 0.0 the point is closer to the base, line P-Q
  \o as \a t approaches 1.0 the point is closer to Q
  \o as \a t approaches 0.0 the point is closer to the point P
  \endlist
  This result may be used to interpolate effects across the triangle.
 */
