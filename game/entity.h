
#ifndef ENTITY_H
#define ENTITY_H

#include "renderable.h"

namespace EvilTemple {

class Entity : public Renderable {
public:
    virtual ~Entity();

    virtual void render(RenderStates &renderStates);
};

};

#endif // ENTITY_H
