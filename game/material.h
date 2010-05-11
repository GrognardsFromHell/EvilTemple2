#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <QByteArray>
#include <QHash>

namespace EvilTemple {

class MaterialData;

class UniformBinding {
public:

};

/**
  * Describes a material, which is applied to the faces of a model.
  * This does not contain any actual resource handles to OpenGL resources.
  */
class Material
{
public:
    Material(const QString &name);

    const QByteArray &vertexShader() const;
    void setVertexShader(const QByteArray &code);

    const QByteArray &fragmentShader() const;
    void setFragmentShader(const QByteArray &code);

    const QString &name() const;
    void setName(const QString &name);

    bool noBackfaceCulling() const;
    void setNoBackfaceCulling(bool disableCulling);

    const QHash<QByteArray,UniformBinding> &uniformBindings() const;

private:
    QByteArray mVertexShader;
    QByteArray mFragmentShader;
    QString mName;
    bool mNoBackfaceCulling;
    bool mNoDepthTest;
    QHash<QByteArray, UniformBinding> mUniformBindings;
};

const QByteArray &Material::vertexShader() const
{
    return mVertexShader;
}

void Material::setVertexShader(const QByteArray &code)
{
    mVertexShader = code;
}

const QByteArray &Material::fragmentShader() const
{
    return mFragmentShader;
}

void Material::setFragmentShader(const QByteArray &code)
{
    mFragmentShader = code;
}

const QString &Material::name() const
{
    return mName;
}

void Material::setName(const QString &name)
{
    mName = name;
}

bool Material::noBackfaceCulling() const
{
    return mNoBackfaceCulling;
}

void Material::setNoBackfaceCulling(bool disableCulling)
{
    mNoBackfaceCulling = disableCulling;
}

}

#endif // MATERIAL_H
