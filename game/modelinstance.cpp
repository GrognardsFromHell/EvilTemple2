
#include <QVector>

#include "texture.h"
#include "modelinstance.h"
#include "util.h"
#include "drawhelper.h"
#include "particlesystem.h"
#include "scenenode.h"
#include "profiler.h"
#include "lighting.h"

namespace EvilTemple {

    ModelInstance::ModelInstance()
        :
        mCurrentAnimation(NULL), mPartialFrameTime(0), mCurrentFrame(0),
        mTransformedPositions(NULL), mTransformedNormals(NULL),
        mCurrentFrameChanged(true), mIdling(true), mLooping(false),
        mDrawsBehindWalls(false), mTimeSinceLastRender(std::numeric_limits<float>::infinity()),
        mSkeleton(NULL)
    {
    }

    ModelInstance::~ModelInstance()
    {
        qDeleteAll(mTransformedNormalsAddMeshes);
        qDeleteAll(mTransformedPositionsAddMeshes);
        qDeleteAll(mNormalBufferAddMeshes);
        qDeleteAll(mPositionBufferAddMeshes);
        delete [] mTransformedPositions;
        delete [] mTransformedNormals;
    }

    void ModelInstance::setModel(const SharedModel &model)
    {
        delete [] mTransformedPositions;
        mTransformedPositions = 0;
        delete [] mTransformedNormals;
        mTransformedNormals = 0;

        delete mSkeleton;
        mSkeleton = NULL;

        mModel = model;
        mReplacementMaterials.clear();
        mReplacementMaterials.resize(mModel->placeholders().size());

        // Create animation state if necessary
        if (mModel->animations().isEmpty()) {
            return;
        }

        mTransformedPositions = new Vector4[mModel->vertices];
        mTransformedNormals = new Vector4[mModel->vertices];
        mSkeleton = new Skeleton(*mModel->skeleton());

        mCurrentAnimation = model->animation("item_idle");

        if (!mCurrentAnimation) {
            mCurrentAnimation = model->animation("unarmed_unarmed_idle");
        }

        if (mCurrentAnimation) {
            if (mCurrentAnimation->animationBones().isEmpty()) {
                // qWarning("The animation %s on %s is empty.", qPrintable(mCurrentAnimation->name()), qPrintable(mSkeleton->name()));
                mCurrentAnimation = NULL;
                return;
            }

            mIdleAnimation = mCurrentAnimation->name();

            mPositionBuffer.upload(model->positions, sizeof(Vector4) * model->vertices);
            mNormalBuffer.upload(model->normals, sizeof(Vector4) * model->vertices);

            // Check for events on frame 0 and trigger them now
            foreach (const AnimationEvent &event, mCurrentAnimation->events()) {
                if (event.frame() == 0) {
                    emit animationEvent(event.type(), event.content());
                }
            }

            // This is for idle animations -> advance by a random number of frames, since
            // not all models should "idle in sync"
            elapseTime(rand() / (float)RAND_MAX);
        }
    }

    /**
      Creates a mapping that assigns a bone to each bone-id used in a binding pose, by comparing the names of bones
      from the binding pose to the bones in the skeleton.
    */
    static QVector<uint> createBoneMapping(const BindingPose *pose, const Skeleton *skeleton)
    {
        QVector<uint> result(pose->boneCount());

        for (uint i = 0; i < pose->boneCount(); ++i) {
            const Bone *bone = skeleton->bone(pose->boneName(i));

            if (bone) {
                result[i] = bone->boneId();
            } else {
                result[i] = -1;
            }
        }

        return result;
    }

