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

#include "qbox3d.h"
#include "qplane3d.h"
#include "qresult.h"

#include <QtGui/qmatrix4x4.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

/*!
    \class QBox3D
    \brief The QBox3D class represents an axis-aligned box in 3D space.
    \since 4.6
    \ingroup qt3d
    \ingroup qt3d::math

    QBox3D can be used to represent the bounding box of objects in a 3D
    scene so that they can be easily culled if they are out of view.

    The sides of the box are always aligned with the x, y, and z axes of
    the world co-ordinate system.  Transforming a box with transformed()
    will result in the smallest axis-aligned bounding box that contains
    the transformed box.

    Boxes may be null, finite, or infinite.  A null box does not occupy
    any space and does not intersect with any other box.  A finite
    box consists of a minimum() and maximum() extent in 3D space.
    An infinite box encompasses all points in 3D space.

    The extents of a finite box are also included within the box.
    A box with minimum() and maximum() set to the same value
    contains a single point.
*/

/*!
    \fn QBox3D::QBox3D()

    Constructs a null box in 3D space.

    \sa isNull()
*/

/*!
    \fn QBox3D::QBox3D(const QVector3D& corner1, const QVector3D& corner2)

    Constructs a finite box in 3D space from \a corner1 to \a corner2.
    The minimum() and maximum() co-ordinates of the new box are set
    to the minimum and maximum x, y, and z values from \a corner1 and
    \a corner2.  The \a corner1 and \a corner2 values can be any two
    opposite corners that define the box.

    \sa isFinite(), minimum(), maximum()
*/

/*!
    \fn bool QBox3D::isNull() const

    Returns true if this box is null; false otherwise.

    \sa isFinite(), isInfinite(), setNull()
*/

/*!
    \fn bool QBox3D::isFinite() const

    Returns true if this box is finite in size; false otherwise.

    \sa isNull(), isInfinite(), setExtents()
*/

/*!
    \fn bool QBox3D::isInfinite() const

    Returns true if this box is infinite in size; false otherwise.

    \sa isNull(), isFinite(), setInfinite()
*/

/*!
    \fn QVector3D QBox3D::minimum() const

    Returns the minimum corner of this box.

    \sa maximum(), setExtents()
*/

/*!
    \fn QVector3D QBox3D::maximum() const

    Returns the maximum corner of this box.

    \sa minimum(), setExtents()
*/

/*!
    \fn void QBox3D::setExtents(const QVector3D& corner1, const QVector3D& corner2)

    Sets the extents of this box to a finite region from \a corner1 to
    \a corner2.  The minimum() and maximum() co-ordinates of the box are
    set to the minimum and maximum x, y, and z values from \a corner1 and
    \a corner2.  The \a corner1 and \a corner2 values can be any two
    opposite corners that define the box.

    \sa minimum(), maximum()
*/

/*!
    \fn void QBox3D::setNull()

    Sets this box to null.

    \sa isNull()
*/

/*!
    \fn void QBox3D::setInfinite()

    Sets this box to be infinite in size.

    \sa isInfinite()
*/

/*!
    \fn QVector3D QBox3D::size() const

    Returns the finite size of this box.  If this box is null or
    infinite, the returned value will be zero.

    \sa center(), volume(), isNull(), isInfinite()
*/

/*!
    \fn QVector3D QBox3D::center() const

    Returns the finite center of this box.  If this box is null
    or infinite, the returned value will be zero.

    \sa size(), volume(), isNull(), isInfinite()
*/

/*!
    \fn qreal QBox3D::volume() const

    Returns the finite volume of this box.  If this box is null
    or infinite, the returned value will be zero.

    \sa size(), center(), isNull(), isInfinite()
*/

/*!
    \fn bool QBox3D::contains(const QVector3D& point) const

    Returns true if this box contains \a point; false otherwise.
    Null boxes do not contain any points and infinite boxes contain
    all points.

    Containment is not a strict test: the point is contained if it
    lies on one of the faces of the box.

    \sa intersects()
*/

/*!
    \fn bool QBox3D::contains(const QBox3D& box) const

    Returns true if this box completely contains \a box.  If this box
    is null, then it will not contain \a box.  If this box is infinite,
    and \a box is not null, then \a box will be contained within this box.
    If \a box is infinite, then this box must also be infinite to contain it.

    \sa intersects()
*/

/*!
    Returns true if \a box intersects this box; false otherwise.

    \sa intersect(), intersected(), contains()
*/
bool QBox3D::intersects(const QBox3D& box) const
{
    if (boxtype == Null)
        return false;
    else if (boxtype == Infinite)
        return box.boxtype != Null;
    else if (box.boxtype == Null)
        return false;
    else if (box.boxtype == Infinite)
        return true;

    if (maxcorner.x() < box.mincorner.x())
        return false;
    if (mincorner.x() > box.maxcorner.x())
        return false;

    if (maxcorner.y() < box.mincorner.y())
        return false;
    if (mincorner.y() > box.maxcorner.y())
        return false;

    if (maxcorner.z() < box.mincorner.z())
        return false;
    if (mincorner.z() > box.maxcorner.z())
        return false;

    return true;
}

