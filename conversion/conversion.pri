
INCLUDEPATH += $${PWD}/include

CONFIG(debug, debug|release) {
 LIBS += -L$${PWD}/../bin -lconversion_d
} else {
 LIBS += -L$${PWD}/../bin -lconversion
}

