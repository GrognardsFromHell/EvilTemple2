#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

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
    class MaterialState;
    class ModelInstance;

    /**
  Models a particle system in world space and it's emitters.
  */
    class ParticleSystem : public Renderable
    {
    Q_OBJECT
    public:
        ParticleSystem(const QString &id, const QList<Emitter*> &emitters);
        ~ParticleSystem();

        void setModelInstance(ModelInstance *modelInstance);

        void elapseTime(float seconds);

        void render(RenderStates &renderStates);

        const Box3d &boundingBox();

    private:
        QScopedPointer<ParticleSystemData> d;
        Q_DISABLE_COPY(ParticleSystem);
    };

    typedef QSharedPointer<ParticleSystem> SharedParticleSystem;

    class ParticleSystems : public QObject
    {
    Q_OBJECT
    public:
        ParticleSystems(Materials *materials);
        ~ParticleSystems();

        bool loadTemplates();

        const QString &error() const;

    public slots:
        /**
            Creates a particle system and returns it. The caller is responsible for updating, calling
            and rendering the particle system.
        */
        SharedParticleSystem instantiate(const QString &name);

    private:

        QScopedPointer<ParticleSystemsData> d;
    };

}

Q_DECLARE_METATYPE(EvilTemple::ParticleSystems*)

#endif // PARTICLESYSTEM_H
