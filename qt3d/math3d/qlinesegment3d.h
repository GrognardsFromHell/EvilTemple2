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

#ifndef QLINESEGMENT3D_H
#define QLINESEGMENT3D_H

#include "qline3d.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3d)

class Q_QT3D_EXPORT QLineSegment3D
{
public:
    QLineSegment3D();
    QLineSegment3D(const QLine3D &line, qreal length);
    QLineSegment3D(const QVector3D &start, const QVector3D &end);

    QVector3D start() const;
    void setStart(const QVector3D& value);

    QVector3D end() const;
    void setEnd(const QVector3D &value);

    QVector3D direction() const;
    QVector3D vector() const;

    QLine3D toLine3D() const;

    qreal length() const;

    bool isPoint() const;

    bool contains(const QVector3D &point) const;
    bool contains(const QLineSegment3D &segment) const;

    bool intersects(const QLine3D &line) const;
    bool intersects(const QLineSegment3D &segment) const;
    QResult<QVector3D> intersection(const QLine3D &line) const;
    QResult<QVector3D> intersection(const QLineSegment3D &segment) const;

private:
    QVector3D m_start;
    QVector3D m_end;
};

inline QLineSegment3D::QLineSegment3D() {}

inline QLineSegment3D::QLineSegment3D(const QVector3D &start, const QVector3D &end)
    : m_start(start), m_end(end)
{
}

inline QVector3D QLineSegment3D::start() const
{
    return m_start;
}

inline void QLineSegment3D::setStart(const QVector3D &value)
{
    m_start = value;
}

inline QVector3D QLineSegment3D::end() const
{
    return m_end;
}

inline void QLineSegment3D::setEnd(const QVector3D &value)
{
    m_end = value;
}

inline QVector3D QLineSegment3D::direction() const
{
    return (m_end - m_start).normalized();
}

inline QVector3D QLineSegment3D::vector() const
{
    return m_end - m_start;
}

inline QLine3D QLineSegment3D::toLine3D() const
{
    return QLine3D(m_start, m_end - m_start);
}

inline qreal QLineSegment3D::length() const
{
    return (m_end - m_start).length();
}

inline bool QLineSegment3D::isPoint() const
{
    return m_start == m_end;
}

QT_END_NAMESPACE

#ifndef QT_NO_LINESEGMENT3D
Q_DECLARE_METATYPE(QLineSegment3D)
#endif

QT_END_HEADER

#endif  // QLINESEGMENT3D_H
