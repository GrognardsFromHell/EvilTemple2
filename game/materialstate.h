
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

template<> inline void bindUniform<int>(GLint location, const int &value) {
    glUniform1i(location, value);
}

template<> inline void bindUniform<float>(GLint location, const float &value) {
    glUniform1f(location, value);
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
    MaterialPassUniformState() : mBinder(NULL), mLocation(-1)
    {
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

    void setBinder(const UniformBinder *binder)
    {
        mBinder = binder;
    }
private:
    GLint mLocation;
    const UniformBinder *mBinder;
};

class MaterialTextureSamplerState {
public:
	void bind();
	void unbind();

	void setSamplerId(int samplerId);
	void setTexture(const SharedTexture &texture);
private:	
	int mSamplerId;
	SharedTexture mTexture;
};

inline void MaterialTextureSamplerState::bind()
{
	glActiveTexture(GL_TEXTURE0 + mSamplerId);
    mTexture->bind();
	// TODO: Set sampler states (wrap+clam+filtering+etc)
}

inline void MaterialTextureSamplerState::unbind()
{
	glActiveTexture(GL_TEXTURE0 + mSamplerId);
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

    GLSLProgram program;
    QVector<MaterialPassAttributeState> attributes;
    QVector<MaterialPassUniformState> uniforms;
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
    int passCount;
    QScopedArrayPointer<MaterialPassState> passes;
    bool createFrom(const Material &material, const RenderStates &renderState, TextureSource *textureSource);
    bool createFromFile(const QString &filename, const RenderStates &renderState, TextureSource *textureSource);
    const QString &error() const;
private:
    QString mError;
};

inline const QString &MaterialState::error() const
{
	return mError;
}

}

#endif // MATERIALSTATE_H
