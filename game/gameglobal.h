#ifndef GAMEGLOBAL_H
#define GAMEGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(GAME_LIBRARY)
#  define GAME_EXPORT Q_DECL_EXPORT
#else
#  define GAME_EXPORT Q_DECL_IMPORT
#endif

#endif // GAMEGLOBAL_H
