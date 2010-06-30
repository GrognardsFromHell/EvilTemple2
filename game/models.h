#ifndef MODELS_H
#define MODELS_H

#include <QMetaType>
#include <QObject>
#include <QScopedPointer>

#include "modelfile.h"

namespace EvilTemple {

class Materials;
class RenderStates;
class ModelsData;

class Models : public QObject
{
    Q_OBJECT
public:
    explicit Models(Materials *materials, RenderStates &renderStates, QObject *parent = 0);
    ~Models();

public slots:

    SharedModel load(const QString &filename);

private:
    QScopedPointer<ModelsData> d;

};

}

Q_DECLARE_METATYPE(EvilTemple::Models*);

#endif // MODELS_H
