
#ifndef MATERIAL_H
#define MATERIAL_H

#include "gameglobal.h"

#include <GL/glew.h>

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QSharedPointer>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QVariant>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>

#include "util.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

/**
* Describes a render state that is changed by a material.
*/
class MaterialRenderState {
public:
    virtual ~MaterialRenderState();
    virtual void enable() = 0;
    virtual void disable() = 0;
};

class MaterialBlendFunction : public MaterialRenderState
{
public:
    MaterialBlendFunction(GLenum srcFactor, GLenum destFactor) : mSrcFactor(srcFactor), mDestFactor(destFactor) {
    }

    void enable();
    void disable();
private:
    GLenum mSrcFactor, mDestFactor;
};

class MaterialDepthMask : public MaterialRenderState
{
public:
    MaterialDepthMask(bool enableDepthWrite) : mEnableDepthWrite(enableDepthWrite) {
    }

    void enable() {
        glDepthMask(mEnableDepthWrite);
    }

    void disable() {
        glDepthMask(true);
    }
private:
    bool mEnableDepthWrite;
};

class MaterialDisableState : public MaterialRenderState
{
public:
    MaterialDisableState(GLenum state) : mState(state) {
    }

    void enable() {
        glDisable(mState);
    }
    void disable() {
        glEnable(mState);
    }
private:
    GLenum mState;
    bool defaultEnabled;
};

class MaterialEnableState : public MaterialRenderState
{
public:
    MaterialEnableState(GLenum state) : mState(state) {
    }

    void enable() {
        glEnable(mState);

    }
    void disable() {
        glDisable(mState);
    }
private:
    GLenum mState;
    bool defaultEnabled;
};

class MaterialColorMaskState : public MaterialRenderState
{
public:
    MaterialColorMaskState(bool red = true, bool green = true, bool blue = true, bool alpha = true)
        : mRed(red), mGreen(green), mBlue(blue), mAlpha(alpha)
    {
    }

    void enable() {
        SAFE_GL(glColorMask(mRed, mGreen, mBlue, mAlpha));
    }

    void disable() {
        // TODO: This encodes how the default state of glColorWrite should look
        SAFE_GL(glColorMask(true, true, true, true));
    }

private:
    bool mRed, mGreen, mBlue, mAlpha;
};

class StencilFuncState : public MaterialRenderState
{
public:
    StencilFuncState(GLenum func, GLint ref, GLuint mask)
        : mFunc(func), mRef(ref), mMask(mask)
    {
    }

    void enable() {
        SAFE_GL(glStencilFunc(mFunc, mRef, mMask));
    }

    void disable() {
        // TODO: This encodes how the default state of glStencilFunc should look
        SAFE_GL(glStencilFunc(GL_ALWAYS, 0, ~0));
    }

private:
    GLenum mFunc;
    GLint mRef;
    GLuint mMask;
};

class ClearStencilState : public MaterialRenderState
{
public:
    ClearStencilState(GLint value)
        : mValue(value)
    {
    }

    void enable() {
        //SAFE_GL(glClearStencil(mValue));
        //SAFE_GL(glClear(GL_STENCIL_BUFFER_BIT));
    }

    void disable() {
    }

private:
    GLint mValue;
};

class StencilOpState : public MaterialRenderState
{
public:
    StencilOpState(GLenum fail, GLenum zfail, GLenum zpass)
        : mFail(fail), mZFail(zfail), mZPass(zpass)
    {
    }

    void enable() {
        SAFE_GL(glStencilOp(mFail, mZFail, mZPass));
    }

    void disable() {
        // TODO: This encodes how the default state of glStencilop should look
        SAFE_GL(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
    }

private:
    GLenum mFail, mZFail, mZPass;
};

typedef QSharedPointer<MaterialRenderState> SharedMaterialRenderState;

/**
* Describes a vertex or fragment shader, including its code.
*/
class MaterialShader {
public:
    enum Type {
        FragmentShader = GL_VERTEX_SHADER,
        VertexShader = GL_FRAGMENT_SHADER
    };

