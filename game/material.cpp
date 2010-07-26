
#include <QtCore/QXmlStreamReader>

#include "material.h"

namespace EvilTemple {

static QHash<QString, GLenum> toggleableGlStates;

static GLenum getToggleableGlState(const QStringRef &stateName, bool *ok)
{
if (toggleableGlStates.isEmpty()) {
    toggleableGlStates["blend"] = GL_BLEND;
    toggleableGlStates["cullFace"] = GL_CULL_FACE;
    toggleableGlStates["depthTest"] = GL_DEPTH_TEST;
    toggleableGlStates["stencilTest"] = GL_STENCIL_TEST;
}

QHash<QString, GLenum>::const_iterator it = toggleableGlStates.find(stateName.toString());

if (it == toggleableGlStates.constEnd()) {
    *ok = false;
    return -1;
} else {
    return *it;
}
}

Material::Material()
{
}

Material::~Material()
{
    qDeleteAll(mPasses);
}

bool Material::loadFromFile(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        mError = file.errorString();
        return false;
    }

    return loadFromData(file.readAll());
}

bool Material::loadFromData(const QByteArray &data)
{
    QXmlStreamReader reader(data);

    return read(&reader);
}

bool Material::read(QXmlStreamReader *reader)
{
    if (reader->readNextStartElement()) {
        if (reader->name() != "material") {
            reader->raiseError(QString("The root element of the material file is %1. Should be material.")
                .arg(reader->name().toString()));
        } else {

            while (reader->readNextStartElement()) {
                if (reader->name() == "pass") {
                    MaterialPass *pass = new MaterialPass();
                    if (!pass->load(reader))
                        delete pass;
                    else
                        mPasses.append(pass);
                } else {
                    reader->raiseError("Expected pass element, got: " + reader->name().toString());
                }
            }

        }
    }

    if (reader->hasError()) {
        mError = QString("Parsing error @ %1:%2: %3").arg(reader->lineNumber())
            .arg(reader->columnNumber()).arg(reader->errorString());
        return false;
    }

    return true;
}

bool MaterialShader::load(QXmlStreamReader *reader)
{
    Q_ASSERT(reader->name() == "vertexShader" || reader->name() == "fragmentShader");

    mType = (reader->name() == "vertexShader") ? VertexShader : FragmentShader;

    mVersion = reader->attributes().value("version").toString();

    // Read content of shader
    while (reader->readNextStartElement()) {

        if (reader->name() == "include") {
            Q_ASSERT(reader->attributes().hasAttribute("file"));
            mIncludes.append(reader->attributes().value("file").toString());
            reader->skipCurrentElement();
        } else if (reader->name() == "code") {
            mCode = reader->readElementText();
        } else {
            reader->raiseError("Unknown vertex or fragment shader element.");
            return false;
        }
    }

    return true;
}

inline GLenum parseStencilOp(const QStringRef &stencilOp, bool *ok) {
    *ok = true;

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

    *ok = false;
    return GL_KEEP;
}

static GLenum parseBlendFunction(const QStringRef &stringRef, bool *ok)
{
    *ok = true;

    if (stringRef == "one")
        return GL_ONE;
    else if (stringRef == "zero")
        return GL_ZERO;
    else if (stringRef == "srcAlpha")
        return GL_SRC_ALPHA;
    else if (stringRef == "oneMinusSrcAlpha")
        return GL_ONE_MINUS_SRC_ALPHA;

    *ok = false;
    return GL_ZERO;
}

