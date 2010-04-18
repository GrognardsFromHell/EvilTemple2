
TEMPLATE = lib

CONFIG += dll
TARGET = qt3d

DEFINES += Q_QT3D_LIBRARY

include(enablers/enablers.pri)
include(math3d/math3d.pri)

include(../../base.pri)

DESTDIR = ../../bin