    void ModelInstance::addMesh(const SharedModel &model)
    {
        mAddMeshes.append(model);

        mTransformedNormalsAddMeshes.append(new Vector4[model->vertices]);
        mTransformedPositionsAddMeshes.append(new Vector4[model->vertices]);

        VertexBufferObject *buffer = new VertexBufferObject;
        buffer->upload(model->positions, sizeof(Vector4) * model->vertices);
        mPositionBufferAddMeshes.append(buffer);

        buffer = new VertexBufferObject;
        buffer->upload(model->normals, sizeof(Vector4) * model->vertices);
        mNormalBufferAddMeshes.append(buffer);

        /*
            Create a bone mapping. This crappy hack is necessary, since the ordering of bones in addmeshes
            generally can differ from the order in the current skeleton. Thus we create a mapping from the
            bone ids in the addmesh to bone ids in the current mesh, so the vertices are bound to the
            correct bones.

            It might be possible to actually fix this problem by enforcing a unified order on bones (i.e. by name
            ascending), although this *could* add additional problems for parent ids. Otherwise, the bones
            of an addmesh could be reordered to the bones of the mesh using it, but the relation between meshes
            and addmeshes is not known a-priori, making this difficult.

            UPDATE:
            The bone mapping is done by-name. Now the addmeshes don't actually contain a skeleton, which wouldn't 
            be used. Instead, they only contain a binding pose, which is mapped to the current skeleton here.
            */
        if (model->bindingPose()) {
            const Skeleton *curSkeleton = skeleton();

            mAddMeshBoneMapping.append(createBoneMapping(model->bindingPose(), curSkeleton));
        } else {
            qWarning("An addmesh was added, although it doesn't have a binding pose.");
            mAddMeshBoneMapping.append(QVector<uint>());
        }

        Q_ASSERT(mTransformedPositionsAddMeshes.size() == mAddMeshes.size());
        Q_ASSERT(mTransformedNormalsAddMeshes.size() == mAddMeshes.size());
        Q_ASSERT(mPositionBufferAddMeshes.size() == mAddMeshes.size());
        Q_ASSERT(mNormalBufferAddMeshes.size() == mAddMeshes.size());
        Q_ASSERT(mAddMeshBoneMapping.size() == mAddMeshes.size());
    }

    Matrix4 ModelInstance::getBoneSpace(uint boneId)
    {
        if (!hasSkeleton()) {
            return Matrix4::identity();
        }

        if (mCurrentFrameChanged) {
            updateBones();
            mCurrentFrameChanged = false;
        }

        const Bone *bone = skeleton()->bone(boneId);

        if (!bone) {
            qWarning("Unknown bone id: %s.", boneId);
            return Matrix4::identity();
        }

        return bone->fullWorld();
    }

    void ModelInstance::drawNormals() const
    {
        if (!mModel)
            return;

        if (!mCurrentAnimation) {
            mModel->drawNormals();
            return;
        }

        SAFE_GL(glLineWidth(1.5));
        SAFE_GL(glEnable(GL_LINE_SMOOTH));
        SAFE_GL(glColor3f(0, 0, 1));
        SAFE_GL(glDisable(GL_LIGHTING));
        glBegin(GL_LINES);

        for (int i = 0; i < mModel->vertices; i += 2) {
            const Vector4 &vertex = mTransformedPositions[i];
            const Vector4 &normal = mTransformedNormals[i];
            glVertex3fv(vertex.data());
            glVertex3fv((vertex + 15 * normal).data());
        }
        glEnd();

        glPointSize(2);
        glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        for (int i = 0; i < mModel->vertices; i += 2) {
            const Vector4 &vertex = mTransformedPositions[i];
            glVertex3fv(vertex.data());
        }
        SAFE_GL(glEnd());
    }