/*!
  Returns true if \a line intersects this box; false otherwise.

  This method is a convenience function that simply returns true if a
  result exists on evaluating intersection() for this line.  If the
  intersection point is used, then call intersection() and test the
  validity of it instead of using this function.

  \sa intersection()
 */
bool QBox3D::intersects(const QLine3D &line) const
{
    return intersection(line).isValid();
}

/*!
  \internal
  Use a kind of cheap hill-climbing approach to find the point intersection,
  where the heuristic is the value of t, the scalar representing the distance
  from the line's origin point to the intersection under consideration.  If
  the t for the intersection being considered is less than t's for other
  intersections then it might be a solution, in which case do the calculation
  to find the point Q = P + tV on the line.  Once the point is found, test it
  to see if it really is a solution (lies in the box) before updating the
  heuristic.
 */
inline static void trackIntersection(qreal t, qreal *closest_t, QVector3D *closest_p,
                         const QBox3D &box, const QLine3D &line)
{
    if ((t > 0.0f && t < *closest_t) || qIsNull(*closest_t))
    {
        // only consider non-negative values of t, and ones better than best
        // value found so far
        QVector3D p = line.point(t);
        if (box.contains(p))
        {
            *closest_p = p;
            *closest_t = t;
        }
    }
}

/*!
  Returns the point at which the \a line intersects this box.
  The result is returned as a QResult<QVector3D> instance.

  When the line intersects the box at least once, a QResult<QVector3D>
  instance is returned where QResult::isValid() returns true; and
  where QResult::value() returns a QVector3D containing the intersection
  point.

  In the case where the origin point of the line lies on one of the box
  faces, this point is simply returned (with no floating point operations).

  The line might intersect at two points - as the line passes through
  the box - one on the near side, one on the far side; where near and far
  are relative to the origin point of the line.  This function only
  returns the near intersection point.

  Only positive values on the line are considered, that is if
  \c{qreal t == line.point(intersectionPoint)} then \bold t is positive.

  This means that if the origin point of the line is inside the box, there
  is only one solution, not two.  To get the other solution, simply change
  the sign of the lines direction vector.

  When the line does not intersect the box, the result returned will have
  a QResult::isValid() value of false.  In the case of an infinite box
  the result reflects that with QResult::OutOfRange
 */
QResult<QVector3D> QBox3D::intersection(const QLine3D &line) const
{
    if (boxtype == Null)
        return QResult<QVector3D>();
    if (boxtype == Infinite)
        return QResult<QVector3D>(QResult<QVector3D>::OutOfRange);

    QVector3D org = line.origin();
    Partition xpos, ypos, zpos;
    partition(org, &xpos, &ypos, &zpos);
    // if the lines origin lies on one of the faces, return it as the intersection
    if ((xpos | ypos | zpos) == (equalMin | equalMax))
        return org;

    // Could use the line/plane intersection functions, with 6 box planes defined by
    // 3 normals at the min and 3 at the max.  But since the planes are axis-aligned
    // there is a cheap optimization.  If the line P + tV intersects a AA-plane with
    // x = c at intersection point X, then P.x + tV.x = X.x = c; and therefore
    // t = (c - P.x) / V.x.  Here the value t is the scalar distance along V from P and
    // so it measures the value of the solution - see trackIntersection() above.
    qreal closest_t = 0.0f;
    QVector3D ln = line.direction();
    QVector3D closest_p;
    for (const QVector3D *p = &maxcorner; p; p = (p == &maxcorner) ? &mincorner : 0)
    {
        if (!qIsNull(ln.x()))
            trackIntersection(p->x() - org.x() / ln.x(), &closest_t, &closest_p, *this, line);
        if (!qIsNull(ln.y()))
            trackIntersection(p->y() - org.y() / ln.y(), &closest_t, &closest_p, *this, line);
        if (!qIsNull(ln.z()))
            trackIntersection(p->z() - org.z() / ln.z(), &closest_t, &closest_p, *this, line);
    }
    return closest_p;
}

