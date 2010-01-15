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

#ifndef QRESULT_H
#define QRESULT_H

#include "qt3dglobal.h"
#include <QtGui/qvector3d.h>
#include <QtCore/qdebug.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3d)

template <typename T>
class QResult
{
public:
    enum Validity {
        NoResult,
        ResultExists,
        MultipleResults,
        InvalidArgument,
        OutOfRange
    };

    QResult(Validity valid = NoResult);
    QResult(const T &result, Validity validity = ResultExists);

    bool isValid() const;
    inline Validity validity() const { return m_validity; }

    inline T value() const;
    inline operator T () const;

private:
    T m_result;
    Validity m_validity;
};

template <typename T>
inline QResult<T>::QResult(Validity valid)
    : m_validity(valid)
{
}

template <typename T>
inline QResult<T>::QResult(const T &result, Validity validity)
    : m_result(result)
    , m_validity(validity)
{
}

template <typename T>
inline bool QResult<T>::isValid() const
{
    return m_validity == ResultExists;
}

template <typename T>
inline T QResult<T>::value() const
{
    return m_result;
}

template <typename T>
inline QResult<T>::operator T () const
{
    return m_result;
}

#ifndef QT_NO_DEBUG_STREAM

template <typename T>
QDebug operator<<(QDebug dbg, const QResult<T> &result)
{
    dbg.nospace() << "QResult( validity=";
    switch (result.validity()) {
        case QResult<T>::NoResult:
            dbg << "NoResult"; break;
        case QResult<T>::ResultExists:
            dbg << "ResultExists"; break;
        case QResult<T>::InvalidArgument:
            dbg << "InvalidArgument"; break;
        case QResult<T>::OutOfRange:
            dbg << "OutOfRange"; break;
        default:
            dbg << int(result.validity()); break;
    }
    dbg << ", value=" << result.value() << ")";
    return dbg.space();
}

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
