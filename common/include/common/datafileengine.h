#ifndef DATAFILEENGINE_H
#define DATAFILEENGINE_H

#include "common/global.h"

#include <QAbstractFileEngineHandler>
#include <QScopedPointer>

namespace EvilTemple {

class DataFileEngineHandlerData;

/**
  The purpose of this class is to make the eviltemple resource system accessible through QFile.
  All paths relative to a directory on the hard drive are checked by this handler. First, if the
  file exists on the disk, the normal FS handler is used instead. If the file does not exist,
  it is checked whether the file or directory exist in one of the registered archives.
  If that is the case, a handler that accesses the file in the ZIP file is used instead.
  */
class COMMON_EXPORT DataFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    /**
      Constructs a file engine handler, that will manage the given data directory.
      */
    DataFileEngineHandler(const QString &dataPath);
    ~DataFileEngineHandler();

    /**
      Adds a ZIP archive to the virtual file system.
      @return True if the archive has been added successfully.
      */
    bool addArchive(const QString &filename);

    /**
      Adds all ZIP Files in the given directory to the virtual file system.
      @return The number of archives that have been added.
      */
    int addArchives(const QString &directory);

    QAbstractFileEngine *create(const QString &fileName) const;
private:
    QScopedPointer<DataFileEngineHandlerData> d_ptr;
};

}

#endif // DATAFILEENGINE_H
