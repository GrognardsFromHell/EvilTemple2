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

#ifndef QPLANE3D_H
#define QPLANE3D_H

#include <QtGui/qvector3d.h>
#include "qresult.h"
#include "qline3d.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3d)

class Q_QT3D_EXPORT QPlane3D
{
public:
    QPlane3D();
    QPlane3D(const QVector3D &point, const QVector3D &normal);
    QPlane3D(const QVector3D &p, const QVector3D &q, const QVector3D &r);

    QVector3D origin() const;
    void setOrigin(const QVector3D& value);

    QVector3D normal() const;
    void setNormal(const QVector3D& value);

    bool contains(const QVector3D &point) const;
    bool contains(const QLine3D &line) const;

    bool intersects(const QLine3D &line) const;
    QResult<QVector3D> intersection(const QLine3D &line) const;

    QVector3D project(const QVector3D &point) const;
    QLine3D project(const QLine3D &line) const;
    bool sameSide(const QVector3D &pointA, const QVector3D &pointB) const;

private:
    QVector3D m_origin; // a point on the plane
    QVector3D m_normal; // a normal, perpendicular to the plane
};

inline QPlane3D::QPlane3D()
    : m_normal(1.0f, 0.0f, 0.0f)
{
}

inline QPlane3D::QPlane3D(const QVector3D &point, const QVector3D &normal)
    : m_origin(point)
{
    setNormal(normal);
}

inline QPlane3D::QPlane3D(const QVector3D &p, const QVector3D &q, const QVector3D &r)
    : m_origin(p)
{
    setNormal(QVector3D::crossProduct(q-p, r-q));
}

inline QVector3D QPlane3D::origin() const
{
    return m_origin;
}

inline void QPlane3D::setOrigin(const QVector3D &value)
{
    m_origin = value;
}

inline QVector3D QPlane3D::normal() const
{
    return m_normal;
}

inline void QPlane3D::setNormal(const QVector3D& value)
{
    // ensure normal is a unit vector
    if (value != m_normal)
        m_normal = value.normalized();
}

inline bool QPlane3D::contains(const QVector3D &point) const
{
    return qIsNull(QVector3D::dotProduct(m_normal, m_origin - point));
}

inline bool QPlane3D::contains(const QLine3D &line) const
{
    return qIsNull(QVector3D::dotProduct(m_normal, line.direction())) &&
            contains(line.origin());
}

inline bool QPlane3D::intersects(const QLine3D &line) const
{
    return !qIsNull(QVector3D::dotProduct(m_normal, line.direction()));
}

inline QLine3D QPlane3D::project(const QLine3D &line) const
{
    return QLine3D(project(line.origin()), project(line.direction()));
}

inline bool QPlane3D::sameSide(const QVector3D &pointA, const QVector3D &pointB) const
{
    qreal dpA = QVector3D::dotProduct(pointA - m_origin, m_normal);
    qreal dpB = QVector3D::dotProduct(pointB - m_origin, m_normal);
    if (qIsNull(dpA) || qIsNull(dpB))
        return false;
}

QT_END_NAMESPACE

#ifndef QT_NO_PLANE3D
Q_DECLARE_METATYPE(QPlane3D)
#endif

QT_END_HEADER

#endif // QPLANE3D_H
