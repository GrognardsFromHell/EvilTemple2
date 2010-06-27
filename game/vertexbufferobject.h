#ifndef VERTEXBUFFEROBJECT_H
#define VERTEXBUFFEROBJECT_H

#include <GL/glew.h>

#include <QByteArray>

namespace EvilTemple {

template<GLenum type>
class BufferObject
{
public:
    BufferObject()
    {
        glGenBuffers(1, &mBufferId);
    }

    ~BufferObject()
    {
        glDeleteBuffers(1, &mBufferId);
    }

    void upload(const QByteArray &data, GLenum usage = GL_STATIC_DRAW) const
    {
        bind();
        glBufferData(type, data.size(), data.constData(), usage);
        unbind();
    }

    void upload(const void *data, uint size, GLenum usage = GL_STATIC_DRAW) const
    {
        bind();
        glBufferData(type, size, data, usage);
        unbind();
    }

    /**
      Tells the graphics card driver that the contents of this buffer object are no longer needed.
      */
    void clear()
    {
        bind();
        glBufferData(type, 0, NULL, GL_STATIC_DRAW);
        unbind();
    }

    void bind() const
    {
        glBindBuffer(type, mBufferId);
    }

    void unbind() const
    {
        glBindBuffer(type, 0);
    }

    // Implicit conversion to buffer-id
    GLuint bufferId() const
    {
        return mBufferId;
    }

private:
    GLuint mBufferId;

    Q_DISABLE_COPY(BufferObject)
};

class VertexBufferObject : public BufferObject<GL_ARRAY_BUFFER>
{
};

class IndexBufferObject : public BufferObject<GL_ELEMENT_ARRAY_BUFFER>
{
};

}

#endif // VERTEXBUFFEROBJECT_H
