#ifndef MATERIALS_H
#define MATERIALS_H

#include <QObject>
#include <QScopedPointer>

#include "materialstate.h"
#include "texturesource.h"

namespace EvilTemple {

class MaterialsData;

class Materials : public QObject
{
    Q_OBJECT
public:
    explicit Materials(RenderStates &renderStates, QObject *parent = 0);
    ~Materials();

    /**
      Loads predefined materials
      */
    bool load();

    /**
      Use this to retrieve errors if loading the materials failed.
      */
    const QString &error() const;

signals:

public slots:

    /**
      Returns a material that can be used to represent a missing material.
      */
    SharedMaterialState &missingMaterial();

    /**
      Retrieve a material.
      */
    SharedMaterialState load(const QString &filename);

private:
    QScopedPointer<MaterialsData> d;

};

}

Q_DECLARE_METATYPE(EvilTemple::Materials*)

#endif // MATERIALS_H
