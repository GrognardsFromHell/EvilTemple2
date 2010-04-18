#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "troikaformatsglobal.h"

#include <QObject>

namespace Troika
{

    class VirtualFileSystem;
    class PrototypesData;

    class TROIKAFORMATS_EXPORT Prototype : public QObject
    {
    Q_OBJECT
    public:        
        explicit Prototype(int id, QObject *parent = 0);

        int id() const { return _id; }

        quint16 modelId() const { return _modelId; }
        void setModelId(quint16 model) { _modelId = model; }

        float rotation() const { return _rotation; }
        void setRotation(float angle) { _rotation = angle; }

        float scale() const { return _scale; }
        void setScale(float scale) { _scale = scale; }

        /**
          Parses a line already split into its part that is read from protos.tab
          */
        void parse(const QStringList &parts);

    private:
        int _id;
        quint16 _modelId;
        float _rotation;
        float _scale;
        Q_DISABLE_COPY(Prototype)
    };

    class TROIKAFORMATS_EXPORT Prototypes : public QObject
    {
    Q_OBJECT
    public:
        explicit Prototypes(VirtualFileSystem *vfs, QObject *parent = 0);
        ~Prototypes();

        Prototype *get(int id) const;

    signals:

    public slots:

    private:
        QScopedPointer<PrototypesData> d_ptr;
        Q_DISABLE_COPY(Prototypes);

    };

}

#endif // PROTOTYPES_H
