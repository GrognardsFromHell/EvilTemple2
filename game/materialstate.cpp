
#include <GL/glew.h>

#include "materialstate.h"
#include "material.h"

namespace EvilTemple {

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

bool MaterialState::createFromFile(const QString &filename, const RenderStates &renderState, TextureSource *textureSource)
{
    Material material;

    if (!material.loadFromFile(filename)) {
        mError = material.error();
        return false;
    }

    return createFrom(material, renderState, textureSource);
}

QString getFullCode(const MaterialShader &shader) {
    QString result;

    // Prepend version if specified
    if (!shader.version().isEmpty()) {
        result = "#version " + shader.version() + "\n";
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

    result.append(shader.code());

    return result;
}

bool MaterialState::createFrom(const Material &material, const RenderStates &states, TextureSource *textureSource)
{
    passCount = material.passes().size();
    passes.reset(new MaterialPassState[passCount]);

    for (int i = 0; i < passCount; ++i) {
        MaterialPassState &passState = passes[i];
        const MaterialPass *pass = material.passes()[i];

        passState.renderStates = pass->renderStates();


        if (!passState.program.load(qPrintable(getFullCode(pass->vertexShader())), qPrintable(getFullCode(pass->fragmentShader())))) {
            mError = QString("Unable to compile shader:\n%1").arg(passState.program.error());
            return false;
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
                qFatal("Unknown buffer type: %s.", qPrintable(binding.bufferName()));
            }

            // Find the corresponding attribute index in the shader
            attribute.location = passState.program.attributeLocation(qPrintable(binding.name()));
            if (attribute.location == -1) {
                mError = QString("Unable to find attribute location for '%1' in shader.").arg(binding.name());
                return false;
            }
            attribute.binding = binding;
        }

        // Process uniform bindings
        passState.program.bind();
        for (int j = 0; j < pass->uniformBindings().size(); ++j) {
            const MaterialUniformBinding &binding = pass->uniformBindings()[j];
            MaterialPassUniformState state;

            GLint location = passState.program.uniformLocation(qPrintable(binding.name()));
            if (location == -1) {
                if (binding.isOptional()) {
                    qWarning("Unable to find uniform location for '%s' in shader.", qPrintable(binding.name()));
                    state.setBinder(&nullBinderInstance);
                    state.setLocation(-1);
                    continue;
                } else {
                    mError = QString("Unable to find uniform location for '%1' in shader.").arg(binding.name());
                    return false;
                }
            }

            state.setLocation(location);

            const UniformBinder *binder = 0;

            // The binder used depends on the semantic, this has to be moved to the "engine" part
            if (binding.semantic().startsWith("Texture")) {
                bool ok;
                int sampler = binding.semantic().right(binding.semantic().length() - 7).toInt(&ok);
                if (!ok) {
                    mError = QString("Invalid texture semantic found: %1.").arg(binding.semantic());
                    return false;
                }

                glUniform1i(location, sampler);
                continue; // This value only needs to be set once
            } else {
                binder = states.getStateBinder(binding.semantic());
            }

            if (!binder) {
                mError = QString("Unknown semantic %1 for uniform %2.").arg(binding.semantic()).arg(binding.name());
                return false;
            }

            state.setBinder(binder);
            passState.uniforms.append(state);
        }
        passState.program.unbind();
    }

    return true;
}

MaterialPassState::MaterialPassState()
{
}

}
