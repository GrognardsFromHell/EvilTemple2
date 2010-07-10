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

            // Draw again for every light affecting this mesh
            foreach (const Light *light, renderStates.activeLights()) {
                SAFE_GL(glUniform1i(typePos, light->type()));
                SAFE_GL(glUniform4fv(colorPos, 1, light->color().data()));
                SAFE_GL(glUniform4fv(directionPos, 1, light->direction().data()));
                SAFE_GL(glUniform4fv(positionPos, 1, light->position().data()));
                SAFE_GL(glUniform1f(attenuationPos, light->attenuation()));

                SAFE_GL(glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, 0));

                if (first) {
                    SAFE_GL(glDepthFunc(GL_LEQUAL));
                    SAFE_GL(glEnable(GL_CULL_FACE));

                    SAFE_GL(glEnable(GL_BLEND));
                    SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
                }
                first = false;
            }

            SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

            SAFE_GL(glDepthFunc(GL_LESS));

            SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
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
