#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

#include "renderable.h"

#include <gamemath.h>

using namespace GameMath;

namespace EvilTemple {

    class Materials;
    class ParticleSystemData;
    class RenderStates;
    class ParticleSystemsData;
    class Emitter;
    class Models;
    class MaterialState;
    class ModelInstance;

    /**
  Models a particle system in world space and it's emitters.
  */
    class ParticleSystem : public Renderable
    {
    Q_OBJECT
    Q_PROPERTY(ModelInstance* modelInstance READ modelInstance WRITE setModelInstance)
    Q_PROPERTY(bool dead READ isDead)
    public:
        ParticleSystem(const QString &id);
        ~ParticleSystem();

        void addEmitter(Emitter *emitter);

        void setModelInstance(ModelInstance *modelInstance);
        ModelInstance *modelInstance() const;

        void elapseTime(float seconds);

        void render(RenderStates &renderStates);

        const Box3d &boundingBox();

        bool isDead() const;

    signals:
        void finished();

    private:
        QScopedPointer<ParticleSystemData> d;
        Q_DISABLE_COPY(ParticleSystem);
    };

    Q_DECLARE_METATYPE(ParticleSystem*);

    class ParticleSystems : public QObject
    {
    Q_OBJECT
    public:
        ParticleSystems(Models *models, Materials *materials);
        ~ParticleSystems();

        bool loadTemplates();

        const QString &error() const;

    public slots:
        /**
            Creates a particle system and returns it. The caller is responsible for updating, calling
            and rendering the particle system, as well as deleting it.
        */
        ParticleSystem *instantiate(const QString &name);

    private:

        QScopedPointer<ParticleSystemsData> d;
    };

}

Q_DECLARE_METATYPE(EvilTemple::ParticleSystems*)

#endif // PARTICLESYSTEM_H
