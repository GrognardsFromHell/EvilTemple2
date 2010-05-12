
#ifndef TEXTURESOURCE_H
#define TEXTURESOURCE_H

#include "texture.h"

namespace EvilTemple {

class TextureSource 
{
public:
    virtual SharedTexture loadTexture(const QString &name) = 0;
};

}

#endif // TEXTURE_H
