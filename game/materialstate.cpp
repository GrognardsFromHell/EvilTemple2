
#include <GL/glew.h>

#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QCryptographicHash>

#include "materialstate.h"
#include "material.h"

namespace EvilTemple {

static uint activeMaterialStates = 0;

typedef QSharedPointer<GLSLProgram> SharedGLSLProgram;

static QHash<QByteArray, SharedGLSLProgram> shaderCache;

uint getActiveMaterialStates()
{
    return activeMaterialStates;
}

class NullBinder : public UniformBinder
{
    void bind(GLint) const;
};

void NullBinder::bind(GLint) const
{
}

static NullBinder nullBinderInstance;

UniformBinder::~UniformBinder()
{
}

MaterialState::MaterialState() : passes((MaterialPassState*)NULL)
{
    activeMaterialStates++;
}

MaterialState::~MaterialState()
{
    activeMaterialStates--;
}

bool MaterialState::createFromFile(const QString &filename, const RenderStates &renderState, TextureSource *textureSource)
{
    Material material;

    if (!material.loadFromFile(filename)) {
        mError = material.error();
        return false;
    }

    return createFrom(material, renderState, textureSource);
}

QByteArray getFullCode(const MaterialShader &shader) {
    QByteArray result;

    // Prepend version if specified
    if (!shader.version().isEmpty()) {
        result.append("#version ");
        result.append(shader.version());
        result.append("\n");
    }

    foreach (const QString &includedFile, shader.includes()) {
        QFile file(includedFile);

        if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            qWarning("Couldn't open shader include file %s.", qPrintable(includedFile));
            continue;
        }

        result.append('\n');
        result.append(file.readAll());
        result.append('\n');
    }

    result.append(shader.code().toLatin1());

    return result;
}

static UniformBinder *createConstantBinder(const MaterialUniformBinding &binding)
{
    const QVariant &constantValue = binding.constantValue();

    QVariant::Type type = constantValue.type();

    if (type == qMetaTypeId<QVector2D>()) {
        return new ConstantBinder<QVector2D>(constantValue.value<QVector2D>());

    } else if (type == qMetaTypeId<QVector3D>()) {
        return new ConstantBinder<QVector3D>(constantValue.value<QVector3D>());

    } else if (type == qMetaTypeId<QVector4D>()) {
        return new ConstantBinder<QVector4D>(constantValue.value<QVector4D>());

    } else if (type == QVariant::Double) {
        return new ConstantBinder<float>(constantValue.toDouble());

    } else if (type == QVariant::Int) {
        return new ConstantBinder<int>(constantValue.toInt());

    } else if (type == QVariant::UInt) {
        return new ConstantBinder<uint>(constantValue.toUInt());
    }

    qWarning("Trying to bind an unknown QVariant type to a uniform: %s.", constantValue.typeName());
    return NULL;
}

bool MaterialState::createFrom(const Material &material, const RenderStates &states, TextureSource *textureSource)
{
    passCount = material.passes().size();
    passes.reset(new MaterialPassState[passCount]);

    for (int i = 0; i < passCount; ++i) {
        MaterialPassState &passState = passes[i];
        const MaterialPass *pass = material.passes()[i];

        passState.renderStates = pass->renderStates();

        QByteArray vertexShaderCode = getFullCode(pass->vertexShader());
        QByteArray fragmentShaderCode = getFullCode(pass->fragmentShader());

        QCryptographicHash shaderHash(QCryptographicHash::Md5);
        shaderHash.addData(vertexShaderCode);
        shaderHash.addData(fragmentShaderCode);
        QByteArray hashValue = shaderHash.result();

        if (shaderCache.contains(hashValue)) {
            passState.program = shaderCache[hashValue];
        } else {
            passState.program = SharedGLSLProgram::create();

            if (!passState.program->load(vertexShaderCode, fragmentShaderCode)) {
                mError = QString("Unable to compile shader:\n%1").arg(passState.program->error());
                return false;
            }

            shaderCache[hashValue] = passState.program;
        }

        // Process textures
        passState.textureSamplers.resize(pass->textureSamplers().size());
        for (int j = 0; j < pass->textureSamplers().size(); ++j) {
            const MaterialTextureSampler &sampler = pass->textureSamplers()[j];
            MaterialTextureSamplerState &state = passState.textureSamplers[j];

            state.setSamplerId(j);

            state.setTexture(textureSource->loadTexture(sampler.texture()));
        }

        passState.attributes.resize(pass->attributeBindings().size());
        // Get all uniforms/attribute locations to enable quick access
        for (int j = 0; j < pass->attributeBindings().size(); ++j) {
            const MaterialAttributeBinding &binding = pass->attributeBindings()[j];
            MaterialPassAttributeState &attribute = passState.attributes[j];

            if (binding.bufferName() == "positions") {
                attribute.bufferType = 0;
            } else if (binding.bufferName() == "normals") {
                attribute.bufferType = 1;
            } else if (binding.bufferName() == "texCoords") {
                attribute.bufferType = 2;
            } else {
                attribute.bufferType = -1;
                qWarning("Unknown buffer type: %s.", qPrintable(binding.bufferName()));
            }

            // Find the corresponding attribute index in the shader
            attribute.location = passState.program->attributeLocation(qPrintable(binding.name()));
            if (attribute.location == -1) {
                mError = QString("Unable to find attribute location for '%1' in shader.").arg(binding.name());
                return false;
            }
            attribute.binding = binding;
        }

        // Process uniform bindings
        passState.program->bind();
        for (int j = 0; j < pass->uniformBindings().size(); ++j) {
            const MaterialUniformBinding &binding = pass->uniformBindings()[j];

            GLint location = passState.program->uniformLocation(qPrintable(binding.name()));

            const UniformBinder *immutableBinder = NULL;
            UniformBinder *binder = NULL;

            QString semantic = binding.semantic();

            // The binder used depends on the semantic, this has to be moved to the "engine" part
            if (semantic.startsWith("Texture")) {
                bool ok;
                int sampler = semantic.right(semantic.length() - strlen("Texture")).toInt(&ok);
                if (!ok) {
                    mError = QString("Invalid texture semantic found: %1.").arg(binding.semantic());
                    return false;
                }

                glUniform1i(location, sampler);
                continue; // This value only needs to be set once
            } else if (semantic == "Constant") {
                binder = createConstantBinder(binding);
            } else {
                immutableBinder = states.getStateBinder(binding.semantic());
            }

            if (!binder && !immutableBinder) {
                mError = QString("Unknown semantic %1 for uniform %2.").arg(binding.semantic()).arg(binding.name());
                return false;
            }

            MaterialPassUniformState *state = new MaterialPassUniformState;

            if (location == -1) {
                if (binding.isOptional()) {
                    qWarning("Unable to find uniform location for '%s' in shader.", qPrintable(binding.name()));
                    delete state;
                    continue;
                } else {
                    delete state;
                    mError = QString("Unable to find uniform location for '%1' in shader.").arg(binding.name());
                    return false;
                }
            }

            state->setLocation(location);

            if (binder) {
                state->setBinder(binder);
            } else {
                Q_ASSERT(immutableBinder);
                state->setImmutableBinder(immutableBinder);
            }
            passState.uniforms.append(state);
        }
        passState.program->unbind();
    }

    return true;
}

MaterialPassState::MaterialPassState()
{
}

MaterialPassState::~MaterialPassState()
{
    qDeleteAll(uniforms);
}

}