bool MaterialPass::load(QXmlStreamReader *reader)
{
    Q_ASSERT(reader->name() == "pass");

    if (!reader->readNextStartElement() || reader->name() != "shader") {
        reader->raiseError("First element of pass must be the shader element.");
        return false;
    }

    if (!reader->readNextStartElement() || reader->name() != "vertexShader") {
        reader->raiseError("Missing vertex shader element.");
        return false;
    }

    if (!mVertexShader.load(reader))
        return false;

    if (!reader->readNextStartElement() || reader->name() != "fragmentShader") {
        reader->raiseError("Missing fragment shader element.");
        return false;
    }

    if (!mFragmentShader.load(reader))
        return false;

    // The rest can be a mix of attributes and uniforms
    while (reader->readNextStartElement()) {
        if (reader->name() == "attribute") {
            MaterialAttributeBinding binding;

            if (!binding.load(reader)) {
                return false;
            }

            mAttributeBindings.append(binding);
        } else if (reader->name() == "uniform") {
            MaterialUniformBinding binding;

            if (!binding.load(reader)) {
                return false;
            }

            mUniformBindings.append(binding);
        } else {
            reader->raiseError("Unknown child-element of shader: " + reader->name().toString());
            return false;
        }
    }

    // Read the rest of the pass element
    while (reader->readNextStartElement()) {
        QStringRef name = reader->name();

        if (name == "textureSampler") {
            MaterialTextureSampler sampler;

            if (!sampler.load(reader)) {
                return false;
            }

            mTextureSamplers.append(sampler);

        } else if (name == "blendFunc") {
            QStringRef srcFunc = reader->attributes().value("src");
            bool ok;

            // TODO: Expand for all other values. Don't add state changer if it's the default anyway
            GLenum srcFuncEnum = parseBlendFunction(srcFunc, &ok);
            if (!ok) {
                reader->raiseError("Unknown source blending function.");
                return false;
            }

            QStringRef destFunc = reader->attributes().value("dest");
            GLenum destFuncEnum = parseBlendFunction(destFunc, &ok);

            if (!ok) {
                reader->raiseError("Unknown destination blending function.");
                return false;
            }

            SharedMaterialRenderState renderState(new MaterialBlendFunction(srcFuncEnum, destFuncEnum));
            mRenderStates.append(renderState);

            reader->skipCurrentElement();

        } else if (name == "depthWrite") {

            QString text = reader->readElementText();
            if (text == "true") {
                SharedMaterialRenderState renderState(new MaterialDepthMask(true));
                mRenderStates.append(renderState);
            } else if (text == "false") {
                SharedMaterialRenderState renderState(new MaterialDepthMask(false));
                mRenderStates.append(renderState);
            } else {
                reader->raiseError("Boolean (true or false) expected.");
                return false;
            }

        } else if (name == "stencilFunc") {

            // This encodes default state
            GLenum func = GL_ALWAYS;
            GLint ref = 0;
            GLuint mask = ~0;

            bool ok;

            if (reader->attributes().hasAttribute("function")) {
                QStringRef funcString = reader->attributes().value("function");

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
                else {
                    reader->raiseError("Unknown stencil function.");
                    return false;
                }
            }

            if (reader->attributes().hasAttribute("reference")) {
                ref = reader->attributes().value("reference").toString().toInt(&ok);

                if (!ok) {
                    reader->raiseError("Invalid stencil function reference.");
                    return false;
                }
            }

            if (reader->attributes().hasAttribute("mask")) {
                mask = reader->attributes().value("mask").toString().toUInt(&ok);

                if (!ok) {
                    reader->raiseError("Invalid stencil function mask.");
                    return false;
                }
            }

            SharedMaterialRenderState renderState(new StencilFuncState(func, ref, mask));
            mRenderStates.append(renderState);

            reader->skipCurrentElement();

        } else if (name == "clearStencil") {

            GLint value = 0;

            if (reader->attributes().hasAttribute("value")) {
                bool ok;
                value = reader->attributes().value("value").toString().toInt(&ok);

                if (!ok) {
                    reader->raiseError("Invalid value.");
                    return false;
                }
            }

            SharedMaterialRenderState renderState(new ClearStencilState(value));
            mRenderStates.append(renderState);

            reader->skipCurrentElement();

        } else if (name == "stencilOp") {

            bool failOk, zFailOk, zPassOk;
            GLenum fail = GL_KEEP;
            GLenum zFail = GL_KEEP;
            GLenum zPass = GL_KEEP;

            if (reader->attributes().hasAttribute("stencilFail"))
                fail = parseStencilOp(reader->attributes().value("stencilFail"), &failOk);
            if (reader->attributes().hasAttribute("depthFail"))
                zFail = parseStencilOp(reader->attributes().value("depthFail"), &zFailOk);
            if (reader->attributes().hasAttribute("depthPass"))
                zPass= parseStencilOp(reader->attributes().value("depthPass"), &zPassOk);

            if (!failOk || !zFailOk || !zPassOk) {
                reader->raiseError("Invalid stencil operation.");
                return false;
            }

            SharedMaterialRenderState renderState(new StencilOpState(fail, zFail, zPass));
            mRenderStates.append(renderState);

            reader->skipCurrentElement();

        } else if (name == "colorMask") {
            bool red = true;
            if (reader->attributes().hasAttribute("red"))
                red = reader->attributes().value("red") == "true";

            bool green = true;
            if (reader->attributes().hasAttribute("green"))
                green = reader->attributes().value("green") == "true";

            bool blue = true;
            if (reader->attributes().hasAttribute("blue"))
                blue = reader->attributes().value("blue") == "true";

            bool alpha = true;
            if (reader->attributes().hasAttribute("alpha"))
                alpha = reader->attributes().value("alpha") == "true";

            SharedMaterialRenderState renderState(new MaterialColorMaskState(red, green, blue, alpha));
            mRenderStates.append(renderState);

            reader->skipCurrentElement();

        } else {
            bool ok;
            GLenum state = getToggleableGlState(name, &ok);

            if (!ok) {
                reader->raiseError("Unknown pass element: " + reader->name().toString());
                return false;
            }

            QString text = reader->readElementText();
            if (text == "true") {
                SharedMaterialRenderState renderState(new MaterialEnableState(state));
                mRenderStates.append(renderState);
            } else if (text == "false") {
                SharedMaterialRenderState renderState(new MaterialDisableState(state));
                mRenderStates.append(renderState);
            } else {
                reader->raiseError("Boolean (true/false) expected.");
                return false;
            }
        }
    }

    return true;
}

