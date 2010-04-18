
#include <QMap>

#include "prototypes.h"
#include "virtualfilesystem.h"
#include "util.h"

namespace Troika
{

    const QString PrototypesFile = "rules/protos.tab";

    class PrototypesData
    {
    public:
        VirtualFileSystem *vfs;
        QMap<int,Prototype*> prototypes;

        void load(QObject *parent)
        {
            QByteArray data = vfs->openFile(PrototypesFile);
            QList<QByteArray> lines = data.split('\n');

            foreach (QByteArray rawline, lines)
            {
                QString line(rawline);

                if (line.trimmed().isEmpty())
                    continue;

                QStringList parts = line.split('\t', QString::KeepEmptyParts);

                // Skip lines with less than 100 parts
                if (parts.size() < 100)
                {
                    qWarning("Skipping prototype: %s", qPrintable(line));
                    continue;
                }

                int id = parts[0].toInt();
                Prototype *prototype = new Prototype(id, parent);
                prototype->parse(parts);

                prototypes[id] = prototype;
            }
        }
    };

    Prototypes::Prototypes(VirtualFileSystem *vfs, QObject *parent) :
            QObject(parent), d_ptr(new PrototypesData)
    {
        d_ptr->vfs = vfs;
        d_ptr->load(this);
    }

    Prototypes::~Prototypes()
    {
    }

    Prototype *Prototypes::get(int id) const
    {
        return d_ptr->prototypes[id];
    }

    Prototype::Prototype(int id, QObject *parent) : QObject(parent), _id(id)
    {
        _modelId = 0;
        _rotation = 0; //rad2deg(LegacyBaseRotation);
        _scale = 1.f;
    }

    inline bool isPartDefined(const QString &part)
    {
        return !part.trimmed().isEmpty();
    }

    void Prototype::parse(const QStringList &parts)
    {
        if (isPartDefined(parts[6]))
            _scale = parts[6].toFloat() / 100.f;

        if (isPartDefined(parts[34]))
            _modelId = parts[34].toInt();

        if (isPartDefined(parts[31]))
            _rotation = rad2deg(parts[31].toFloat() + LegacyBaseRotation);
    }

}
