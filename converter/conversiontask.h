#ifndef CONVERSIONTASK_H
#define CONVERSIONTASK_H

#include <QObject>
#include <QString>
#include <QByteArray>

namespace Troika
{
    class ZoneTemplate;
    class ZoneTemplates;
    class VirtualFileSystem;
    class Prototypes;
    class Materials;
}

/**
  Interface for writing files. This is necessary to allow writing to either Zip Files or the filesystem
  directly.
  */
class IFileWriter
{
public:
    virtual ~IFileWriter();

    virtual void addFile(const QString &filename, const QByteArray &content, bool compressable = true) = 0;

    virtual void close() = 0;
};

inline IFileWriter::~IFileWriter()
{
}

/**
  Exception class
  */
class ConversionTaskAborted
{
};

/**
  Interface for utility services requested by individual conversion tasks.
  */
class IConversionService
{
public:
    virtual IFileWriter *createOutput(const QString &groupName) = 0;

    virtual Troika::VirtualFileSystem *virtualFileSystem() = 0;

    /**
     Services related to getting access to already parsed data.
     */
    virtual Troika::ZoneTemplates *zoneTemplates() = 0;

    virtual Troika::Prototypes *prototypes() = 0;

    virtual Troika::Materials *materials() = 0;

    /**
      Registers a mesh filename, so it will be converted by the mesh conversion process.
      */
    virtual void addMeshReference(const QString &meshFilename) = 0;

    virtual const QSet<QString> &getMeshReferences() const = 0;

    virtual void addParticleSystem(const QString &particleSystemName) = 0;

    virtual QString getParticleSystemFromHash(uint hash) = 0;

    virtual QString getInternalName(uint id) = 0;

    virtual QHash<uint, QString> openMessageFile(const QString &filename) = 0;

    virtual QString convertMapId(uint mapId) const = 0;
};

class ConversionTask : public QObject
{
    Q_OBJECT
public:
    explicit ConversionTask(IConversionService *service, QObject *parent = 0);

    virtual void run() = 0;

    virtual uint cost() const = 0;

    virtual QString description() const = 0;

    IConversionService *service() const;

protected:
    void assertNotAborted() const;
    bool isAborted() const;

signals:

    void progress(int currentWork, int totalWork);

public slots:

    void abort();

private:
    IConversionService *mService;
    volatile bool mAborted;
};

inline void ConversionTask::abort()
{
    mAborted = true;
}

inline IConversionService *ConversionTask::service() const
{
    return mService;
}

inline void ConversionTask::assertNotAborted() const
{
    if (mAborted)
        throw ConversionTaskAborted();
}

inline bool ConversionTask::isAborted() const
{
    return mAborted;
}

#endif // CONVERSIONTASK_H
