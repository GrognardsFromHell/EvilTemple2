#ifndef ZONEMAPS_H
#define ZONEMAPS_H

#include <QObject>
#include <QSharedPointer>

namespace EvilTemple {

    class VirtualFileSystem;
    class ZoneTemplatesData;
    class ZoneTemplate;
    class Zone;
    class Game;

    class ZoneTemplates : public QObject
    {
        Q_OBJECT
    public:
        explicit ZoneTemplates(const Game &game, QObject *parent = 0);
        ~ZoneTemplates();

        QSharedPointer<ZoneTemplate> get(quint32 id);

    signals:

    public slots:

    private:
        QScopedPointer<ZoneTemplatesData> d_ptr;

        Q_DISABLE_COPY(ZoneTemplates);

    };

}

#endif // ZONEMAPS_H
