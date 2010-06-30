
#include <GL/glew.h>

#include "lighting_debug.h"
#include "renderstates.h"

namespace EvilTemple {

LightDebugRenderer::LightDebugRenderer(RenderStates &renderStates) : mRenderStates(renderStates)
{
}

bool LightDebugRenderer::loadMaterial()
{
    if (!mMaterialState.createFromFile(":/material/light_material.xml", mRenderStates, FileTextureSource::instance())) {
        qWarning("Unable to load light debugger material: %s.", qPrintable(mMaterialState.error()));
        return false;
    } else {
        return true;
    }
}

void LightDebugRenderer::render(const Light &light)
{

    Vector4 basePoint = light.position();
    basePoint.setW(0);
    basePoint.setY(0);
    mRenderStates.setWorldMatrix(Matrix4::translation(basePoint));

    for (int i = 0; i < mMaterialState.passCount; ++i) {
        MaterialPassState &pass = mMaterialState.passes[i];

        pass.program->bind();

        for (int j = 0; j < pass.textureSamplers.size(); ++j) {
            pass.textureSamplers[i].bind();
        }

        for (int j = 0; j < pass.uniforms.size(); ++j) {
            pass.uniforms[j]->bind();
        }

        for (int j = 0; j < pass.renderStates.size(); ++j) {
            pass.renderStates[j]->enable();
        }

        int attribLocation = pass.program->attributeLocation("vertexPosition");
        int texCoordLocation = pass.program->attributeLocation("vertexTexCoord");
        int typeLocation = pass.program->uniformLocation("type");
        int attenuationLocation = pass.program->uniformLocation("attenuation");
        int centerLocation = pass.program->uniformLocation("center");
        int rangeLocation = pass.program->uniformLocation("range");
        glUniform1i(typeLocation, 1);

        glBegin(GL_QUADS);
        glVertexAttrib4f(attribLocation, -10, 0, -10, 1);
        glVertexAttrib4f(attribLocation, 10, 0, -10, 1);
        glVertexAttrib4f(attribLocation, 10, 0, 10, 1);
        glVertexAttrib4f(attribLocation, -10, 0, 10, 1);
        glEnd();

        glUniform1i(typeLocation, 2);
        glUniform1f(rangeLocation, light.range());
        glUniform1f(attenuationLocation, light.attenuation());
        glUniform4fv(centerLocation, 1, light.position().data());

        float r = light.range();

        glBegin(GL_QUADS);
        glVertexAttrib2f(texCoordLocation, 0, 0);
        glVertexAttrib4f(attribLocation, -r, light.position().y(), -r, 1);
        glVertexAttrib2f(texCoordLocation, 1, 0);
        glVertexAttrib4f(attribLocation, r, light.position().y(), -r, 1);
        glVertexAttrib2f(texCoordLocation, 1, 1);
        glVertexAttrib4f(attribLocation, r, light.position().y(), r, 1);
        glVertexAttrib2f(texCoordLocation, 0, 1);
        glVertexAttrib4f(attribLocation, -r, light.position().y(), r, 1);
        glEnd();

        glUniform1i(typeLocation, 3);

        glBegin(GL_LINES);
        glVertexAttrib4f(attribLocation, 0, 0, 0, 1);
        glVertexAttrib4f(attribLocation, 0, light.position().y(), 0, 1);
        glEnd();

        for (int j = 0; j < pass.renderStates.size(); ++j) {
            pass.renderStates[j]->disable();
        }

        for (int j = 0; j < pass.textureSamplers.size(); ++j) {
            pass.textureSamplers[i].unbind();
        }

        pass.program->unbind();
    }

    mRenderStates.setWorldMatrix(Matrix4::identity());

}

}
