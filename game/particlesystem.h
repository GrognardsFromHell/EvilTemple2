#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <QtCore/QString>
#include <QtCore/QScopedPointer>

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
class ParticleSystem
{
public:
    ParticleSystem(const QString &id, const QList<Emitter*> &emitters);
    ~ParticleSystem();

    void setModelInstance(const ModelInstance *modelInstance);

    void setPosition(const Vector4 &position);
    const Vector4 &position() const;

    void elapseTime(float timeUnits);
    void elapseSeconds(float seconds);

    void render(RenderStates &renderStates, MaterialState *material) const;

private:
    QScopedPointer<ParticleSystemData> d;
	Q_DISABLE_COPY(ParticleSystem)
};

class ParticleSystems
{
public:
    ParticleSystems(RenderStates &renderState);
    ~ParticleSystems();

    void render();
    void create(const QString &name, const Vector4 &position);

    /**
    Creates a particle system and returns it. The caller is responsible for updating, calling
    and rendering the particle system.
    */
    ParticleSystem *instantiate(const QString &name);

    MaterialState *spriteMaterial();
    
private:
    QScopedPointer<ParticleSystemsData> d;
};

}

#endif // PARTICLESYSTEM_H
