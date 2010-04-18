#ifndef MINIZIPGLOBAL_H
#define MINIZIPGLOBAL_H

#if defined(WIN32)
#if defined(MINIZIP_LIBRARY)
#  define MINIZIP_EXPORT __declspec(dllexport)
#else
#  define MINIZIP_EXPORT __declspec(dllimport)
#endif
#else
#  define MINIZIP_EXPORT
#endif

#endif // MINIZIPGLOBAL_H
