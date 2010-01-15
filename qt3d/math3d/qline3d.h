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

#ifndef QLINE3D_H
#define QLINE3D_H

#include "qt3dglobal.h"
#include <QtGui/qvector3d.h>
#include "qresult.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3d)

class QPlane3D;

class Q_QT3D_EXPORT QLine3D
{
public:
    QLine3D();
    QLine3D(const QVector3D &origin, const QVector3D &direction);
    QVector3D origin() const;
    void setOrigin(const QVector3D& value);

    QVector3D direction() const;
    void setDirection(const QVector3D& value);

    bool contains(const QVector3D &point) const;
    bool contains(const QLine3D &line) const;
    bool intersects(const QLine3D &line) const;
    QResult<QVector3D> intersection(const QLine3D &line) const;

    QVector3D point(qreal t) const;
    QVector3D project(const QVector3D &vector) const;
    qreal distance(const QVector3D &point) const;
    qreal distanceFromOrigin(const QVector3D &point) const;
    bool sameSide(const QVector3D &pointA, const QVector3D &pointB) const;

    bool operator==(const QLine3D &rhs);
private:
    QVector3D m_origin;
    QVector3D m_direction;

};

inline QLine3D::QLine3D()
    : m_direction(1.0f, 0.0f, 0.0f) {}

inline QLine3D::QLine3D(const QVector3D &origin, const QVector3D &direction)
    : m_origin(origin)
    , m_direction(direction)
{
}

inline QVector3D QLine3D::origin() const
{
    return m_origin;
}

inline void QLine3D::setOrigin(const QVector3D &value)
{
    m_origin = value;
}

inline QVector3D QLine3D::direction() const
{
    return m_direction;
}

inline void QLine3D::setDirection(const QVector3D& value)
{
    m_direction = value;
}

inline bool QLine3D::contains(const QVector3D &point) const
{
    QVector3D ppVec(point - m_origin);
    if (ppVec.isNull()) // point coincides with origin
        return true;
    qreal dot = QVector3D::dotProduct(ppVec, m_direction);
    if (qFuzzyIsNull(dot))
        return false;
    return qFuzzyCompare(dot*dot, ppVec.lengthSquared() * m_direction.lengthSquared());
}

inline bool QLine3D::contains(const QLine3D &line) const
{
    qreal dot = QVector3D::dotProduct(m_direction, line.direction());
    if (!qFuzzyCompare(dot*dot, m_direction.lengthSquared() * line.direction().lengthSquared()))
        return false;
    return contains(line.origin());
}

inline QVector3D QLine3D::project(const QVector3D &vector) const
{
    QVector3D norm = m_direction.normalized();
    return QVector3D::dotProduct(vector, norm) * norm;
}

inline QVector3D QLine3D::point(qreal t) const
{
    if (qIsNull(t))
        return m_origin;
    return m_origin + (t * m_direction.normalized());
}

inline qreal QLine3D::distance(const QVector3D &point) const
{
    QVector3D ppVec = point - m_origin;
    QVector3D collVec = project(ppVec);
    QVector3D perpVec = ppVec - collVec;
    return perpVec.length();
}

inline qreal QLine3D::distanceFromOrigin(const QVector3D &point) const
{
    QVector3D ppVec = point - m_origin;
    return ppVec.length();
}

inline bool QLine3D::sameSide(const QVector3D &pointA, const QVector3D &pointB) const
{
    QVector3D cpA = QVector3D::crossProduct(pointA - m_origin, m_direction).normalized();
    QVector3D cpB = QVector3D::crossProduct(pointB - m_origin, m_direction).normalized();
    return cpA == cpB;
}

inline bool QLine3D::operator==(const QLine3D &rhs)
{
    return rhs.origin() == m_origin && rhs.direction() == m_direction;
}

#ifndef QT_NO_DEBUG_STREAM
Q_QT3D_EXPORT QDebug operator<<(QDebug dbg, const QLine3D &line);
#endif

QT_END_NAMESPACE

#ifndef QT_NO_LINE3D
Q_DECLARE_METATYPE(QLine3D)
#endif

QT_END_HEADER

#endif // QLINE3D_H
