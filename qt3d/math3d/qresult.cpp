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

#include "qresult.h"

/*!
  \class QResult
  \brief The QResult class represents a return value that includes a solution and a validity.
  \since 4.6
  \ingroup qt3d
  \ingroup qt3d::math
  A QResult instance is returned by mathematical and other operations where
  for the given set of parameters to the operation, a valid or otherwise
  useful solution might not exist.

  The QResult is generally constructed by library code implementing the
  operation, and inspected by client code processing the result.

  QResult is a lightweight template class, which wraps an instance of the
  solution type (which is the template parameter), and a validity value.
  QResult can be efficiently passed by value if the solution type can be.

  The library operation implementation determines the type of solution
  result (being T, the template parameter), and the instance of QResult
  returns this value either by the cast operator or explicity by the value
  function.

  \sa QLine3D, QPlane3D
*/

/*!
  \enum QResult::Validity
  This enum defines the validity for solutions returned in a QResult.
  \value NoResult A valid result could not be found; for example, two primitives did not intersect,
  \value ResultExists A valid result exists, which can be retrieved via QResult::value().
  \value MultipleResults More than one solution exists, possibly infinitely many.
  \value InvalidArgument One of the arguments was invalid so no result is possible; for example, a null vector.
  \value OutOfRange The result cannot be represented, or is infinite.
 */

/*!
  \fn QResult::QResult(Validity valid = NoResult)
  Construct a new QResult with validity \a valid, set by default to QResult::NoResult
  The result value is set to a default constructed instance of type T
 */

/*!
  \fn QResult::QResult(const T &result, Validity validity = ResultExists)
  Construct a new QResult with the given \a result as the solution, and
  given \a validity.  Typically this will be called by library routines which
  return a computed result to encapsulate the validity of the solution and its value.
 */

/*!
  \fn bool QResult::isValid() const
  Return true if the result is valid.
  This is a shortcut for testing against the result of the validity() method,
  and is equivalent to \c{myResult.validity() == ResultExists}
  \sa validity()
 */

/*!
  \fn int QResult::validity() const
  Return the validity of this result.
  \sa isValid()
 */

/*!
  \fn T QResult::value() const
  Return the calculated result of the operation.  Note that testing this variable will
  not determine if the solution is valid, since for example if T is QVector3D, the null
  vector (0, 0, 0) may be a correct result.
  \sa validity()
 */

/*!
  \fn QResult::operator T () const
  Cast this QResult to T, the templated T.
  This simply returns the encapsulated T solution value.  Note
  that this can be used as a shortcut if the validity of the result is known
  since it effectively discards the validity information.
  \sa value()
 */