    void ModelInstance::animateVertices(const SharedModel &model,
        Vector4 *transformedPositions, Vector4 *transformedNormals,
        VertexBufferObject *positionBuffer, VertexBufferObject *normalBuffer,
        const QVector<uint> &boneMapping)
    {
        const Skeleton::Bones &bones = mSkeleton->bones();
        const BindingPose *bindingPose = model->bindingPose();
        
        for (int i = 0; i < model->vertices; ++i) {
            const BoneAttachment &attachment = bindingPose->attachment(i);

            if (attachment.count() == 0) {
                transformedPositions[i] = model->positions[i];
                transformedNormals[i] = model->normals[i];
                continue;
            }

            float weight = attachment.weights()[0];
            uint boneId = attachment.bones()[0];                        
            Q_ASSERT(boneId >= 0 && boneId < boneMapping.size());

            const Matrix4 &fullWorldInverse = bindingPose->fullWorldInverse(boneId);

            boneId = boneMapping.at(boneId);
            Q_ASSERT(boneId >= 0 && boneId < bones.size());

            const Matrix4 &firstTransform = bones[boneId]->fullWorld() * fullWorldInverse;

            __m128 factor = _mm_set_ps(weight, - weight, weight, weight);
            
            transformedPositions[i] = _mm_mul_ps(factor, firstTransform.mapPosition(model->positions[i]));
            transformedNormals[i] = _mm_mul_ps(factor, firstTransform.mapNormal(model->normals[i]));

            for (int k = 1; k < qMin(4, attachment.count()); ++k) {
                weight = attachment.weights()[k];
                boneId = attachment.bones()[k];

                Q_ASSERT(boneId >= 0 && boneId < boneMapping.size());

                const Matrix4 &fullWorldInverse = bindingPose->fullWorldInverse(boneId);

                boneId = boneMapping.at(boneId);
                Q_ASSERT(boneId >= 0 && boneId < bones.size());                               

                const Matrix4 &fullTransform = bones[boneId]->fullWorld() * fullWorldInverse;

                // This flips the z coordinate, since the models are geared towards DirectX
                factor = _mm_set_ps(weight, - weight, weight, weight);

                transformedPositions[i] += _mm_mul_ps(factor, fullTransform * model->positions[i]);
                transformedNormals[i] += _mm_mul_ps(factor, fullTransform.mapNormal(model->normals[i]));
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer->bufferId());
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * model->vertices, transformedPositions, GL_STATIC_DRAW);

        // This is extremely costly. Accurately recomputing the normals for each vertex
        if (model->needsNormalsRecalculated()) {
            for (int i = 0; i < model->vertices; ++i) {
                Vector4 averagedNormal(0, 0, 0, 0);
                Vector4 thisPos = transformedPositions[i];

                // Find all faces that use this vertex
                for (int j = 0; j < model->faces; ++j) {
                    const FaceGroup &faceGroup = model->faceGroups[j];

                    for (int k = 0; k < faceGroup.indices.size(); k += 3) {
                        int index1 = faceGroup.indices[k+2];
                        int index2 = faceGroup.indices[k+1];
                        int index3 = faceGroup.indices[k];

                        if (index1 == i) {
                            averagedNormal += (transformedPositions[index2] - thisPos).cross(transformedPositions[index3] - thisPos);
                        } else if (index2 == i) {
                            averagedNormal += (transformedPositions[index3] - thisPos).cross(transformedPositions[index1] - thisPos);
                        } else if (index3 == i) {
                            averagedNormal += (transformedPositions[index1] - thisPos).cross(transformedPositions[index2] - thisPos);
                        }
                    }
                }

                averagedNormal.setW(0);
                transformedNormals[i] = averagedNormal.normalized();
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer->bufferId());
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * model->vertices, transformedNormals, GL_STATIC_DRAW);
    }

    void ModelInstance::updateBones()
    {
        if (!mCurrentAnimation)
            return;

        const Skeleton::Bones &bones = mSkeleton->bones();

        // Create a bone state map for all bones
        const Animation::BoneMap &animationBones = mCurrentAnimation->animationBones();

        for (int i = 0; i < bones.size(); ++i) {
            Bone *bone = bones[i];

            Matrix4 relativeWorld;

            if (animationBones.contains(i)) {
                const AnimationBone *animationBone = animationBones[i];

                relativeWorld = animationBone->getTransform(mCurrentFrame, mCurrentAnimation->frames());
            } else {
                relativeWorld = bone->relativeWorld();
            }

            // Use relative world and fullWorld of parent to build this bone's full world
            Bone *parent = bone->parent();

            if (parent) {
                bone->setFullWorld(parent->fullWorld() * relativeWorld);
            } else {
                bone->setFullWorld(relativeWorld);
            }
        }

        QVector<uint> mapping = createBoneMapping(mModel->bindingPose(), mModel->skeleton());

        animateVertices(mModel, mTransformedPositions, mTransformedNormals, &mPositionBuffer, &mNormalBuffer, mapping);

        for (int i = 0; i < mAddMeshes.size(); ++i) {
            animateVertices(mAddMeshes[i], mTransformedPositionsAddMeshes[i], mTransformedNormalsAddMeshes[i], mPositionBufferAddMeshes[i], mNormalBufferAddMeshes[i],
                            mAddMeshBoneMapping[i]);
        }
    }

    struct ModelInstanceDrawStrategy : public ModelDrawStrategy {
        ModelInstanceDrawStrategy(GLint bufferId, int elementCount, bool drawShadows)
            : ModelDrawStrategy(bufferId, elementCount), mDrawShadows(drawShadows)
        {
        }

        inline void draw(const RenderStates &renderStates, MaterialPassState &state) const
        {
            if (state.id > 0 && !mDrawShadows)
                return;

            ModelDrawStrategy::draw(renderStates, state);
        }

