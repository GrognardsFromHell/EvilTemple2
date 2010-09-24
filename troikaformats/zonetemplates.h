#ifndef ZONEMAPS_H
#define ZONEMAPS_H

#include "troikaformatsglobal.h"

#include <QScopedPointer>
#include <QList>

namespace Troika {

    class VirtualFileSystem;
    class ZoneTemplate;
    class Prototypes;
    class ZoneTemplatesData;

    class TROIKAFORMATS_EXPORT ZoneTemplates : public QObject
    {
    Q_OBJECT
    public:
        ZoneTemplates(VirtualFileSystem *vfs, Prototypes *prototypes, QObject *parent = NULL);
        ~ZoneTemplates();

        ZoneTemplate *load(quint32 id);
        QList<quint32> mapIds();

    private:
        QScopedPointer<ZoneTemplatesData> d_ptr;

    };

}

#endif // ZONEMAPS_H