MaterialAttributeBinding::MaterialAttributeBinding()
    : mComponents(4), mType(Float), mNormalized(false), mStride(0), mOffset(0)
{
}

bool MaterialAttributeBinding::load(QXmlStreamReader *reader)
{
    Q_ASSERT(reader->attributes().hasAttribute("name"));
    Q_ASSERT(reader->attributes().hasAttribute("buffer"));

    mName = reader->attributes().value("name").toString();
    mBufferName = reader->attributes().value("buffer").toString();

    if (reader->attributes().hasAttribute("components")) {
        bool ok;
        int val = reader->attributes().value("components").toString().toInt(&ok);

        if (!ok || val < 1 || val > 4) {
            reader->raiseError("Invalid number of attribute components. Must be 1-4.");
            return false;
        }

        mComponents = val;
    }

    if (reader->attributes().hasAttribute("type")) {
        QStringRef typeName = reader->attributes().value("type");
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
            reader->raiseError("Unknown attribute type name.");
            return false;
        }
    } else {
        mType = Float;
    }

    if (reader->attributes().hasAttribute("normalized")) {
        QStringRef normalized = reader->attributes().value("normalized");

        if (normalized == "true") {
            mNormalized = true;
        } else if (normalized == "false") {
            mNormalized = false;
        } else {
            reader->raiseError("Expected boolean for normalized attribute.");
            return false;
        }
    } else {
        mNormalized = false;
    }

    if (reader->attributes().hasAttribute("stride")) {
        bool ok;
        int value = reader->attributes().value("stride").toString().toInt(&ok);

        if (!ok || value < 0) {
            reader->raiseError("Invalid stride.");
            return false;
        }

        mStride = value;
    } else {
        mStride = 0;
    }

    if (reader->attributes().hasAttribute("offset")) {
        bool ok;
        int value = reader->attributes().value("offset").toString().toInt(&ok);

        if (!ok || value < 0) {
            reader->raiseError("Invalid offset.");
            return false;
        }

        mOffset = value;
    } else {
        mOffset = 0;
    }

    reader->skipCurrentElement();

    return true;
}

inline static float attributeToFloat(QXmlStreamReader *reader, const QXmlStreamAttributes &attributes, const char *name)
{
    bool ok;
    float result = attributes.value(name).toString().toFloat(&ok);
    if (!ok) {
        reader->raiseError(QString("Attribute %1 is missing.").arg(name));
    }
    return result;
}

inline static int attributeToInt(QXmlStreamReader *reader, const QXmlStreamAttributes &attributes, const char *name)
{
    bool ok;
    int result = attributes.value(name).toString().toInt(&ok);
    if (!ok) {
        reader->raiseError(QString("Attribute %1 is missing.").arg(name));
    }
    return result;
}

