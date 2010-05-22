
#ifndef TEXTURESOURCE_H
#define TEXTURESOURCE_H

#include "texture.h"

namespace EvilTemple {

class TextureSource 
{
public:
    virtual SharedTexture loadTexture(const QString &name) = 0;
};

class FileTextureSource : public TextureSource
{
public:
    SharedTexture loadTexture(const QString &name);

    static TextureSource* instance();
private:
    static FileTextureSource mInstance;
};

inline TextureSource *FileTextureSource::instance()
{
    return &mInstance;
}

}

#endif // TEXTURE_H
