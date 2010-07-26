#ifndef UTIL_H
#define UTIL_H


const float Pi = 3.14159265358979323846f;

/**
  The original game applies an additional base rotation to everything in order to align it
  with the isometric grid. This is the radians value of that rotation.
  */
const float LegacyBaseRotation = 0.77539754f;

/**
  Converts radians to degree.
  */
inline float rad2deg(float rad)
{
    return rad * 180.f / Pi;
}

/**
  Converts radians to degree.
  */
inline float deg2rad(float deg)
{
    return deg / 180.f * Pi;
}

#endif // UTIL_H
