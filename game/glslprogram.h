
#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QSharedPointer>
#include <GL/glew.h>

namespace EvilTemple {

class GLSLProgram {
public:
    GLSLProgram();
    ~GLSLProgram();

    /**
     * Binds this program to the OpenGL state
     */
    bool bind();

    /**
     * Unbinds the current program from the OpenGL state.
     */
    void unbind();

    /**
     * Loads a program from two
     */
    bool loadFromFile(const QString &vertexShaderFile, const QString &fragmentShaderFile);

    /**
     * Loads program from two strings.
     */
    bool load(const QByteArray &vertexShaderCode, const QByteArray &fragmentShaderCode);

    /**
     * Releases resources held by this class.
     */
    void release();

    GLuint handle() const;

    /**
     * Sets a texture sampler uniform by name.
     */
    void setUniformTexture(const char *name, GLuint texture);

    /**
     * Returns the last error string if a method fails.
     */
    const QString &error() const {
        return mError;
    }

    GLint attributeLocation(const QByteArray &name) const;

    GLint uniformLocation(const QByteArray &name) const;

private:
    void updateActiveUniforms();
    void updateActiveAttributes();

    QString mError;
    GLuint mVertexShaderId, mFragmentShaderId, mProgramId;
    QHash<QString, uint> mUniforms;
    QHash<QString, uint> mAttributes;

    Q_DISABLE_COPY(GLSLProgram)
};

inline GLint GLSLProgram::attributeLocation(const QByteArray &name) const
{
    return mAttributes.value(name, -1);
}

inline GLint GLSLProgram::uniformLocation(const QByteArray &name) const {
    return mUniforms.value(name, -1);
}

typedef QSharedPointer<GLSLProgram> SharedGLSLProgram;

inline GLuint GLSLProgram::handle() const
{
        return mProgramId;
}

}

#endif
