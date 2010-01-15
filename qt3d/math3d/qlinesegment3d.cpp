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

#include "qlinesegment3d.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \class QLineSegment3D
    \brief The QLineSegment3D class models the mathematics of line between two points in 3D space.
    \since 4.6
    \ingroup qt3d
    \ingroup qt3d::math

    QLineSegment is a finite portion of a QLine3D, represented by its
    start() and end() points.  The segment can be constructed as a QLine3D
    plus its length; or by supplying the two end points.

    The containment and intersection properties of the line segment are a
    strict subset of those for a QLine3D.  An intersection or containment
    only occurs on the line segment if it does for the underlying line, and
    the solution lies between the start() and the end() points, inclusive.

    It is possible for a line segment to be a single point with a zero
    length().  Such a line segment occurs when the start() and the end()
    points are the same.  The isPoint() function will return true
    if the line segment is a point; false for non-zero length line segments.

    The toLine3D() function can be used to convert a line segment
    into an infinite line that coincides with the line segment.

    \sa QLine3D
*/

/*!
    \fn QLineSegment3D::QLineSegment3D()

    Constructs a default line segment with both start() and end()
    set to null.

    \sa isPoint()
*/

/*!
    Constructs a line segment from the origin of \a line and progressing
    along its direction by \a length units.

    \sa start(), end()
*/
QLineSegment3D::QLineSegment3D(const QLine3D &line, qreal length)
    : m_start(line.origin()), m_end(line.point(length))
{
}

/*!
    \fn QLineSegment3D::QLineSegment3D(const QVector3D &start, const QVector3D &end)

    Constructs a line segment from the point \a start to the point \a end.

    \sa start(), end()
*/

/*!
    \fn QVector3D QLineSegment3D::start() const

    Returns the start of this line segment.

    \sa setStart(), end()
*/

/*!
    \fn QLineSegment3D::setStart(const QVector3D &value)

    Sets the start of this line segment to \a value.

    \sa start(), setEnd()
*/

/*!
    \fn QVector3D QLineSegment3D::end() const

    Returns the end of this line segment.

    \sa setEnd(), start()
*/

/*!
    \fn void QLineSegment3D::setEnd(const QVector3D &value)

    Sets the end of this line segment to \a value.

    \sa end(), setStart()
*/

/*!
    \fn QVector3D QLineSegment3D::direction() const

    Returns the direction of the line segment, which is the vector
    from start() to end(), normalized to a unit vector.  If start()
    and end() are the same, then a null vector is returned.

    \sa vector()
*/

/*!
    \fn QVector3D QLineSegment3D::vector() const

    Returns the vector from start() to end().

    \sa direction(), length()
*/

/*!
    \fn QLine3D QLineSegment3D::toLine3D() const

    Returns the infinite line on which this segment lies, based at
    start() and extending along direction().
*/

/*!
    \fn qreal QLineSegment3D::length() const

    Returns the length of the line segment from start() to end().

    \sa vector()
*/

/*!
    \fn bool QLineSegment3D::isPoint() const

    Returns true if this line segment consists of a single point,
    that is start() is the same as end(); false otherwise.
*/

/*!
    Returns true if \a point lies on this line segment between start()
    and end(), inclusive.  A zero-length line segument contains only a
    single point.

    \sa intersects()
*/
bool QLineSegment3D::contains(const QVector3D &point) const
{
    if (point == m_start || point == m_end)
        return true;
    else if (m_start == m_end)
        return false;

    // Determine if the point lies on the infinite version of the line.
    // This is true if the cross product of the vector to the point with
    // the direction of the line is zero.
    QVector3D v1 = point - m_start;
    QVector3D v2 = m_end - m_start;
    QVector3D cross = QVector3D::crossProduct(v1, v2);
    if (!qFuzzyCompare(cross, QVector3D(0.0f, 0.0f, 0.0f)))
        return false;

    // Determine how far along the direction vector the point is.
    // We use the largest co-ordinate to determine the t for the line.
    // If it is between 0 and 1, then it is on the line segment.
    qreal t;
    if (qAbs(v2.x()) >= qAbs(v2.y()) && qAbs(v2.x()) >= qAbs(v2.z()))
        t = v1.x() / v2.x();
    else if (qAbs(v2.y()) >= qAbs(v2.z()))
        t = v1.y() / v2.y();
    else
        t = v1.z() / v2.z();
    return (t >= 0.0f && t <= 1.0f);
}

