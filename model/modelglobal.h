#ifndef MODELGLOBAL_H
#define MODELGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(MODEL_LIBRARY)
#  define MODEL_EXPORT Q_DECL_EXPORT
#else
#  define MODEL_EXPORT Q_DECL_IMPORT
#endif

#endif // MODELGLOBAL_H
