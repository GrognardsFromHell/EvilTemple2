#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <QtCore/QString>
#include <QtCore/QScopedPointer>

#include "renderable.h"

#include <gamemath.h>

using namespace GameMath;

namespace EvilTemple {

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
    public:
        ParticleSystem(const QString &id, const QList<Emitter*> &emitters);
        ~ParticleSystem();

        void setModelInstance(const ModelInstance *modelInstance);

        void elapseTime(float timeUnits);
        void elapseSeconds(float seconds);

        void render(RenderStates &renderStates);

        const Box3d &boundingBox();

    private:
        QScopedPointer<ParticleSystemData> d;
        Q_DISABLE_COPY(ParticleSystem);
    };

    class ParticleSystems
    {
    public:
        ParticleSystems();
        ~ParticleSystems();

        /**
            Creates a particle system and returns it. The caller is responsible for updating, calling
            and rendering the particle system.
        */
        ParticleSystem *instantiate(const QString &name);

        bool loadTemplates();

        const QString &error() const;

    private:
        QScopedPointer<ParticleSystemsData> d;
    };

}

#endif // PARTICLESYSTEM_H
