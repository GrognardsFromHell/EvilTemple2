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

#include "qline3d.h"
#include "qplane3d.h"

#include <QtCore/qmath.h>

/*!
  \class QLine3D
  \brief The QLine3D class models the mathematics of lines in 3D space.
  \since 4.6
  \ingroup qt3d
  \ingroup qt3d::math

  A line is defined by:

  \list
  \o a point, which can be thought of as an origin point.
  \o a direction vector, giving the heading of the line from the point.
  \endlist

  A line is infinite.

  It can be thought of as a one-dimensional co-ordinate system.  If the single
  co-ordinate is \bold t then the system has the \bold origin point, where
  \c{t == 0.0f}, and the \bold direction vector pointing in the direction of
  increasing values of the t-coordinate.

  Construct a line with QLine3D(const QVector3D &origin, const QVector3D &direction)
  where direction is a non-null vector, and origin is a point.

  If a null vector is supplied for direction, then the \bold {behaviour of the QLine3D is undefined}.

  The default constructor for QLine3D() arbitrarily chooses \c{(1.0, 0.0, 0.0)} as
  the direction vector.

  The point() method returns a point at the tip of a ray from the origin, lying on the
  line, where the length of the ray is the amount \bold t.  The sign of \bold t
  distinguishes which direction (same or opposite) the ray points, with respect to
  the direction vector.

  Another way to think of this, is that the point() method allows for parametric
  generation of all points on the line, as \bold t varies from negative infinity
  to positive infinity.

  \code
    QLine3D cameraAxis(objectCenter, viewingOrientation);
    QVector3D viewPoint = line.point(viewingDistance);
    QVector3D cameraVector = -viewingOrientation;
  \endcode

  From this definition it can be seen that this primitive is in fact a line (not a ray
  or a line-segment), since t can take on infinitely many positive or negative values.

  For more details of the math of QLine3D see \l {3D Math Basis}
 */

/*!
  \fn QLine3D::QLine3D()
  Construct a default QLine3D with a defining point at the origin.
  The direction vector cannot be the null vector and is thus set to a
  unit vector on the X-axis.
 */

/*!
  \fn QLine3D::QLine3D(const QVector3D &origin, const QVector3D &direction)
  Construct a QLine3D given its defining \a origin and \a direction.

  If the direction vector is null, then behaviour is undefined.

  To construct a line that passes through two given points, use this code:
  \code
  QLine3D thruAB(pointA, pointB - pointA);
  \endcode

  If you want a finite line, which starts at A and ends at B, use the
  QLineSegment class.
 */

/*!
  \fn QVector3D QLine3D::origin() const
  Returns the origin of this line as a QVector3D point.
 */

/*!
  \fn void QLine3D::setOrigin(const QVector3D &value)
  Sets the origin point of this line to \a value.
 */

/*!
  \fn QVector3D QLine3D::direction() const
  Returns the direction vector of this line.
 */

/*!
  \fn void QLine3D::setDirection(const QVector3D &direction)
  Sets the direction vector of this line to \a direction.

  If the direction vector is null, then behaviour of the QLine3D is \bold undefined.
 */

/*!
  \fn bool QLine3D::contains(const QVector3D &point) const
  Returns true if the \a point given lies on this line; and false otherwise.
 */

/*!
  \fn bool QLine3D::contains(const QLine3D &line) const
  Returns true if the \a line lies on this line; and false otherwise.  If true,
  this implies that the two lines are the same line, however they may define
  different 1D coordinate systems in that they may have different origin points,
  and the direction may be inverted.
 */

/*!
  Returns true if the \a line intersects with this line, false otherwise.
 */
bool QLine3D::intersects(const QLine3D &line) const
{
    // if the dot-product is 1.0 then the lines are parallel
    qreal dotDirections = QVector3D::dotProduct(m_direction, line.direction());
    if (qFuzzyCompare(qreal(1.0f), dotDirections))
        return false;
    QVector3D ppVec = line.origin() - m_origin;
    if (ppVec.isNull())
        return true;
    QVector3D planeNormal = QVector3D::crossProduct(ppVec, m_direction);
    return qIsNull(QVector3D::dotProduct(planeNormal, line.direction()));
}

#ifndef QT_NO_DEBUG_STREAM
/*
static void dumpMatrix(qreal **a, qreal row, qreal col)
{
    QString astr = QString("A: about to process row %1, col %2\n").arg(row).arg(col);
    QString buf;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            astr += buf.sprintf("   %0.10f", a[i][j]);
        astr += "\n";
    }
    qDebug() << astr;
}
*/
#endif

/*!
  Calculates the intersection of this line with the given \a line.
  If this line and the given \a line do not intersect, then the
  QResult<QVector3D> returned will return false from QResult<QVector3D>::isValid(),
  and the QResult<QVector3D>::value() will return a null QVector3D.

  In this example the intersection() function is used to find if a point is
  inside a triangle:
  \code
  QVector3D p, q, r;
  loadTriangle(&p, &q, &r);  // known to valid triangle
  QVector3D point = userClick();
  QLine3D base(p, q - p);
  QLine3D apexToPoint(r, point - r);
  // know apriori that the lines intersect
  QVector3D proj = base.intersection(apexToPoint);
  return base.distanceFromOrigin(proj) <= base.distanceFromOrigin(m_q) &&
            apexToPoint.distanceFromOrigin(point) <= apexToPoint.distanceFromOrigin(proj);
  \endcode

  If the direction vector of either this or \a line is null, then
  QResult::InvalidArgument is returned.

  \sa {qt3d-math-basis.html#vector-and-point-arithmetic}
 */
