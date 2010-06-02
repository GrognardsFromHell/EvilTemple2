
#include <QtXml/QtXml>

#include "material.h"

namespace EvilTemple {

static const QString PASS_TAG = "pass";

Material::Material()
{

}

Material::~Material()
{
    qDeleteAll(mPasses);
    mPasses.clear();
}

bool Material::loadFromFile(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        mError = file.error();
        return false;
    }

    return loadFromData(file.readAll());
}

bool Material::loadFromData(const QByteArray &data)
{
    QString errorMsg;
    int errorLine, errorColumn;

    QDomDocument document;
    if (!document.setContent(data, false, &errorMsg, &errorLine, &errorColumn)) {
        mError = QString("[%1:%2] %3.").arg(errorLine).arg(errorColumn).arg(errorMsg);
        return false;
    }

    QDomElement documentRoot = document.documentElement();

    QDomElement passNode = documentRoot.firstChildElement(PASS_TAG);

    while (!passNode.isNull()) {
        MaterialPass *pass = new MaterialPass;
        mPasses.append(pass);
        pass->load(passNode);

        passNode = passNode.nextSiblingElement(PASS_TAG);
    }

    return true;
}

inline GLenum convertStencilOp(const QString &stencilOp) {
    if (stencilOp == "keep")
        return GL_KEEP;
    else if (stencilOp == "zero")
        return GL_ZERO;
    else if (stencilOp == "replace")
        return GL_REPLACE;
    else if (stencilOp == "increment")
        return GL_INCR;
    else if (stencilOp == "incrementWrap")
        return GL_INCR_WRAP;
    else if (stencilOp == "decrement")
        return GL_DECR;
    else if (stencilOp == "decrementWrap")
        return GL_DECR_WRAP;
    else if (stencilOp == "invert")
        return GL_INVERT;
    else
        qWarning("Invalid stencil op: %s.", qPrintable(stencilOp));

    return GL_KEEP;
}

