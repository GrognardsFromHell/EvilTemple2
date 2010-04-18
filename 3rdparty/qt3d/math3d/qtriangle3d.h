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

#ifndef QTRIANGLE3D_H
#define QTRIANGLE3D_H

#include <QtGui/qvector3d.h>
#include "qresult.h"
#include "qline3d.h"
#include "qplane3d.h"
#include "qlinesegment3d.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3d)

class Q_QT3D_EXPORT QTriangle3D
{
public:
    QTriangle3D();
    QTriangle3D(const QVector3D &p, const QVector3D &q, const QVector3D &r);

    QVector3D p() const;
    void setP(const QVector3D& point);

    QVector3D q() const;
    void setQ(const QVector3D& point);

    QVector3D r() const;
    void setR(const QVector3D& point);

    qreal width() const;
    qreal height() const;
    qreal area() const;

    bool isDegenerate() const;
    bool isNull() const;

    QPlane3D plane() const;
    QVector3D center() const;
    QVector3D faceNormal() const;

    bool contains(const QVector3D &point) const;

    bool intersects(const QLine3D &line) const;
    QResult<QVector3D> intersection(const QLine3D &line) const;

    QVector3D interpolate(qreal s, qreal t) const;

private:
    QVector3D m_p, m_q, m_r;   // vertex points of a triangle in 3D space
};

inline QTriangle3D::QTriangle3D()
    : m_p(0.0f, 0.0f, 0.0f)
    , m_q(1.0f, 0.0f, 0.0f)
    , m_r(0.0f, 1.0f, 0.0f) {}

inline QTriangle3D::QTriangle3D(const QVector3D &p, const QVector3D &q, const QVector3D &r)
    : m_p(p)
    , m_q(q)
    , m_r(r) {}

inline QVector3D QTriangle3D::p() const
{
    return m_p;
}

inline void QTriangle3D::setP(const QVector3D &point)
{
    m_p = point;
}

inline QVector3D QTriangle3D::q() const
{
    return m_q;
}

inline void QTriangle3D::setQ(const QVector3D &point)
{
    m_q = point;
}

inline QVector3D QTriangle3D::r() const
{
    return m_r;
}

inline void QTriangle3D::setR(const QVector3D &point)
{
    m_r = point;
}

inline qreal QTriangle3D::width() const
{
    return (m_q - m_p).length();
}

inline qreal QTriangle3D::height() const
{
    return QLine3D(m_p, m_q - m_p).distance(m_r);
}

inline qreal QTriangle3D::area() const
{
    return 0.5f * width() * height();
}

inline bool QTriangle3D::isDegenerate() const
{
    return qIsNull(width()) || qIsNull(height());
}

inline bool QTriangle3D::isNull() const
{
    return m_p == m_q && m_p == m_r;
}

inline QPlane3D QTriangle3D::plane() const
{
    return QPlane3D(m_p, m_q, m_r);
}

inline QVector3D QTriangle3D::center() const
{
    return (m_p + m_q + m_r) / 3.0f;
}

inline QVector3D QTriangle3D::faceNormal() const
{
    return QVector3D::crossProduct(m_q - m_p, m_r - m_q);
}

inline bool QTriangle3D::intersects(const QLine3D &line) const
{
    if (qFuzzyCompare(QVector3D::dotProduct(m_q - m_p, line.direction()), qreal(1.0f)))
        return false;
    return contains(plane().intersection(line));
}

inline QResult<QVector3D> QTriangle3D::intersection(const QLine3D &line) const
{
    if (qFuzzyCompare(QVector3D::dotProduct(m_q - m_p, line.direction()), qreal(1.0f)))
        return QResult<QVector3D>();
    QVector3D inter = plane().intersection(line);
    if (contains(inter))
        return inter;
    return QResult<QVector3D>();
}

inline QVector3D QTriangle3D::interpolate(qreal s, qreal t) const
{
    QLineSegment3D base(m_p, m_q);
    QVector3D inter = base.toLine3D().point(t);
    QLineSegment3D apexToPoint(m_r, inter);
    return apexToPoint.toLine3D().point(s);
    // afaict this method from the parametric definition of a plane using three points
    // actually results in a bias, such that the s coefficient is out of proportion to t.
    // return (1.0f - s) * (((1.0f - t) * m_p) + (t * m_q)) + s * m_r;
}

QT_END_NAMESPACE

#ifndef QT_NO_PLANE3D
Q_DECLARE_METATYPE(QTriangle3D)
#endif

QT_END_HEADER

#endif // QTRIANGLE3D_H
