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

class ParticleSystems
{
public:
    ParticleSystems(RenderStates &renderState);
    ~ParticleSystems();

    void render();
    void create(const QString &name, const Vector4 &position);

private:
    QScopedPointer<ParticleSystemsData> d;
};

}

#endif // PARTICLESYSTEM_H