        bool mDrawShadows;
    };

    void ModelInstance::render(RenderStates &renderStates, MaterialState *overrideMaterial)
    {
        ProfileScope<Profiler::ModelInstanceRender> profiler;

        mTimeSinceLastRender = 0;

        const Model *model = mModel.data();

        if (!model)
            return;

        if (mCurrentAnimation && mCurrentFrameChanged) {
            updateBones();
            mCurrentFrameChanged = false;
        }

        DrawHelper<ModelDrawStrategy, ModelBufferSource> drawHelper;
        ModelBufferSource bufferSource(mCurrentAnimation ? mPositionBuffer.bufferId() : model->positionBuffer.bufferId(),
                                       mCurrentAnimation ? mNormalBuffer.bufferId() : model->normalBuffer.bufferId(),
                                       model->texcoordBuffer.bufferId());

        for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
            const FaceGroup &faceGroup = model->faceGroups[faceGroupId];

            MaterialState *material = faceGroup.material;

            // This needs special handling (material replacement)
            if (faceGroup.placeholderId >= 0) {
                material = mReplacementMaterials[faceGroup.placeholderId].data();
            }

            if (overrideMaterial)
                material = overrideMaterial;

            if (material) {
                ModelDrawStrategy drawStrategy(faceGroup.buffer.bufferId(), faceGroup.indices.size());
                drawHelper.draw(renderStates, material, drawStrategy, bufferSource);
            }
        }