QResult<QVector3D> QLine3D::intersection(const QLine3D &line) const
{
    // if one or both direction vectors are null, bogus!
    if (m_direction.isNull() || line.direction().isNull())
        return QResult<QVector3D>(QResult<QVector3D>::InvalidArgument);

    // Do gaussian elimination with a m x n matrix
    const int maxRows = 3;   // m - data rows
    const int maxCols = 2;   // n - data columns
    const int augmented = 3; // augmented columns == rhs of eqn
    qreal *a[maxRows];                   // solution matrix row pointers
    qreal b[maxRows][augmented];         // solution matrix row contents
    b[0][0] = m_direction.x();
    b[0][1] = -line.direction().x();
    b[0][2] = line.origin().x() - m_origin.x();
    a[0] = b[0];
    b[1][0] = m_direction.y();
    b[1][1] = -line.direction().y();
    b[1][2] = line.origin().y() - m_origin.y();
    a[1] = b[1];
    b[2][0] = m_direction.z();
    b[2][1] = -line.direction().z();
    b[2][2] = line.origin().z() - m_origin.z();
    a[2] = b[2];

    int row = 0;
    int col = 0;

    // dumpMatrix(a, row, col);
    while (row < maxRows && col < maxCols)
    {
        int max = row;
        for (int k = row + 1; k < maxRows; ++k)
            if (qAbs(a[k][col]) > qAbs(a[max][col]))
                max = k;
        if (a[max][col] != 0)
        {
            if (max != row)
                qSwap(a[row], a[max]);
            for (int w = col + 1; w < augmented; ++w)
                a[row][w] /= a[row][col];
            a[row][col] = 1.0;
            for (int u = row + 1; u < maxRows; ++u)  // eliminate
            {
                for (int w = col + 1; w < augmented; ++w)
                    a[u][w] -= a[row][w] * a[u][col];
                a[u][col] = 0.0;
            }
            ++row;
        }
        ++col;
        // dumpMatrix(a, row, col);
    }
    qreal s;
    // If there is a solution then the last row of the matrix will have
    // been reduced to all zeroes
    if ((qAbs(a[2][2]) < 0.00001f) && a[1][1] == 1.0)
        s = a[1][2];
    else
        return QResult<QVector3D>();

    QVector3D ps = line.origin() + (line.direction() * s);
    return ps;
}

/*!
  \fn QVector3D QLine3D::point(qreal t) const
  Returns the point on the line defined by moving a distance \a t along the line
  in the direction of the direction vector.  Note that \a t may be negative in
  which case the point returned will lie "behind" the origin point with respect to
  the direction vector.

  The magnitude of the direction vector plays no role in the result.  The scalar
  t determines the distance, while the direction vector determines the orientation.

  Internally the direction vector is normalized to a unit vector and scaled.
 */

/*!
  \fn QVector3D QLine3D::project(const QVector3D &vector) const
  Returns the projection of the \a vector in this line.

  \image qline3d-project.png

  Consider vector V as the sum of two perpendicular vectors: \c{V = V' + V''}
  then project(V) gives V', the component of V in the direction of this line.
  V'' is the component of V which is perpendicular to this line.

  Imagine a 3D model of a robotic arm lifting a weight in uniform gravity.
  The effort applied by the fore-arm of the robot depends on the angle of the
  forearm compared to the down-vector:
  \code
    QLine3D forearm(elbowSite, forearmOrientation);
    QVector3D gravityForce = forearm.project(worldModel->getDownVector());
  \endcode
 */

/*!
  \fn qreal QLine3D::distance(const QVector3D &point) const
  Returns the distance of a \a point from this line.  This is the minimum distance
  of the point from the line, or equivalently, the length of a line perpendicular
  to this line which passes through the point.
 */

/*!
  \fn qreal QLine3D::distanceFromOrigin(const QVector3D &point) const
  Returns the distance of a \a point from the origin of this line.  There is
  no test that the point is on the line: this function is exactly equivalent to:
  \code
  QVector3D vec = point - line.origin();
  myDistanceFromOrigin = vec.length();
  \endcode
  If the point is known to be on the line, then this method is the inverse of
  the point(qreal t) function.
  \sa point()
 */

/*!
  \fn bool QLine3D::sameSide(const QVector3D &pointA, const QVector3D &pointB) const
  Returns true if \a pointA and \a pointB are co-planar and lie on the same side of
  this line; false otherwise.
 */

/*!
  \fn bool QLine3D::operator==(const QLine3D &rhs)
  Returns true if this line is the same as \a rhs; that is they are collinear and have the same
  direction and origin.
 */

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug dbg, const QLine3D &line)
{
    if (!line.direction().isNull()) {
        dbg.nospace() << "QLine3D(origin("
            << line.origin().x() << ", " << line.origin().y() << ", "
            << line.origin().z() << ") - direction("
            << line.direction().x() << ", " << line.direction().y() << ", "
            << line.direction().z() << ')';
        return dbg.space();
    } else {
        dbg << "QLine3D(invalid - null direction)";
        return dbg;
    }
}

#endif
