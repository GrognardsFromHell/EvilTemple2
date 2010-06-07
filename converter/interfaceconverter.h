#ifndef INTERFACECONVERTER_H
#define INTERFACECONVERTER_H

#include <QScopedPointer>

class ZipWriter;
class InterfaceConverterData;
namespace Troika {
    class VirtualFileSystem;
}

class InterfaceConverter
{
public:
    InterfaceConverter(ZipWriter *writer, Troika::VirtualFileSystem *vfs);
    ~InterfaceConverter();

    bool convert();

private:
    QScopedPointer<InterfaceConverterData> d_ptr;
};

#endif // INTERFACECONVERTER_H