/*!
    Intersects this box with \a box.

    \sa intersected(), intersects(), expand()
*/
void QBox3D::intersect(const QBox3D& box)
{
    // Handle the simple cases first.
    if (boxtype == Null) {
        // Null intersected with anything is null.
        return;
    } else if (boxtype == Infinite) {
        // Infinity intersected with a box is that box.
        *this = box;
        return;
    } else if (box.boxtype == Null) {
        // Anything intersected with null is null.
        setNull();
        return;
    } else if (box.boxtype == Infinite) {
        // Box intersected with infinity is the box.
        return;
    }

    // Intersect two finite boxes.
    QVector3D min1 = mincorner;
    QVector3D max1 = maxcorner;
    QVector3D min2 = box.mincorner;
    QVector3D max2 = box.maxcorner;
    if (min2.x() > min1.x())
        min1.setX(min2.x());
    if (min2.y() > min1.y())
        min1.setY(min2.y());
    if (min2.z() > min1.z())
        min1.setZ(min2.z());
    if (max2.x() < max1.x())
        max1.setX(max2.x());
    if (max2.y() < max1.y())
        max1.setY(max2.y());
    if (max2.z() < max1.z())
        max1.setZ(max2.z());
    if (min1.x() > max1.x() || min1.y() > max1.y() || min1.z() > max1.z()) {
        setNull();
    } else {
        mincorner = min1;
        maxcorner = max1;
    }
}

/*!
    Returns a new box which is the intersection of this box with \a box.

    \sa intersect(), intersects(), expanded()
*/
QBox3D QBox3D::intersected(const QBox3D& box) const
{
    QBox3D result(*this);
    result.intersect(box);
    return result;
}

/*!
    Expands this box so that it also includes \a point.  If \a point
    is already contained within this box, then this box will be unchanged.

    \sa expanded(), intersect()
*/
void QBox3D::expand(const QVector3D& point)
{
    if (boxtype == Finite) {
        if (point.x() < mincorner.x())
            mincorner.setX(point.x());
        else if (point.x() > maxcorner.x())
            maxcorner.setX(point.x());
        if (point.y() < mincorner.y())
            mincorner.setY(point.y());
        else if (point.y() > maxcorner.y())
            maxcorner.setY(point.y());
        if (point.z() < mincorner.z())
            mincorner.setZ(point.z());
        else if (point.z() > maxcorner.z())
            maxcorner.setZ(point.z());
    } else if (boxtype == Null) {
        boxtype = Finite;
        mincorner = point;
        maxcorner = point;
    }
}

/*!
    Expands this box so that it also includes \a box.  If \a box
    is already contained within this box, then this box will be unchanged.

    \sa expanded(), intersect()
*/
void QBox3D::expand(const QBox3D& box)
{
    if (box.boxtype == Finite) {
        expand(box.minimum());
        expand(box.maximum());
    } else if (box.boxtype == Infinite) {
        setInfinite();
    }
}

/*!
    Returns a new box which expands this box so that it also includes
    \a point.  The returned value will be the smallest box that contains
    both this box and \a point.

    \sa expand(), intersected()
*/
QBox3D QBox3D::expanded(const QVector3D& point) const
{
    if (boxtype == Finite) {
        QBox3D result(*this);
        result.expand(point);
        return result;
    } else if (boxtype == Null) {
        return QBox3D(point, point);
    } else {
        return *this;
    }
}

/*!
    Returns a new box which expands this box so that it also includes
    \a box.  The returned value will be the smallest box that contains
    both this box and \a box.

    \sa expand(), intersected()
*/
QBox3D QBox3D::expanded(const QBox3D& box) const
{
    if (boxtype == Finite) {
        QBox3D result(*this);
        result.expand(box);
        return result;
    } else if (boxtype == Null) {
        return box;
    } else {
        return *this;
    }
}

/*!
    Translates this box by \a vector.

    \sa translated(), scale(), transform()
*/
void QBox3D::translate(const QVector3D& vector)
{
    if (boxtype == Finite) {
        mincorner += vector;
        maxcorner += vector;
    }
}

/*!
    Returns the resuls of translating this box by \a vector.

    \sa translate(), scaled(), transformed()
*/
QBox3D QBox3D::translated(const QVector3D& vector) const
{
    if (boxtype == Finite)
        return QBox3D(mincorner + vector, maxcorner + vector);
    else
        return *this;
}

/*!
    Scales this box by the components of \a vector.

    The minimum() and maximum() extents are multiplied by \a vector.
    To scale a box about its center(), use the following:

    \code
    QVector3D c = box.center();
    box.translate(-c);
    box.scale(vector);
    box.translate(c);
    \endcode

    \sa scaled(), translate(), transform()
*/
void QBox3D::scale(const QVector3D& vector)
{
    if (boxtype == Finite)
        setExtents(mincorner * vector, maxcorner * vector);
}

/*!
    Scales this box by \a factor.

    The minimum() and maximum() extents are multiplied by \a factor.
    To scale a box about its center(), use the following:

    \code
    QVector3D c = box.center();
    box.translate(-c);
    box.scale(factor);
    box.translate(c);
    \endcode

    \sa scaled(), translate(), transform()
*/
void QBox3D::scale(qreal factor)
{
    if (boxtype == Finite)
        setExtents(mincorner * factor, maxcorner * factor);
}

