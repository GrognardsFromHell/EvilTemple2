To build Qt, the following command-line was used:

configure -no-rtti -graphicssystem opengl -no-qt3support -no-stl -ltcg -debug-and-release -no-s60 -no-style-motif -no-style-cde -openvg -D QT_COORD_TYPE=float -buildkey qt-coords-float -openssl

In addition, the examples and demo directories were renamed to greatly reduce the build time.

What the command line does:
- Builds both a debug and release version of Qt
- Disable RTTI (Since using it is bad style anyway)
- Use the OpenGL Rasterizer
- Disable QT3 Support (Not used by EvilTemple)
- Disable STL Support (Also not used by EvilTemple)
- Use LTCG (Link Time Code Generation) to improve performance
- Disable S60 support
- Disable two unused styles (Motif+CDE)
- !IMPORTANT! Define QT_COORD_TYPE as float, effectively making QMatrix and other 3d classes use float
- Since qreal is redefined, also redefine the buildkey to prevent plugin incompatibility
- Enable SSL support

All changes are optional. EvilTemple should build with the default library.