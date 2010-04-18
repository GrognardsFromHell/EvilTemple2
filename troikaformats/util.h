#ifndef UTIL_H
#define UTIL_H

#include "constants.h"

/**
  Converts radians to degree.
  */
inline float rad2deg(float rad)
{
    return rad * 180.f / Pi;
}

#endif // UTIL_H