/*!
    Returns this box scaled by the components of \a vector.

    The minimum() and maximum() extents of this box are multiplied
    by \a vector.  To scale a box about its center(), use the following:

    \code
    QVector3D c = box.center();
    QBox3D result = box.translated(-c);
    result.scale(vector);
    result.translate(c);
    \endcode

    \sa scale(), translated(), transformed()
*/
QBox3D QBox3D::scaled(const QVector3D& vector) const
{
    if (boxtype == Finite)
        return QBox3D(mincorner * vector, maxcorner * vector);
    else
        return *this;
}

/*!
    Returns this box scaled by \a factor.

    The minimum() and maximum() extents of this box are multiplied
    by \a factor.  To scale a box about its center(), use the following:

    \code
    QVector3D c = box.center();
    QBox3D result = box.translated(-c);
    result.scale(factor);
    result.translate(c);
    \endcode

    \sa scale(), translated(), transformed()
*/
QBox3D QBox3D::scaled(qreal factor) const
{
    if (boxtype == Finite)
        return QBox3D(mincorner * factor, maxcorner * factor);
    else
        return *this;
}

/*!
    Transforms this box according to \a matrix.  Each of the 8 box
    corners are transformed and then a new box that encompasses all
    of the transformed corner values is created.

    The scale() and translate() functions are more efficient than
    this function if the transformation is a simple scale or translate.

    \sa transformed(), scaled(), translated()
*/
void QBox3D::transform(const QMatrix4x4& matrix)
{
    *this = transformed(matrix);
}

/*!
    Returns this box transformed by \a matrix.  Each of the 8 box
    corners are transformed and then a new box that encompasses all
    of the transformed corner values is returned.

    The scaled() and translated() functions are more efficient than
    this function if the transformation is a simple scale or translate.

    \sa transform(), scaled(), translated()
*/
QBox3D QBox3D::transformed(const QMatrix4x4& matrix) const
{
    if (boxtype != Finite)
        return *this;
    QBox3D result;
    result.expand(matrix * mincorner);
    result.expand(matrix * QVector3D(mincorner.x(), mincorner.y(), maxcorner.z()));
    result.expand(matrix * QVector3D(mincorner.x(), maxcorner.y(), maxcorner.z()));
    result.expand(matrix * QVector3D(mincorner.x(), maxcorner.y(), mincorner.z()));
    result.expand(matrix * QVector3D(maxcorner.x(), mincorner.y(), mincorner.z()));
    result.expand(matrix * QVector3D(maxcorner.x(), maxcorner.y(), mincorner.z()));
    result.expand(matrix * QVector3D(maxcorner.x(), mincorner.y(), maxcorner.z()));
    result.expand(matrix * maxcorner);
    return result;
}

/*!
    \fn bool QBox3D::operator==(const QBox3D& box) const

    Returns true if this box is identical to \a box.
*/

/*!
    \fn bool QBox3D::operator!=(const QBox3D& box) const

    Returns true if this box is not identical to \a box.
*/

void QBox3D::partition
    (const QVector3D &point, Partition *xpart,
     Partition *ypart, Partition *zpart) const
{
    *xpart = point.x() < mincorner.x() ? belowMin : (
            point.x() == mincorner.x() ? equalMin : (
                    point.x() < maxcorner.x() ? between : (
                            point.x() == maxcorner.x() ? equalMax : aboveMax)));
    *ypart = point.y() < mincorner.y() ? belowMin : (
            point.y() == mincorner.y() ? equalMin : (
                    point.y() < maxcorner.y() ? between : (
                            point.y() == maxcorner.y() ? equalMax : aboveMax)));
    *zpart = point.z() < mincorner.z() ? belowMin : (
            point.z() == mincorner.z() ? equalMin : (
                    point.z() < maxcorner.z() ? between : (
                            point.z() == maxcorner.z() ? equalMax : aboveMax)));
}

/*!
    \fn bool qFuzzyCompare(const QBox3D& box1, const QBox3D& box2)
    \relates QBox3D

    Returns true if \a box1 and \a box2 are almost equal; false otherwise.
*/

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug dbg, const QBox3D &box)
{
    if (box.isFinite()) {
        dbg.nospace() << "QBox3D(("
            << box.minimum().x() << ", " << box.minimum().y() << ", "
            << box.minimum().z() << ") - ("
            << box.maximum().x() << ", " << box.maximum().y() << ", "
            << box.maximum().z() << ')';
        return dbg.space();
    } else if (box.isNull()) {
        dbg << "QBox3D(null)";
        return dbg;
    } else {
        dbg << "QBox3D(infinite)";
        return dbg;
    }
}

#endif

QT_END_NAMESPACE
