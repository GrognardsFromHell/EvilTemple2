#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

#include <QObject>
#include <QFile>
#include <QList>
#include <QDir>

namespace EvilTemple {

/*!
    This interface is the base for all virtual file system handlers.
*/
class VirtualFileSystemHandler {
public:
    virtual QByteArray openFile(const QString &filename) = 0;
    virtual bool exists(const QString &filename) = 0;
    virtual QStringList listFiles(const QString &path, const QString &filter = "*") = 0;
};

class VirtualFileSystem : public QObject
{
Q_OBJECT
public:
    typedef VirtualFileSystemHandler *Handler;

    explicit VirtualFileSystem(QObject *parent = 0);

    QByteArray openFile(const QString &filename);
    bool exists(const QString &filename);
    QStringList listFiles(const QString &path, const QString &filter = "*");

signals:

public slots:
    void add(Handler handler);
    void remove(Handler handler);

private:
    QList<Handler> handler;

    Q_DISABLE_COPY(VirtualFileSystem)    
};

}

#endif // VIRTUALFILESYSTEM_H
