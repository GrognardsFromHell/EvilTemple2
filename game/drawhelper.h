#ifndef DRAWHELPER_H
#define DRAWHELPER_H

#include <GL/glew.h>
#include "materialstate.h"
#include "renderstates.h"
#include "util.h"

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

            pass.program.bind();

            // Bind texture samplers
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                pass.textureSamplers[j].bind();
            }

            // Bind uniforms
            for (int j = 0; j < pass.uniforms.size(); ++j) {
                pass.uniforms[j].bind();
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

            pass.program.unbind();
        }
    }
};

}

#endif // DRAWHELPER_H