/*!
    Returns true if \a segment lies entirely within this line segment.
    This is true if both the start() and end() points of \a segment are
    contained in this line segment.

    \sa intersects()
*/
bool QLineSegment3D::contains(const QLineSegment3D &segment) const
{
    return contains(segment.m_start) && contains(segment.m_end);
}

/*!
    Returns true if \a line uniquely intersects with this
    line segment; false otherwise.

    If this line segment is zero-length, then its start() point
    must be contained within \a line for a unique intersection.

    If this line segment is not zero-length and is contained in \a line,
    then this function will return false because the intersection
    is not unique.

    \sa intersection(), contains()
*/
bool QLineSegment3D::intersects(const QLine3D &line) const
{
    if (m_start == m_end) {
        return line.contains(m_start);
    } else {
        QResult<QVector3D> res = toLine3D().intersection(line);
        return res.isValid() && contains(res.value());
    }
}

/*!
    Returns true if \a segment uniquely intersects with this
    line segment; false otherwise.

    If either this line segment or \a segment is zero-length,
    then its start() point must be contained in the other segment
    for a unique intersection.

    If the line segments are not zero-length and this line and
    \a segment coincide, then this function will return false
    because the intersection is not unique.

    \sa intersection(), contains()
*/
bool QLineSegment3D::intersects(const QLineSegment3D &segment) const
{
    if (m_start == m_end) {
        return segment.contains(m_start);
    } else if (segment.m_start == segment.m_end) {
        return contains(segment.m_start);
    } else {
        QResult<QVector3D> res = toLine3D().intersection(segment.toLine3D());
        return res.isValid() && contains(res.value()) &&
               segment.contains(res.value());
    }
}

/*!
    Returns the unique intersection of this line segment with \a line.

    The caller can invoke QResult::isValid() on the return
    value to determine if an intersection actually exists.  If it does,
    then QResult::value() will return the intersection point.

    \sa intersects()
*/
QResult<QVector3D> QLineSegment3D::intersection(const QLine3D &line) const
{
    if (m_start == m_end) {
        if (line.contains(m_start))
            return m_start;
        else
            return QResult<QVector3D>::NoResult;
    } else {
        QResult<QVector3D> res = toLine3D().intersection(line);
        if (res.isValid() && contains(res.value()))
            return res;
        else
            return QResult<QVector3D>::NoResult;
    }
}

/*!
    Returns the unique intersection of this line segment with \a segment.

    The caller can invoke QResult::isValid() on the return
    value to determine if an intersection actually exists.  If it does,
    then QResult::value() will return the intersection point.

    \sa intersects()
*/
QResult<QVector3D> QLineSegment3D::intersection(const QLineSegment3D &segment) const
{
    if (m_start == m_end) {
        if (segment.contains(m_start))
            return m_start;
        else
            return QResult<QVector3D>::NoResult;
    } else if (segment.m_start == segment.m_end) {
        if (contains(segment.m_start))
            return segment.m_start;
        else
            return QResult<QVector3D>::NoResult;
    } else {
        QResult<QVector3D> res = toLine3D().intersection(segment.toLine3D());
        if (res.isValid() && contains(res.value()) &&
                segment.contains(res.value()))
            return res;
        else
            return QResult<QVector3D>::NoResult;
    }
}

QT_END_NAMESPACE
