
// Target windows xp sp1
#define WINVER 0x0502

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <GL/glew.h>

#define GAMEMATH_NO_MEMORY_OPERATORS
#include <gamemath.h>
using namespace GameMath;

#include <dagreader.h>
#include <troika_skeleton.h>
#include <troika_material.h>
#include <troika_materials.h>
#include <prototypes.h>
#include <objectfilereader.h>

#include <QtCore>
#include <QtXml>
