
#include <GL/glew.h>

#include "boxrenderable.h"
#include "drawhelper.h"
#include "scenenode.h"
#include "materialcache.h"

namespace EvilTemple {

struct BoxDrawStrategy {
    BoxDrawStrategy(const Box3d &box) : mBox(box) {}

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        float minX = mBox.minimum().x();
        float minY = mBox.minimum().y();
        float minZ = mBox.minimum().z();

        float maxX = mBox.maximum().x();
        float maxY = mBox.maximum().y();
        float maxZ = mBox.maximum().z();

        Vector4 vertices[8] = {
            // Bottom
            Vector4(minX, minY, minZ, 1),
            Vector4(maxX, minY, minZ, 1),
            Vector4(maxX, minY, maxZ, 1),
            Vector4(minX, minY, maxZ, 1),
            // Top
            Vector4(minX, maxY, minZ, 1),
            Vector4(maxX, maxY, minZ, 1),
            Vector4(maxX, maxY, maxZ, 1),
            Vector4(minX, maxY, maxZ, 1),
        };

        // One normal per side
        Vector4 normals[6] = {
            Vector4(1, 0, 0, 0), // Right
            Vector4(0, 1, 0, 0), // Top
            Vector4(0, 0, 1, 0), // Front
            Vector4(-1, 0, 0, 0), // Left
            Vector4(0, -1, 0, 0), // Bottom
            Vector4(0, 0, -1, 0), // Back
        };

        int indices[] = {
            // Bottom Face
            0, 1, 1, 2, 2, 3, 3, 0,
            // Top Face
            4, 5, 5, 6, 6, 7, 7, 4,
            // Connection from bottom to top
            0, 4, 1, 5, 2, 6, 3, 7
        };
        int indicesCount = 24;

        glBegin(GL_LINES);
        for (int i = 0; i < indicesCount; ++i) {
            
            glVertex4fv(vertices[indices[i]].data());
        }
        glEnd();
    }

    const Box3d mBox;
};

BoxRenderable::BoxRenderable()
{
}

void BoxRenderable::render(RenderStates &renderStates)
{
    if (!mMaterial) {
        mMaterial = loadMaterial(":/material/box_material.xml", renderStates);
    }

    Box3d parentBounds = mParentNode->boundingBox().transformAffine(renderStates.worldMatrix());
    renderStates.setWorldMatrix(Matrix4::identity());

    BoxDrawStrategy drawer(parentBounds);
    DrawHelper<BoxDrawStrategy> boxDrawStrategy;
    boxDrawStrategy.draw(renderStates, mMaterial.data(), drawer, EmptyBufferSource());

}

const Box3d &BoxRenderable::boundingBox()
{
    static Box3d nullBox;
    return nullBox;
}

}
