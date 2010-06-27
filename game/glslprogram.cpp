
#include <QtCore/QByteArray>
#include <QtCore/QFile>

#include "GLSLProgram.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#define HANDLE_GL_ERROR if (handleGlError(__FILE__, __LINE__)) {return false;}

namespace EvilTemple {

    GLSLProgram::GLSLProgram() : vertexShader(0), fragmentShader(0), program(0)
    {
    }

    GLSLProgram::~GLSLProgram()
    {
        release();
    }

    bool GLSLProgram::bind()
    {
        glUseProgram(program);
        HANDLE_GL_ERROR
                return true;
    }

    void GLSLProgram::unbind()
    {
        glUseProgram(0);
    }

    bool GLSLProgram::checkShaderError(GLuint shader, const char *file, int line)
    {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success == GL_TRUE) {
            return false;
        }

        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            mError.append(QString("GLSL Shader error @ %1:%2: ").arg(file).arg(line));

            QByteArray message(length, Qt::Uninitialized);
            GLsizei actualLength;
            glGetShaderInfoLog(shader, length, &actualLength, message.data());
            mError.append(message);
            mError.append("\n");
            return true;
        }

        return false;
    }

    bool GLSLProgram::checkProgramError(const char *file, int line)
    {
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (status == GL_TRUE) {
            return false;
        }

        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            mError.append(QString("GLSL Program error @ %1:%2: ").arg(file).arg(line));

            QByteArray message(length, Qt::Uninitialized);
            GLsizei actualLength;
            glGetProgramInfoLog(program, length, &actualLength, message.data());
            mError.append(message);
            mError.append("\n");
            return true;
        }

        return false;
    }

    bool GLSLProgram::handleGlError(const char *file, int line) {
        bool errorOccured = false;

        mError.clear();

        if (program) {
            errorOccured |= checkProgramError(file, line);
        }
        if (vertexShader) {
            errorOccured |= checkShaderError(vertexShader, file, line);
        }
        if (fragmentShader) {
            errorOccured |= checkShaderError(fragmentShader, file, line);
        }

        GLenum glErr = glGetError();
        while (glErr != GL_NO_ERROR) {
            const char *errorString = (const char*)gluErrorString(glErr);
            if (errorString)
                mError.append(QString("OpenGL error @ %1:%2: %3").arg(file).arg(line).arg(errorString));
            else
                mError.append(QString("Unknown OpenGL error @ %1:%2").arg(file).arg(line));
            errorOccured = true;
            glErr = glGetError();
        }

        return errorOccured;
    }

    bool GLSLProgram::loadFromFile( const QString &vertexShaderFile, const QString &fragmentShaderFile )
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

    bool GLSLProgram::load( const QByteArray &vertexShaderCode, const QByteArray &fragmentShaderCode )
    {

        release();

        // TODO: Fix this mess
        GLuint shader = glCreateShader(GL_VERTEX_SHADER);

        const char *code = vertexShaderCode.constData();
        glShaderSource(shader, 1, &code, 0);
        HANDLE_GL_ERROR
        vertexShader = shader;

        glCompileShader(vertexShader);
        HANDLE_GL_ERROR
        shader = glCreateShader(GL_FRAGMENT_SHADER);
        HANDLE_GL_ERROR

        code = fragmentShaderCode.constData();
        glShaderSource(shader, 1, &code, 0);
        HANDLE_GL_ERROR
        fragmentShader = shader;

        glCompileShader(fragmentShader);
        HANDLE_GL_ERROR

        shader = glCreateProgram();
        HANDLE_GL_ERROR

        glAttachShader(shader, vertexShader);
        HANDLE_GL_ERROR

        glAttachShader(shader, fragmentShader);
        HANDLE_GL_ERROR

        glLinkProgram(shader);
        HANDLE_GL_ERROR

        program = shader;

        return true;
    }

    void GLSLProgram::setUniformMatrix(const char *name, GLfloat *matrix)
    {
        GLint position = glGetUniformLocation(program, name);
        glUniformMatrix4fv(position, 16, false, matrix);
    }

    GLint GLSLProgram::attributeLocation(const char *name)
    {
        return glGetAttribLocation(program, name);
    }

    void GLSLProgram::release()
    {
        glUseProgram(0); // Avoid flagging for deletion, instead delete *now*

        if (program) {
            glDeleteProgram(program);
            program = 0;
        }

        if (vertexShader) {
            glDeleteShader(vertexShader);
            vertexShader = 0;
        }

        if (fragmentShader) {
            glDeleteShader(fragmentShader);
            fragmentShader = 0;
        }
    }

    GLint GLSLProgram::uniformLocation( const char *name )
    {
        GLint location;
        SAFE_GL(location = glGetUniformLocation(program, name));
        return location;
    }

}