        // Render all addmeshes
        for (int i = 0; i < mAddMeshes.size(); ++i) {
            model = mAddMeshes[i].data();

            ModelBufferSource bufferSource(mCurrentAnimation ? mPositionBufferAddMeshes[i]->bufferId() : model->positionBuffer.bufferId(),
                mCurrentAnimation ? mNormalBufferAddMeshes[i]->bufferId() : model->normalBuffer.bufferId(),
                model->texcoordBuffer.bufferId());

            for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
                const FaceGroup &faceGroup = model->faceGroups[faceGroupId];

                MaterialState *material = faceGroup.material;

                if (overrideMaterial)
                    material = overrideMaterial;

                if (material) {
                    ModelDrawStrategy drawStrategy(faceGroup.buffer.bufferId(), faceGroup.indices.size());
                    drawHelper.draw(renderStates, material, drawStrategy, bufferSource);
                }
            }
        }
    }

    void ModelInstance::draw(RenderStates &renderStates, const CustomDrawHelper<ModelDrawStrategy, ModelBufferSource> &drawHelper)
    {
        ProfileScope<Profiler::ModelInstanceRender> profiler;

        const Model *model = mModel.data();

        if (!model)
            return;

        if (mCurrentFrameChanged) {
            updateBones();
            mCurrentFrameChanged = false;
        }

        ModelBufferSource bufferSource(mCurrentAnimation ? mPositionBuffer.bufferId() : model->positionBuffer.bufferId(),
                                       mCurrentAnimation ? mNormalBuffer.bufferId() : model->normalBuffer.bufferId(),
                                       model->texcoordBuffer.bufferId());

        for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
            const FaceGroup &faceGroup = model->faceGroups[faceGroupId];

            MaterialState *material = faceGroup.material;

            // This needs special handling (material replacement)
            if (faceGroup.placeholderId >= 0) {
                material = mReplacementMaterials[faceGroup.placeholderId].data();
            }

            if (material) {
                ModelDrawStrategy drawStrategy(faceGroup.buffer.bufferId(), faceGroup.indices.size());
                drawHelper.draw(renderStates, material, drawStrategy, bufferSource);
            }
        }

        // Render all addmeshes
        for (int i = 0; i < mAddMeshes.size(); ++i) {
            model = mAddMeshes[i].data();

            ModelBufferSource bufferSource(mCurrentAnimation ? mPositionBufferAddMeshes[i]->bufferId() : model->positionBuffer.bufferId(),
                mCurrentAnimation ? mNormalBufferAddMeshes[i]->bufferId() : model->normalBuffer.bufferId(),
                model->texcoordBuffer.bufferId());

            for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
                const FaceGroup &faceGroup = model->faceGroups[faceGroupId];

                MaterialState *material = faceGroup.material;

                if (material) {
                    ModelDrawStrategy drawStrategy(faceGroup.buffer.bufferId(), faceGroup.indices.size());
                    drawHelper.draw(renderStates, material, drawStrategy, bufferSource);
                }
            }
        }
    }

    void ModelInstance::elapseTime(float elapsedSeconds)
    {
        mTimeSinceLastRender += elapsedSeconds;

        if (mTimeSinceLastRender >= 5) {
            return; // Stop the animations as well
        }

        ProfileScope<Profiler::ModelInstanceElapseTime> profile;

        if (!mModel || !mCurrentAnimation || mCurrentAnimation->driveType() != Animation::Time)
            return;

        mPartialFrameTime += elapsedSeconds;

        float timePerFrame = 1 / mCurrentAnimation->frameRate();

        if (mPartialFrameTime < timePerFrame)
            return;

        while (mPartialFrameTime >= timePerFrame) {
            if (!advanceFrame())
                return;

            mPartialFrameTime -= timePerFrame;
        }
    }

    void ModelInstance::elapseDistance(float distance)
    {
        if (!mModel || !mCurrentAnimation || mCurrentAnimation->driveType() != Animation::Distance)
            return;

        mPartialFrameTime += distance;

        float distancePerFrame = 1 / mCurrentAnimation->frameRate();

        if (mPartialFrameTime < distancePerFrame)
            return;

        while (mPartialFrameTime >= distancePerFrame) {
            if (!advanceFrame())
                return;

            mPartialFrameTime -= distancePerFrame;
        }
    }

    void ModelInstance::elapseRotation(float rotation)
    {
        if (!mModel || !mCurrentAnimation || mCurrentAnimation->driveType() != Animation::Rotation)
            return;

        mPartialFrameTime += rotation;

        float rotationPerFrame = 1 / mCurrentAnimation->frameRate();

        if (mPartialFrameTime < rotationPerFrame)
            return;

        while (mPartialFrameTime >= rotationPerFrame) {
            if (!advanceFrame())
                return;

            mPartialFrameTime -= rotationPerFrame;
        }
    }

    bool ModelInstance::advanceFrame()
    {
        Q_ASSERT(mCurrentAnimation != NULL);

        if (mCurrentAnimation->frames() <= 1) {
            // qWarning("Animation has only one frame.");
            return true;
        }

        mCurrentFrame++;

        const QVector<AnimationEvent> &events = mCurrentAnimation->events();

        for (size_t i = 0; i < events.size(); ++i) {
            if (events[i].frame() == mCurrentFrame) {
                emit animationEvent(events[i].type(), events[i].content());
            }
        }

        if (mCurrentFrame >= mCurrentAnimation->frames()) {
            // Decide whether it's time to loop or end the animation
            if (!mIdling && !mLooping) {
                emit animationFinished(mCurrentAnimation->name(), false);
                playIdleAnimation();
                return false;
            }

            mCurrentFrame = 0;

            for (size_t i = 0; i < events.size(); ++i) {
                if (events[i].frame() == mCurrentFrame) {
                    emit animationEvent(events[i].type(), events[i].content());
                }
            }
        }
        mCurrentFrameChanged = true;
        return true;
    }

    const Box3d &ModelInstance::boundingBox()
    {
        static Box3d emptyBox;
        if (mModel)
            return mModel->boundingBox();
        else
            return emptyBox;
    }

    const Matrix4 &ModelInstance::worldTransform() const
    {
        Q_ASSERT(mParentNode);

        return mParentNode->fullTransform();
    }

    /**
      Intersects the ray with a triangle.

      This algorithm is equivalent to the algorithm presented in Realtime Rendering p.750
      as RayTriIntersect.

      @param uOut If not null, this pointer receives the barycentric weight of
                    p1 for the point of intersection. But only if there is an intersection.
      @param vOut If not null, this pointer receives the barycentric weight of
                    p2 for the point of intersection. But only if there is an intersection.

      @returns True if the ray shoots through the triangle.
      */
    inline bool intersectRay(const Ray3d &ray,
                           const Vector4 &p0,
                           const Vector4 &p1,
                           const Vector4 &p2,
                           float &distance,
                           float *uOut = 0,
                           float *vOut = 0) {

        Vector4 e1 = p1 - p0;
        Vector4 e2 = p2 - p0;

        Vector4 q = ray.direction().cross(e2);
        float determinant = e1.dot(q);

        /**
          If the determinant is close to zero, the ray lies in the plane of the triangle and thus
          is very unlikely to intersect it.
          */
        if (qFuzzyIsNull(determinant))
            return false;

        float invertedDeterminant = 1 / determinant;

        // Distance from vertex 0 to ray origin
        Vector4 s = ray.origin() - p0;

        // Calculate the first barycentric coordinate
        float u = invertedDeterminant * s.dot(q);

        if (u < 0)
            return false; // Definetly outside the triangle

        Vector4 r = s.cross(e1);

        // Calcaulate the second barycentric coordinate
        float v = invertedDeterminant * ray.direction().dot(r);

        if (v < 0 || u + v > 1)
            return false; // Definetly outside the triangle

        // Store u + v for further use
        if (uOut)
            *uOut = u;
        if (vOut)
            *vOut = v;

        // Calculate the exact point of intersection and then the distance to the ray's origin
        Vector4 point = (1 - u - v) * p0 + u * p1 + v * p2;
        distance = (ray.origin() - point).length();
        return true;
    }

    /*
        Intersects the given ray with this models geometry
     */
    IntersectionResult ModelInstance::intersect(const Ray3d &ray) const
    {
        IntersectionResult result;
        result.intersects = false;
        result.distance = std::numeric_limits<float>::infinity();

        if (!mModel) {
            result.intersects = false;
            return result;
        }

        Vector4 *positions = mCurrentAnimation ? mTransformedPositions : mModel->positions;

        // Do it per-face
        for (int i = 0; i < mModel->faces; ++i) {
            const FaceGroup &group = mModel->faceGroups[i];

            const QVector<ushort> &indices = group.indices;

            for (int j = 0; j < indices.size(); j += 3) {
                const Vector4 &va = positions[indices[j]];
                const Vector4 &vb = positions[indices[j+1]];
                const Vector4 &vc = positions[indices[j+2]];

                float distance;
                if (intersectRay(ray, va, vb, vc, distance) && distance < result.distance) {
                    result.distance = distance;
                    result.intersects = true;
                }
            }
        }

        return result;
    }

    bool ModelInstance::overrideMaterial(const QString &name, const SharedMaterialState &state)
    {
        if (!mModel)
            return false;

        int placeholderId = mModel->placeholders().indexOf(name);

        if (placeholderId == -1)
            return false;

        mReplacementMaterials[placeholderId] = state;

        return true;
    }

    bool ModelInstance::clearOverrideMaterial(const QString &name)
    {
        if (!mModel)
            return false;

        int placeholderId = mModel->placeholders().indexOf(name);

        if (placeholderId == -1)
            return false;

        mReplacementMaterials[placeholderId].clear();

        return true;
    }

    void ModelInstance::clearOverrideMaterials()
    {
        for (int i = 0; i < mReplacementMaterials.size(); ++i)
            mReplacementMaterials[i].clear();
    }

    void ModelInstance::setIdleAnimation(const QString &idleAnimation)
    {
        mIdleAnimation = idleAnimation;
    }

    const QString &ModelInstance::idleAnimation() const
    {
        return mIdleAnimation;
    }

    bool ModelInstance::playAnimation(const QString &name, bool loop)
    {
        if (!mModel)
            return false;

        const Animation *animation = mModel->animation(name);

        if (!animation)
            return false;

        mLooping = loop && animation->isLoopable();

        if (!mIdling && mCurrentAnimation) {
            emit animationFinished(mCurrentAnimation->name(), true);
        }

        // Start playing the animation
        mCurrentAnimation = animation;
        mCurrentFrame = 0;
        mCurrentFrameChanged = true;
        mPartialFrameTime = 0;
        mIdling = false;

        if (mCurrentAnimation->animationBones().isEmpty()) {
            emit animationFinished(mCurrentAnimation->name(), true);
            mCurrentAnimation = NULL;
            //qWarning("The animation %s on %s is empty.", qPrintable(name), qPrintable(mSkeleton->name()));
        }

        return true;
    }

    void ModelInstance::stopAnimation()
    {
        if (!mIdling)
            playIdleAnimation();
    }

    void ModelInstance::playIdleAnimation()
    {
        mIdling = true;
        if (mIdleAnimation.isEmpty())
            mCurrentAnimation = NULL;
        else
            mCurrentAnimation = mModel->animation(mIdleAnimation);
        mCurrentFrame = 0;
        mCurrentFrameChanged = true;
        mPartialFrameTime = 0;
    }

}