    bool load(QXmlStreamReader *reader);

    Type type() const;
    const QString &code() const;
    const QString &version() const;
    const QStringList &includes() const;
private:
    QString mVersion;
    Type mType;
    QString mCode;
    QStringList mIncludes;
};

inline const QString &MaterialShader::version() const
{
    return mVersion;
}

inline MaterialShader::Type MaterialShader::type() const
{
    return mType;
}

inline const QString &MaterialShader::code() const
{
    return mCode;
}

inline const QStringList &MaterialShader::includes() const
{
    return mIncludes;
}

/**
* Associates an attribute within the linked shader program with one of the buffers of the model.
* The association is made by name.
*/
class MaterialAttributeBinding {
public:
    MaterialAttributeBinding();

    /**
     * The name of the attribute variable in the shader.
     */
    const QString &name() const;

    /**
     * The name of the buffer in the model.
     */
    const QString &bufferName() const;

    /**
     * Returns the number of components the vertex attribute has. Must be a value between 1 and 4.
     * Default is 4.
     */
    int components() const;

    /**
     * Enumerates all possible data types for components in the buffer.
     */
    enum Type {
        Byte = GL_BYTE,
        UnsignedByte = GL_UNSIGNED_BYTE,
        Short = GL_SHORT,
        UnsignedShort = GL_UNSIGNED_SHORT,
        Integer = GL_INT,
        UnsignedInteger = GL_UNSIGNED_INT,
        Float = GL_FLOAT,
        Double = GL_DOUBLE
             };

    /**
     * Returns the data type used by components in the buffer. Default is Float.
     */
    Type type() const;

    /**
     * Indicates that components in the buffer should be normalized to the range [0,1] for unsigned integer types
     * and [-1,1] for signed integer types, when they are accessed. See the specification of glVertexAttribPointer
     * for more information. Default is false.
     */
    bool normalized() const;

    /**
     * The byte offset between consecutive attributes in the buffer. 0 for tightly packed data, which is the default.
     */
    unsigned int stride() const;

    /**
     * Byte offset from the start of the buffer to the first vertex attribute. 0 if the data starts at the beginning
     * of the buffer, which is the default.
     */
    unsigned int offset() const;

    /**
     * Loads this attribute binding from an XML element.
     */
    bool load(QXmlStreamReader *reader);
private:
    QString mName;
    QString mBufferName;
    int mComponents;
    Type mType;
    bool mNormalized;
    unsigned int mStride;
    unsigned int mOffset;
};

inline const QString &MaterialAttributeBinding::name() const
{
    return mName;
}

inline const QString &MaterialAttributeBinding::bufferName() const
{
    return mBufferName;
}

inline MaterialAttributeBinding::Type MaterialAttributeBinding::type() const
{
    return mType;
}

inline int MaterialAttributeBinding::components() const
{
    return mComponents;
}

inline bool MaterialAttributeBinding::normalized() const
{
    return mNormalized;
}

inline unsigned int MaterialAttributeBinding::stride() const
{
    return mStride;
}

inline unsigned int MaterialAttributeBinding::offset() const
{
    return mOffset;
}

/**
* Defines what kind of binding a uniform variable inside a shader expects. Since uniform variables may need
* to be changed by the engine dynamically, this binding is required.
*/
class MaterialUniformBinding {
public:
    /**
     * Returns the name of the uniform binding.
     */
    const QString &name() const;

    /**
     * Returns the semantic of this uniform variable. Semantics are application defined.
     */
    const QString &semantic() const;

    /**
      * Indicates that this uniform is optional. This prevents an error from being raised when the uniform isn't found.
      * In debug mode, a warning is still raised.
      */
    bool isOptional() const;

