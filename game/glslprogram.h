
#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <QtCore/QString>
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

    GLint attributeLocation(const char *name);

    GLint uniformLocation(const char *name);

private:
    QString mError;
    GLuint mVertexShaderId, mFragmentShaderId, mProgramId;

    Q_DISABLE_COPY(GLSLProgram)
};

typedef QSharedPointer<GLSLProgram> SharedGLSLProgram;

inline GLuint GLSLProgram::handle() const
{
        return mProgramId;
}

}

#endif