bool MaterialUniformBinding::load(QXmlStreamReader *reader)
{
    QXmlStreamAttributes attributes = reader->attributes();

    Q_ASSERT(attributes.hasAttribute("name"));

    if (attributes.hasAttribute("optional"))
        mOptional = attributes.value("optional") == "true";
    else
        mOptional = true;

    mName = attributes.value("name").toString();

    if (attributes.hasAttribute("semantic"))
        mSemantic = attributes.value("semantic").toString();
    else
        mSemantic = "Constant";

   if (!reader->readNextStartElement())
       return true; // No constant value

    // Retrieve the constant value
    QStringRef constantType = reader->name();

    attributes = reader->attributes(); // Refresh attributes vector

    if (constantType == "float4") {
        QVector4D value;
        if (attributes.hasAttribute("x"))
            value.setX(attributeToFloat(reader, attributes, "x"));
        if (attributes.hasAttribute("r"))
            value.setX(attributeToFloat(reader, attributes, "r"));

        if (attributes.hasAttribute("y"))
            value.setY(attributeToFloat(reader, attributes, "y"));
        if (attributes.hasAttribute("g"))
            value.setY(attributeToFloat(reader, attributes, "g"));

        if (attributes.hasAttribute("z"))
            value.setZ(attributeToFloat(reader, attributes, "z"));
        if (attributes.hasAttribute("b"))
            value.setZ(attributeToFloat(reader, attributes, "b"));

        if (attributes.hasAttribute("w"))
            value.setW(attributeToFloat(reader, attributes, "w"));
        if (attributes.hasAttribute("a"))
            value.setW(attributeToFloat(reader, attributes, "a"));
        mConstantValue = QVariant(value);
    } else if (constantType == "float3") {
        QVector3D value;
        if (attributes.hasAttribute("x"))
            value.setX(attributeToFloat(reader, attributes, "x"));
        if (attributes.hasAttribute("r"))
            value.setX(attributeToFloat(reader, attributes, "r"));

        if (attributes.hasAttribute("y"))
            value.setY(attributeToFloat(reader, attributes, "y"));
        if (attributes.hasAttribute("g"))
            value.setY(attributeToFloat(reader, attributes, "g"));

        if (attributes.hasAttribute("z"))
            value.setZ(attributeToFloat(reader, attributes, "z"));
        if (attributes.hasAttribute("b"))
            value.setZ(attributeToFloat(reader, attributes, "b"));

        mConstantValue = QVariant(value);
    } else if (constantType == "float2") {
        QVector2D value;
        if (attributes.hasAttribute("x"))
            value.setX(attributeToFloat(reader, attributes, "x"));
        if (attributes.hasAttribute("u"))
            value.setX(attributeToFloat(reader, attributes, "r"));

        if (attributes.hasAttribute("y"))
            value.setY(attributeToFloat(reader, attributes, "u"));
        if (attributes.hasAttribute("u"))
            value.setY(attributeToFloat(reader, attributes, "v"));

        mConstantValue = QVariant(value);
    } else if (constantType == "float") {
        float value = 0;
        if (attributes.hasAttribute("value")) {
            value = attributeToFloat(reader, attributes, "value");
        }
        mConstantValue = QVariant(value);
    } else if (constantType == "int") {
        int value = 0;
        if (attributes.hasAttribute("value")) {
            value = attributeToInt(reader, attributes, "value");
        }
        mConstantValue = QVariant(value);
    } else {
        reader->raiseError("Unknown constant type.");
        return false;
    }

    QXmlStreamReader::TokenType token = reader->readNext();

    if (token != QXmlStreamReader::EndElement) {
        reader->raiseError("Constant uniform elements must not have a body.");
        return false;
    }

    if (reader->readNextStartElement()) {
        reader->raiseError("Uniform elements may only have one child-element.");
        return false;
    }

    return !reader->hasError();
}

static MaterialTextureSampler::WrapMode parseWrapMode(const QStringRef &text) {
    if (text == "repeat")
        return MaterialTextureSampler::Repeat;
    else if (text == "wrap")
        return MaterialTextureSampler::Wrap;
    else if (text == "clamp")
        return MaterialTextureSampler::Clamp;

    qWarning("Unknown texture sampler wrap mode: %s", qPrintable(text.toString()));
    return MaterialTextureSampler::Repeat;
}

bool MaterialTextureSampler::load(QXmlStreamReader *reader)
{
    Q_ASSERT(reader->attributes().hasAttribute("texture"));

    mTexture = reader->attributes().value("texture").toString();

    if (reader->attributes().hasAttribute("wrapU"))
        mWrapU = parseWrapMode(reader->attributes().value("wrapU"));

    if (reader->attributes().hasAttribute("wrapV"))
        mWrapV = parseWrapMode(reader->attributes().value("wrapV"));

    reader->skipCurrentElement();

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

