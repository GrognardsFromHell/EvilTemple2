
_SUBDIR_REL = $$replace(_PRO_FILE_PWD_, $$PWD, )
_SUBDIR_REL ~= s|/[^/]+|../

# The root of the build directory (shadow-build safe)
BUILD_ROOT = $$OUT_PWD/$$_SUBDIR_REL
