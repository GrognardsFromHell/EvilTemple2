
#include <GL/glew.h>

#include "lighting_debug.h"
#include "renderstates.h"

namespace EvilTemple {

LightDebugRenderer::LightDebugRenderer(RenderStates &renderStates) : mRenderStates(renderStates)
{
}

bool LightDebugRenderer::loadMaterial()
{
    if (!mMaterialState.createFromFile(":/material/light_material.xml", mRenderStates, NULL)) {
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

        pass.program.bind();

        for (int j = 0; j < pass.uniforms.size(); ++j) {
            pass.uniforms[j].bind();
        }

        for (int j = 0; j < pass.renderStates.size(); ++j) {
            pass.renderStates[j]->enable();
        }

        int attribLocation = pass.program.attributeLocation("vertexPosition");
        int typeLocation = pass.program.uniformLocation("type");
        glUniform1i(typeLocation, 1);

        glBegin(GL_QUADS);
        glVertexAttrib4f(attribLocation, -10, 0, -10, 1);
        glVertexAttrib4f(attribLocation, 10, 0, -10, 1);
        glVertexAttrib4f(attribLocation, 10, 0, 10, 1);
        glVertexAttrib4f(attribLocation, -10, 0, 10, 1);
        glEnd();

        glUniform1i(typeLocation, 2);

        glBegin(GL_QUADS);
        glVertexAttrib4f(attribLocation, -10, light.position().y(), -10, 1);
        glVertexAttrib4f(attribLocation, 10, light.position().y(), -10, 1);
        glVertexAttrib4f(attribLocation, 10, light.position().y(), 10, 1);
        glVertexAttrib4f(attribLocation, -10, light.position().y(), 10, 1);
        glEnd();

        glUniform1i(typeLocation, 3);

        glBegin(GL_LINES);
        glVertexAttrib4f(attribLocation, 0, 0, 0, 1);
        glVertexAttrib4f(attribLocation, 0, light.position().y(), 0, 1);
        glEnd();

        for (int j = 0; j < pass.renderStates.size(); ++j) {
            pass.renderStates[j]->disable();
        }

        pass.program.unbind();
    }

    mRenderStates.setWorldMatrix(Matrix4::identity());

}

}
