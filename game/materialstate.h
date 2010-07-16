
#ifndef MATERIALSTATE_H
#define MATERIALSTATE_H

#include <QtCore/QString>
#include <QtCore/QScopedPointer>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QVector>

#include "matrix4.h"
using namespace GameMath;

#include "material.h"
#include "renderstates.h"
#include "texture.h"
#include "texturesource.h"
#include "glslprogram.h"

namespace EvilTemple {

uint getActiveMaterialStates();

class RenderStates;

/**
  Allows binding of a value to a uniform variable in a shader. Objects of this class
  are not owned by the shader, since they are generic and represent the source of a
  certain value. Instead, they should be owned by the object from which the value originates.
  */
class UniformBinder {
public:
    virtual ~UniformBinder();

    /**
      Binds the value from this binder to the given uniform location of the currently
      bound shader program.
      */
    virtual void bind(GLint location) const = 0;
};

template<typename T> inline void bindUniform(GLint location, const T &uniform) {
    throw std::exception("There is no generic uniform binding function.");
}

template<> inline void bindUniform<Matrix4>(GLint location, const Matrix4 &matrix) {
    glUniformMatrix4fv(location, 1, false, matrix.data());
}

template<> inline void bindUniform<uint>(GLint location, const uint &value) {
    glUniform1ui(location, value);
}

template<> inline void bindUniform<int>(GLint location, const int &value) {
    glUniform1i(location, value);
}

template<> inline void bindUniform<float>(GLint location, const float &value) {
    glUniform1f(location, value);
}

template<> inline void bindUniform<QVector2D>(GLint location, const QVector2D &value) {
    glUniform2f(location, value.x(), value.y());
}

template<> inline void bindUniform<QVector3D>(GLint location, const QVector3D &value) {
    glUniform3f(location, value.x(), value.y(), value.z());
}

template<> inline void bindUniform<QVector4D>(GLint location, const QVector4D &value) {
    glUniform4f(location, value.x(), value.y(), value.z(), value.w());
}

template<> inline void bindUniform<Vector4>(GLint location, const Vector4 &value) {
    glUniform4fv(location, 1, value.data());
}

/**
  This binder will bind a constant value to a uniform.

  A use case for this binder is the binding of constant values by different
  users of the same shader.
  */
template<typename T> class ConstantBinder : public UniformBinder {
public:
    ConstantBinder(const T &value) : mValue(value)
    {
    }

    void bind(GLint location) const
    {
        bindUniform<T>(location, mValue);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            qWarning("Unable to bind value to uniform location %d: %s.", location, gluErrorString(error));
        }
    }

private:
    T mValue;
};

/**
  This uniform binder keeps a reference to a value, so if the value is updated, this binder
  will always bind the current value.
  */
template<typename T> class ReferenceBinder : public UniformBinder {
public:
    ReferenceBinder(const T &ref) : mRef(ref)
    {
    }

    void bind(GLint location) const
    {
        bindUniform<T>(location, mRef);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            qWarning("Unable to bind value to uniform location %d: %s.", location, gluErrorString(error));
        }
    }
private:
    const T &mRef;
};

class MaterialPassUniformState
{
public:
    MaterialPassUniformState() : mLocation(-1), mDeleteBinder(false), mBinder(NULL)
    {
    }

    ~MaterialPassUniformState()
    {
        if (mDeleteBinder)
            delete mBinder;
    }

    inline void bind()
    {
        Q_ASSERT(mBinder);
        mBinder->bind(mLocation);
    }

    void setLocation(GLint location)
    {
        mLocation = location;
    }

    void setImmutableBinder(const UniformBinder *binder)
    {
        if (mDeleteBinder)
            delete mBinder;
        mBinder = const_cast<UniformBinder*>(binder);
        mDeleteBinder = false;
    }

    void setBinder(UniformBinder *binder)
    {
        if (mDeleteBinder)
            delete mBinder;
        mBinder = binder;
        mDeleteBinder = true;
    }
private:
    GLint mLocation;
    bool mDeleteBinder;
    UniformBinder *mBinder;

    Q_DISABLE_COPY(MaterialPassUniformState);
};

class MaterialTextureSamplerState {
public:
        void bind();
        void unbind();

        void setSamplerId(int samplerId);
        void setTexture(const SharedTexture &texture);
        void setWrapMode(GLenum wrapU, GLenum wrapV);
private:
        int mSamplerId;
        SharedTexture mTexture;
        GLenum mWrapU, mWrapV;
};

inline void MaterialTextureSamplerState::setWrapMode(GLenum wrapU, GLenum wrapV)
{
    mWrapU = wrapU;
    mWrapV = wrapV;
}

inline void MaterialTextureSamplerState::bind()
{
        glActiveTexture(GL_TEXTURE0 + mSamplerId);
        mTexture->bind();
        if (mWrapU != GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapU);
        if (mWrapV != GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapV);
        // TODO: Set sampler states (wrap+clam+filtering+etc)
}

inline void MaterialTextureSamplerState::unbind()
{
        glActiveTexture(GL_TEXTURE0 + mSamplerId);
        if (mWrapU != GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        if (mWrapV != GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
}

inline void MaterialTextureSamplerState::setSamplerId(int samplerId)
{
        mSamplerId = samplerId;
}

inline void MaterialTextureSamplerState::setTexture(const SharedTexture &texture)
{
        mTexture = texture;
}

struct MaterialPassAttributeState
{
    GLint location;
    int bufferType;
    MaterialAttributeBinding binding; // Parameters
};

class MaterialPassState {
public:
    MaterialPassState();
    ~MaterialPassState();

    uint id;
    SharedGLSLProgram program;
    QVector<MaterialPassAttributeState> attributes;
    QVector<MaterialPassUniformState*> uniforms;
    QVector<MaterialTextureSamplerState> textureSamplers;
    QList<SharedMaterialRenderState> renderStates;
private:
    Q_DISABLE_COPY(MaterialPassState)
};

/**
 * Models the runtime state of a material. Especially important for maintaining textures,
 * and shader bindings.
 */
class MaterialState {
public:
    MaterialState();
    ~MaterialState();

    int passCount;
    QScopedArrayPointer<MaterialPassState> passes;
    bool createFrom(const Material &material, const RenderStates &renderState, TextureSource *textureSource = FileTextureSource::instance());
    bool createFromFile(const QString &filename, const RenderStates &renderState, TextureSource *textureSource = FileTextureSource::instance());
    const QString &error() const;
private:
    QString mError;
};

typedef QSharedPointer<MaterialState> SharedMaterialState;

inline const QString &MaterialState::error() const
{
        return mError;
}

}

#endif // MATERIALSTATE_H
