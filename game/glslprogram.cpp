
#include <QtCore/QByteArray>
#include <QtCore/QFile>

#include "glslprogram.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#define HANDLE_GL_ERROR if (handleGlError(__FILE__, __LINE__)) {return false;}

namespace EvilTemple {

    GLSLProgram::GLSLProgram() : mVertexShaderId(0), mFragmentShaderId(0), mProgramId(0)
    {
    }

    GLSLProgram::~GLSLProgram()
    {
        release();
    }

    bool GLSLProgram::bind()
    {
        SAFE_GL(glUseProgram(mProgramId));
        return true;
    }

    void GLSLProgram::unbind()
    {
        glUseProgram(0);
    }

    bool GLSLProgram::loadFromFile(const QString &vertexShaderFile, const QString &fragmentShaderFile)
    {
        QFile file(vertexShaderFile);

        if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            mError = QString("Unable to open vertex shader file %1").arg(file.fileName());
            return false;
        }

        QByteArray vertexShaderCode = file.readAll();
        vertexShaderCode.append('\0');
        file.close();

        file.setFileName(fragmentShaderFile);
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            mError = QString("Unable to open fragment shader file %1").arg(file.fileName());
            return false;
        }

        QByteArray fragmentShaderCode = file.readAll();
        file.close();
        fragmentShaderCode.append('\0');

        return load(fragmentShaderCode, fragmentShaderCode);
    }

    static bool checkCompileStatus(GLuint shader, QString *error = NULL)
    {
        // An OpenGL error is not raised when compilation of the shader failed
        GLint compileStatus;
        SAFE_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));

        if (compileStatus == GL_TRUE)
            return true; // Compilation succeeded

        if (!error)
            return false; // No error log needs to be retrieved

        // Retrieve the error log
        GLint logLength;
        SAFE_GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

        // According to spec, log-length is 0 for no log, otherwise it includes the nul-termination
        if (logLength == 0) {
            *error = "Compilation failed due to an unknown error.";
            return false;
        }

        // Create a temporary buffer for the info log, retrieve it and set it as the current error
        QScopedArrayPointer<char> infoLog(new char[logLength]);

        GLint realLogLength;
        SAFE_GL(glGetShaderInfoLog(shader, logLength, &realLogLength, infoLog.data()));

        *error = QString::fromLatin1(infoLog.data(), realLogLength);
        return false;
    }

    static bool checkLinkStatus(GLuint program, QString *error = NULL)
    {
        GLint linkStatus;
        SAFE_GL(glGetProgramiv(program, GL_LINK_STATUS, &linkStatus));

        if (linkStatus == GL_TRUE)
            return true; // Linking succeeded

        if (!error)
            return false; // No error log needs to be retrieved

        // Retrieve the error log
        GLint logLength;
        SAFE_GL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength));

        // According to spec, log-length is 0 for no log, otherwise it includes the nul-termination
        if (logLength == 0) {
            *error = "Compilation failed due to an unknown error.";
            return false;
        }

        // Create a temporary buffer for the info log, retrieve it and set it as the current error
        QScopedArrayPointer<char> infoLog(new char[logLength]);

        GLint realLogLength;
        SAFE_GL(glGetProgramInfoLog(program, logLength, &realLogLength, infoLog.data()));

        *error = QString::fromLatin1(infoLog.data(), realLogLength);
        return false;
    }

    bool GLSLProgram::load( const QByteArray &vertexShaderCode, const QByteArray &fragmentShaderCode )
    {
        const char *code = NULL;
        QString compileError;

        release();

        /*
         Create the vertex shader.
         */
        mVertexShaderId = glCreateShader(GL_VERTEX_SHADER);

        if (!mVertexShaderId) {
            mError = "Unable to create the vertex shader object.";
            return false;
        }

        code = vertexShaderCode.constData();
        SAFE_GL(glShaderSource(mVertexShaderId, 1, &code, 0));

        SAFE_GL(glCompileShader(mVertexShaderId));

        if (!checkCompileStatus(mVertexShaderId, &compileError)) {
            mError = "Unable to compile the vertex shader: " + compileError;
            return false;
        }

        /*
         Create the fragment shader.
         */
        mFragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

        if (!mFragmentShaderId) {
            mError = "Unable to create the fragment shader object.";
            return false;
        }

        code = fragmentShaderCode.constData();
        SAFE_GL(glShaderSource(mFragmentShaderId, 1, &code, 0));

        SAFE_GL(glCompileShader(mFragmentShaderId));

        if (!checkCompileStatus(mFragmentShaderId, &compileError)) {
            mError = "Unable to compile the fragment shader: " + compileError;
            return false;
        }

        /*
         Create the program.
         */
        mProgramId = glCreateProgram();

        if (!mProgramId) {
            mError = "Unable to create the program object.";
            return false;
        }

        SAFE_GL(glAttachShader(mProgramId, mVertexShaderId));
        SAFE_GL(glAttachShader(mProgramId, mFragmentShaderId));

        SAFE_GL(glLinkProgram(mProgramId));

        if (!checkLinkStatus(mProgramId, &compileError)) {
            mError = "Unable to link the program: " + compileError;
            return false;
        }

        updateActiveUniforms();
        updateActiveAttributes();

        return true;
    }

    void GLSLProgram::updateActiveUniforms()
    {
        GLint activeUniforms;
        SAFE_GL(glGetProgramiv(mProgramId, GL_ACTIVE_UNIFORMS, &activeUniforms));

        GLint maxUniformLength;
        SAFE_GL(glGetProgramiv(mProgramId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength));

        // The GL spec is not clear whether GL_ACTIVE_UNIFORM_MAX_LENGTH includes null-termination
        QByteArray uniformName(maxUniformLength + 1, Qt::Uninitialized);

        // Get the uniform-name of each index
        mUniforms.clear();
        mUniforms.reserve(activeUniforms);
        for (int i = 0; i < activeUniforms; ++i) {
            GLsizei maxLength = uniformName.size();
            GLint size;
            GLenum type;
            SAFE_GL(glGetActiveUniform(mProgramId, i, maxLength, &maxLength, &size, &type, uniformName.data()));
            int locationId = glGetUniformLocation(mProgramId, uniformName.constData());

            QByteArray truncatedName(uniformName.constData());
            mUniforms.insert(truncatedName, locationId);
        }
    }

    void GLSLProgram::updateActiveAttributes()
    {
        GLint activeAttributes;
        SAFE_GL(glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTES, &activeAttributes));

        GLint maxAttributeLength;
        SAFE_GL(glGetProgramiv(mProgramId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeLength));

        // The GL spec is not clear whether GL_ACTIVE_ATTRIBUTE_MAX_LENGTH includes null-termination
        QByteArray attributeName(maxAttributeLength + 1, Qt::Uninitialized);

        // Get the attribute-name of each index
        mAttributes.clear();
        mAttributes.reserve(activeAttributes);
        for (int i = 0; i < activeAttributes; ++i) {
            GLsizei maxLength = attributeName.size();
            GLint size;
            GLenum type;
            SAFE_GL(glGetActiveAttrib(mProgramId, i, maxLength, &maxLength, &size, &type, attributeName.data()));
            int locationId = glGetAttribLocation(mProgramId, attributeName.constData());

            QByteArray truncatedName(attributeName.constData());
            mAttributes.insert(truncatedName, locationId);
        }
    }

    void GLSLProgram::release()
    {
        if (mProgramId) {
            glDeleteProgram(mProgramId);
            mProgramId = 0;
        }

        if (mVertexShaderId) {
            glDeleteShader(mVertexShaderId);
            mVertexShaderId = 0;
        }

        if (mFragmentShaderId) {
            glDeleteShader(mFragmentShaderId);
            mFragmentShaderId = 0;
        }
    }

}