bool MaterialPass::load(const QDomElement &passElement)
{
    QDomElement shaderElement = passElement.firstChildElement("shader");
    Q_ASSERT(!shaderElement.isNull());

    QDomNodeList children = shaderElement.childNodes();

    for (int i = 0; i < children.size(); ++i) {
        QDomElement element = children.item(i).toElement();

        if (element.isNull())
            continue;

        if (element.nodeName() == "vertexShader") {
            if (!mVertexShader.load(element)) {
                return false;
            }
        } else if (element.nodeName() == "fragmentShader") {
            if (!mFragmentShader.load(element)) {
                return false;
            }
        } else if (element.nodeName() == "attribute") {
            MaterialAttributeBinding binding;

            if (!binding.load(element)) {
                return false;
            }

            mAttributeBindings.append(binding);
        } else if (element.nodeName() == "uniform") {
            MaterialUniformBinding binding;

            if (!binding.load(element)) {
                return false;
            }

            mUniformBindings.append(binding);
        } else {
            qWarning("Unknown child-element of pass: %s", qPrintable(element.nodeName()));
            return false;
        }
    }

    QDomElement textureSamplerElement = shaderElement.nextSiblingElement("textureSampler");

    while (!textureSamplerElement.isNull()) {
        MaterialTextureSampler sampler;

        if (!sampler.load(textureSamplerElement)) {
            return false;
        }

        mTextureSamplers.append(sampler);

        textureSamplerElement = textureSamplerElement.nextSiblingElement("textureSampler");
    }

    QDomElement blendFuncElement = passElement.firstChildElement("blendFunc");
    if (!blendFuncElement.isNull()) {
        QString srcFunc = blendFuncElement.attribute("src");
        QString destFunc = blendFuncElement.attribute("dest");

        // TODO: Expand for all other values. Don't add state changer if it's the default anyway
        GLenum srcFuncEnum = GL_ZERO;
        if (srcFunc == "one") {
            srcFuncEnum = GL_ONE;
        } else if (srcFunc == "srcAlpha") {
            srcFuncEnum = GL_SRC_ALPHA;
        }
        GLenum destFuncEnum = GL_ZERO;
        if (destFunc == "one") {
            destFuncEnum = GL_ONE;
        } else if (destFunc == "srcAlpha") {
            destFuncEnum = GL_SRC_ALPHA;
        } else if (destFunc == "oneMinusSrcAlpha") {
            destFuncEnum = GL_ONE_MINUS_SRC_ALPHA;
        }

        SharedMaterialRenderState renderState(new MaterialBlendFunction(srcFuncEnum, destFuncEnum));
        mRenderStates.append(renderState);
    }

    QDomElement cullFaceElement = passElement.firstChildElement("cullFace");
    if (!cullFaceElement.isNull()) {
        QString cullFaces = cullFaceElement.text();

        // TODO: Sanity check
        if (cullFaces == "false") {
            SharedMaterialRenderState renderState(new MaterialDisableState(GL_CULL_FACE));
            mRenderStates.append(renderState);
        }
    }

    QDomElement depthWriteElement = passElement.firstChildElement("depthWrite");
    if (!depthWriteElement.isNull()) {
        QString depthWrite = depthWriteElement.text();

        // TODO: Sanity check
        if (depthWrite == "false") {
            SharedMaterialRenderState renderState(new MaterialDepthMask(false));
            mRenderStates.append(renderState);
        }
    }

    QDomElement depthTestElement = passElement.firstChildElement("depthTest");
    if (!depthTestElement.isNull()) {
        QString depthTest = depthTestElement.text();

        // TODO: Sanity check
        if (depthTest == "false") {
            SharedMaterialRenderState renderState(new MaterialDisableState(GL_DEPTH_TEST));
            mRenderStates.append(renderState);
        }
    }

    QDomElement stencilTestElement = passElement.firstChildElement("stencilTest");
    if (!stencilTestElement.isNull()) {
        QString stencilTest = stencilTestElement.text();

        // TODO: Sanity check
        if (stencilTest == "true") {
            SharedMaterialRenderState renderState(new MaterialEnableState(GL_STENCIL_TEST));
            mRenderStates.append(renderState);
        }
    }

    QDomElement stencilFuncElement = passElement.firstChildElement("stencilFunc");
    if (!stencilFuncElement.isNull()) {
        // This encodes default state
        GLenum func = GL_ALWAYS;
        GLint ref = 0;
        GLuint mask = ~0;

        QString funcString = stencilFuncElement.attribute("function", "always");
        if (funcString == "always")
            func = GL_ALWAYS;
        else if (funcString == "never")
            func = GL_NEVER;
        else if (funcString == "less")
            func = GL_LESS;
        else if (funcString == "lequal")
            func = GL_LEQUAL;
        else if (funcString == "greater")
            func = GL_GREATER;
        else if (funcString == "gequal")
            func = GL_GEQUAL;
        else if (funcString == "equal")
            func = GL_EQUAL;
        else if (funcString == "notequal")
            func = GL_NOTEQUAL;
        else
            qWarning("Unknown stencil function: %s", qPrintable(funcString));

        bool ok;
        ref = stencilFuncElement.attribute("reference", "0").toInt(&ok);

        if (!ok)
            qWarning("Invalid stencil function reference: %s", stencilFuncElement.attribute("reference"));

        if (stencilFuncElement.hasAttribute("mask"))
            mask = stencilFuncElement.attribute("mask").toUInt(&ok);

        if (!ok)
            qWarning("Invalid stencil function mask: %s", stencilFuncElement.attribute("mask"));

        SharedMaterialRenderState renderState(new StencilFuncState(func, ref, mask));
        mRenderStates.append(renderState);
    }

    QDomElement clearStencilElement = passElement.firstChildElement("clearStencil");
    if (!clearStencilElement.isNull()) {
        bool ok;
        GLint value = clearStencilElement.attribute("value", "0").toInt(&ok);
        Q_ASSERT(ok);

        SharedMaterialRenderState renderState(new ClearStencilState(value));
        mRenderStates.append(renderState);
    }

    QDomElement stencilOpElement = passElement.firstChildElement("stencilOp");
    if (!stencilOpElement.isNull()) {
        GLenum fail = convertStencilOp(stencilOpElement.attribute("stencilFail", "keep"));
        GLenum zFail = convertStencilOp(stencilOpElement.attribute("depthFail", "keep"));
        GLenum zPass= convertStencilOp(stencilOpElement.attribute("depthPass", "keep"));

        SharedMaterialRenderState renderState(new StencilOpState(fail, zFail, zPass));
        mRenderStates.append(renderState);
    }

    QDomElement colorMaskElement = passElement.firstChildElement("colorMask");
    if (!colorMaskElement.isNull()) {
        bool red = colorMaskElement.attribute("red", "true") == "true";
        bool green = colorMaskElement.attribute("green", "true") == "true";
        bool blue = colorMaskElement.attribute("blue", "true") == "true";
        bool alpha = colorMaskElement.attribute("alpha", "true") == "true";

        // TODO: Need some way to determine default state without encoding it here
        if (!red || !green || !blue || !alpha) {
            SharedMaterialRenderState renderState(new MaterialColorMaskState(red, green, blue, alpha));
            mRenderStates.append(renderState);
        }
    }

    return true;
}

