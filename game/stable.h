
/*
  This is the precompiled header.
 */

#include <QtDeclarative>
#include <QtCore>
#include <QtGui>
#include <QtScript>
#include <QtScriptTools>

// Target windows xp sp1
#define WINVER 0x0502

// Windows headers define min/max macros that kill <limits> support for min/max
#define NOMINMAX

#include <GL/glew.h>

#include <QtOpenGL>

#include "qbox3d.h"

#include <limits>
#include <cmath>