    /**
      * If a constant value is associated with this uniform binding, this method will return it.
      * Otherwise an invalid QVariant is returned.
      */
    const QVariant &constantValue() const;

    /**
     * Loads this uniform binding from an XML element.
     */
    bool load(QXmlStreamReader *reader);

private:
    QString mName;
    QString mSemantic;
    QVariant mConstantValue;
    bool mOptional;
};

inline const QString &MaterialUniformBinding::name() const
{
    return mName;
}

inline bool MaterialUniformBinding::isOptional() const
{
    return mOptional;
}

inline const QString &MaterialUniformBinding::semantic() const
{
    return mSemantic;
}

inline const QVariant &MaterialUniformBinding::constantValue() const
{
    return mConstantValue;
}

/**
* Describes the settings for a single texture sampling stage, valid for a single pass.
*/
class MaterialTextureSampler {
public:
    MaterialTextureSampler();

    /**
     * The filename of the texture that will be bound to this texturing stage.
     * The special filename #<name> can be used to reference textures within the same file.
     */
    const QString &texture() const;

    /**
     * Loads this material texture from an XML element.
     */
    bool load(QXmlStreamReader *reader);

    enum WrapMode {
        Clamp,
        Wrap,
        Repeat
    };

    WrapMode wrapU() const;

    WrapMode wrapV() const;

private:
    QString mTexture;
    WrapMode mWrapU;
    WrapMode mWrapV;
};

inline MaterialTextureSampler::MaterialTextureSampler()
    : mWrapU(Repeat), mWrapV(Repeat)
{
}

inline const QString &MaterialTextureSampler::texture() const
{
    return mTexture;
}

inline MaterialTextureSampler::WrapMode MaterialTextureSampler::wrapU() const
{
    return mWrapU;
}

inline MaterialTextureSampler::WrapMode MaterialTextureSampler::wrapV() const
{
    return mWrapV;
}

class MaterialPass {
public:

    bool load(QXmlStreamReader *reader);

    const MaterialShader &vertexShader() const;
    const MaterialShader &fragmentShader() const;

    const QList<MaterialAttributeBinding> &attributeBindings() const;

    const QList<MaterialUniformBinding> &uniformBindings() const;

    const QList<MaterialTextureSampler> &textureSamplers() const;

    const QList<SharedMaterialRenderState> &renderStates() const;
private:
    MaterialShader mVertexShader;
    MaterialShader mFragmentShader;
    QList<MaterialAttributeBinding> mAttributeBindings;
    QList<MaterialUniformBinding> mUniformBindings;
    QList<MaterialTextureSampler> mTextureSamplers;
    QList<SharedMaterialRenderState> mRenderStates;
};

inline const QList<SharedMaterialRenderState> &MaterialPass::renderStates() const
{
    return mRenderStates;
}

inline const MaterialShader &MaterialPass::vertexShader() const
{
    return mVertexShader;
}

inline const QList<MaterialAttributeBinding> &MaterialPass::attributeBindings() const
{
    return mAttributeBindings;
}

inline const QList<MaterialUniformBinding> &MaterialPass::uniformBindings() const
{
    return mUniformBindings;
}

inline const QList<MaterialTextureSampler> &MaterialPass::textureSamplers() const
{
    return mTextureSamplers;
}

inline const MaterialShader &MaterialPass::fragmentShader() const
{
    return mFragmentShader;
}

class GAME_EXPORT Material
{
public:
    Material();
    ~Material();

    bool loadFromData(const QByteArray &data);

    bool loadFromFile(const QString &filename);

    const QString &error() const;

    const QList<MaterialPass*> &passes() const;
private:
    bool read(QXmlStreamReader *reader);

    QString mError;
    QList<MaterialPass*> mPasses;
};

inline const QString &Material::error() const
{
    return mError;
}

inline const QList<MaterialPass*> &Material::passes() const
{
    return mPasses;
}

}

#endif
