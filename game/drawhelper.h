#ifndef DRAWHELPER_H
#define DRAWHELPER_H

#include <GL/glew.h>
#include "materialstate.h"
#include "renderstates.h"
#include "util.h"

#include "lighting.h"

namespace EvilTemple {

struct DrawStrategy {
    virtual void draw(const RenderStates &renderStates, MaterialPassState &state) const = 0;
};

struct BufferSource {
    virtual GLint buffer(const MaterialPassAttributeState &attribute) const = 0;
};

struct EmptyBufferSource : public BufferSource {
    GLint buffer(const MaterialPassAttributeState &attribute) const
    {
        Q_UNUSED(attribute);
        return -1;
    }
};

template<typename DrawStrategy, typename BufferSource = EmptyBufferSource>
class DrawHelper
{
public:
    void draw(const RenderStates &renderStates, MaterialState *material, const DrawStrategy &drawer, const BufferSource &bufferSource) const
    {
        for (int i = 0; i < material->passCount; ++i) {
            MaterialPassState &pass = material->passes[i];

            pass.program->bind();

            // Bind texture samplers
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                pass.textureSamplers[j].bind();
            }

            // Bind uniforms
            for (int j = 0; j < pass.uniforms.size(); ++j) {
                pass.uniforms[j]->bind();
            }

            // Bind attributes
            for (int j = 0; j < pass.attributes.size(); ++j) {
                MaterialPassAttributeState &attribute = pass.attributes[j];

                GLint bufferId = bufferSource.buffer(attribute);

                SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, bufferId));

                // Assign the attribute
                SAFE_GL(glEnableVertexAttribArray(attribute.location));
                SAFE_GL(glVertexAttribPointer(attribute.location,
                                                attribute.binding.components(),
                                                attribute.binding.type(),
                                                attribute.binding.normalized(),
                                                attribute.binding.stride(),
                                                (GLvoid*)attribute.binding.offset()));

            }
            SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0)); // Unbind any previously bound buffers

            // Set render states
            foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                state->enable();
            }

            // Draw the actual model
            drawer.draw(renderStates, pass);

            // Reset render states to default
            foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                state->disable();
            }

            // Unbind textures
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                pass.textureSamplers[j].unbind();
            }

            // Unbind attributes
            for (int j = 0; j < pass.attributes.size(); ++j) {
                MaterialPassAttributeState &attribute = pass.attributes[j];
                SAFE_GL(glDisableVertexAttribArray(attribute.location));
            }

            pass.program->unbind();
        }
    }
};

template<typename DrawStrategy, typename BufferSource = EmptyBufferSource>
class CustomDrawHelper
{
public:
    virtual void draw(const RenderStates &renderStates,
                      MaterialState *material,
                      const DrawStrategy &drawer,
                      const BufferSource &bufferSource) const = 0;
};

struct ModelBufferSource : public BufferSource {
    inline ModelBufferSource(GLint positionBuffer, GLint normalBuffer, GLint texCoordBuffer)
        : mPositionBuffer(positionBuffer), mNormalBuffer(normalBuffer), mTexCoordBuffer(texCoordBuffer)
    {
    }

    inline GLint buffer(const MaterialPassAttributeState &attribute) const
    {
        switch (attribute.bufferType)
        {
        case 0:
            return mPositionBuffer;
        case 1:
            return mNormalBuffer;
        case 2:
            return mTexCoordBuffer;
        default:
            qWarning("Unknown buffer id requested: %d.", attribute.bufferType);
            return 0;
        }
    }

    GLint mPositionBuffer;
    GLint mNormalBuffer;
    GLint mTexCoordBuffer;
};

struct ModelDrawStrategy : public DrawStrategy {
    ModelDrawStrategy(GLint bufferId, int elementCount)
        : mBufferId(bufferId), mElementCount(elementCount)
    {
    }

    inline void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        Q_UNUSED(state);
        Q_UNUSED(renderStates);

        // Render once without diffuse/specular, then render again without ambient
        int typePos = state.program->uniformLocation("lightSourceType");
        if (!renderStates.activeLights().isEmpty() && typePos != -1) {
            SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

            typePos = state.program->uniformLocation("lightSourceType");
            int colorPos = state.program->uniformLocation("lightSourceColor");
            int positionPos = state.program->uniformLocation("lightSourcePosition");
            int directionPos = state.program->uniformLocation("lightSourceDirection");
            int attenuationPos = state.program->uniformLocation("lightSourceAttenuation");

            bool first = true;

            static const int MaxLightsPerPass = 12;

            GLint types[MaxLightsPerPass] = {0, };
            Vector4 color[MaxLightsPerPass];
            Vector4 direction[MaxLightsPerPass];
            Vector4 position[MaxLightsPerPass];
            float attenuation[MaxLightsPerPass];

            // Draw again for every light affecting this mesh
            for (int i = 0; i < renderStates.activeLights().size(); i += MaxLightsPerPass) {
                int lightsThisPass = qMin(MaxLightsPerPass, renderStates.activeLights().size() - i);

                for (int j = 0; j < lightsThisPass; ++j) {
                    const Light *light = renderStates.activeLights().at(i + j);

                    types[j] = light->type();
                    color[j] = light->color();
                    direction[j] = light->direction();
                    position[j] = light->position();
                    attenuation[j] = light->attenuation();
                }

                for (int j = lightsThisPass; j < MaxLightsPerPass; ++j) {
                    color[j] = Vector4(0,0,0,0);
                }

                SAFE_GL(glUniform1iv(typePos, MaxLightsPerPass, types));
                SAFE_GL(glUniform4fv(colorPos, MaxLightsPerPass, (GLfloat*)color);
                SAFE_GL(glUniform4fv(directionPos, MaxLightsPerPass, (GLfloat*)direction)));
                SAFE_GL(glUniform4fv(positionPos, MaxLightsPerPass, (GLfloat*)position));
                SAFE_GL(glUniform1fv(attenuationPos, MaxLightsPerPass, (GLfloat*)attenuation));

                SAFE_GL(glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, 0));

                if (first && i + 1 < renderStates.activeLights().size()) {
                    SAFE_GL(glDepthFunc(GL_LEQUAL));
                    SAFE_GL(glEnable(GL_CULL_FACE));

                    SAFE_GL(glEnable(GL_BLEND));
                    SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
                }
                first = false;
            }

            SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

            SAFE_GL(glDepthFunc(GL_LESS));
        } else {
            SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));
            SAFE_GL(glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, 0));
            SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
    }

    GLint mBufferId;
    int mElementCount;
};

}

#endif // DRAWHELPER_H