bool MaterialShader::load(const QDomElement &shaderElement)
{
    if (shaderElement.nodeName() == "vertexShader") {
        mType = VertexShader;
    } else if (shaderElement.nodeName() == "fragmentShader") {
        mType = FragmentShader;
    } else {
        qWarning("Shader element has invalid name %s.", qPrintable(shaderElement.nodeName()));
        return false;
    }

    mVersion = shaderElement.attribute("version");

    QDomElement childElement = shaderElement.firstChildElement();
    while (!childElement.isNull()) {
        QString childName = childElement.nodeName();

        if (childName == "code") {
            mCode = childElement.text();
        } else if (childName == "include") {
            Q_ASSERT(childElement.hasAttribute("file"));
            mIncludes.append(childElement.attribute("file"));
        } else if (childName == "code") {
            mCode = childElement.text();
        } else {
            qFatal("Unknown sub-element of shader element: %s", qPrintable(childName));
        }

        childElement = childElement.nextSiblingElement();
    }

    return true;
}

MaterialAttributeBinding::MaterialAttributeBinding()
    : mComponents(4), mType(Float), mNormalized(false), mStride(0), mOffset(0)
{
}

bool MaterialAttributeBinding::load(const QDomElement &element)
{
    Q_ASSERT(element.hasAttribute("name"));
    Q_ASSERT(element.hasAttribute("buffer"));

    mName = element.attribute("name");
    mBufferName = element.attribute("buffer");

    bool ok;
    int value = element.attribute("components", "4").toInt(&ok);

    if (!ok || value < 1 || value > 4) {
        qWarning("Invalid number of components for attribute: %s.", qPrintable(element.attribute("components")));
        return false;
    }

    mComponents = value;

    QString typeName = element.attribute("type", "float");
    if (typeName == "byte") {
        mType = Byte;
    } else if (typeName == "unsigned_byte") {
        mType = UnsignedByte;
    } else if (typeName == "short") {
        mType = Short;
    } else if (typeName == "unsigned_short") {
        mType = UnsignedShort;
    } else if (typeName == "integer") {
        mType = Integer;
    } else if (typeName == "unsigned_integer") {
        mType = UnsignedInteger;
    } else if (typeName == "float") {
        mType = Float;
    } else if (typeName == "double") {
        mType = Double;
    } else {
        qWarning("Unknown type name for attribute: %s", qPrintable(typeName));
        return false;
    }

    QString normalized = element.attribute("normalized", "false");

    if (normalized == "true") {
        mNormalized = true;
    } else if (normalized == "false") {
        mNormalized = false;
    } else {
        qWarning("Invalid value for normalized, only true and false are allowed: %s.", qPrintable(normalized));
        return false;
    }

    value = element.attribute("stride", "0").toInt(&ok);

    if (!ok || value < 0) {
        qWarning("Invalid stride for attribute: %s.", qPrintable(element.attribute("stride")));
        return false;
    }

    mStride = value;

    value = element.attribute("offset", "0").toInt(&ok);

    if (!ok || value < 0) {
        qWarning("Invalid offset for attribute: %s.", qPrintable(element.attribute("offset")));
        return false;
    }

    mOffset = value;

    return true;
}

bool MaterialUniformBinding::load(const QDomElement &element)
{
    Q_ASSERT(element.hasAttribute("name"));
    Q_ASSERT(element.hasAttribute("semantic"));

    mOptional = element.attribute("optional", "true") == "true";
    mName = element.attribute("name");
    mSemantic = element.attribute("semantic");

    return true;
}

bool MaterialTextureSampler::load(const QDomElement &element)
{
    Q_ASSERT(element.hasAttribute("texture"));

    mTexture = element.attribute("texture");

    return true;
}

MaterialRenderState::~MaterialRenderState()
{
}

void MaterialBlendFunction::enable()
{
    glBlendFunc(mSrcFactor, mDestFactor);
}

void MaterialBlendFunction::disable()
{
    glBlendFunc(GL_ONE, GL_ZERO);
}

}
