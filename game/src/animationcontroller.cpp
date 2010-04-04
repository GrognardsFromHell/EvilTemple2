
#include <QTime>

#include "animationcontroller.h"
#include "skeleton.h"

namespace EvilTemple
{

    class AnimationControllerPrivate
    {
    public:
        const Skeleton *skeleton;
        const Animation &animation;
        QTime lastUpdate; // Time when the last update was performed.
        float distanceCovered; // Distance accumulated between updates
        float rotationCovered; // Rotation accumulated between updates (in degrees)
        float currentFrame; // Which frame in the animation is current
        QScopedPointer<AnimationStream> animationStream; // The key frame stream

        QVector<QMatrix4x4> boneMatrices; // World matrix for every bone

        AnimationControllerPrivate(const Skeleton *skeleton, const Animation &animation);

        void update();

        void updateBoneMatrices();
        void updateBoneMatrix(const Bone &bone, const QMatrix4x4 &worldForward);
    };

    inline AnimationControllerPrivate::AnimationControllerPrivate(const Skeleton *_skeleton,
                                                                  const Animation &_animation)
                                                                      : skeleton(_skeleton),
                                                                      animation(_animation),
                                                                      distanceCovered(0),
                                                                      rotationCovered(0),
                                                                      currentFrame(0),
                                                                      animationStream(_animation.openStream(_skeleton)),
                                                                      boneMatrices(_skeleton->bones().size())
    {
        updateBoneMatrices();
    }

    inline void AnimationControllerPrivate::update()
    {
        /*
            The timer starts when the first frame update is requested
            This avoids a "jump" in the animation when there is a delay between
            construction of this controller and the first frame update.
        */
        if (lastUpdate.isNull()) {
            lastUpdate.start();
            return;
        }

        int elapsed = lastUpdate.restart(); // Miliseconds
        currentFrame += animation.frameRate() * (elapsed / 1000.0f);

        if (currentFrame >= animation.frames())
            animationStream->rewind();

        while (currentFrame >= animation.frames())
            currentFrame -= animation.frames();

        animationStream->seek((int)currentFrame);
        updateBoneMatrices();
    }

    void AnimationControllerPrivate::updateBoneMatrices()
    {
        QMatrix4x4 identity;

        // Update each bone
        foreach (const Bone &bone, skeleton->bones()) {
            if (bone.parentId == -1)
                updateBoneMatrix(bone, identity);
        }
    }

    inline float getEndWeight(int start, int end, int frame) {
        if (start == end) {
            return 0.0f;
        }
        
        if (frame >= end) {
            return 1.0f;
        }
        
        if (frame <= start) {
            return 0.0f;
        }
        
        float between = frame - start;
        float factor = between / (end - start);
        return factor;
    }

    inline QVector3D interpolateLinear(const QVector3D &start, const QVector3D &end, float endWeight) {
        return (1 - endWeight) * start + endWeight * end;
    }

    void AnimationControllerPrivate::updateBoneMatrix(const Bone &bone, const QMatrix4x4 &worldForward)
    {        
        const AnimationBoneState *boneState = animationStream->getBoneState(bone.id);

        QMatrix4x4 fullWorldMatrix(worldForward);

        if (boneState) {
            Q_ASSERT(boneState->boneId == bone.id);

            QMatrix4x4 relativeWorld;

            // Interpolate
            float weight = getEndWeight(boneState->translationFrame, boneState->nextTranslationFrame, currentFrame);
            relativeWorld.translate(interpolateLinear(boneState->translation, boneState->nextTranslation, weight));

            weight = getEndWeight(boneState->rotationFrame, boneState->nextRotationFrame, currentFrame);
            QQuaternion rotation(QQuaternion::slerp(boneState->rotation, boneState->nextRotation, weight));
            relativeWorld.rotate(rotation);

            weight = getEndWeight(boneState->scaleFrame, boneState->nextScaleFrame, currentFrame);
            relativeWorld.scale(interpolateLinear(boneState->scale, boneState->nextScale, weight));

            fullWorldMatrix = fullWorldMatrix * relativeWorld;
        } else {
            // Use the default world matrix if the stream
            // doesn't animate the current bone
            fullWorldMatrix = fullWorldMatrix * bone.relativeWorld;
        }

        boneMatrices[bone.id] = fullWorldMatrix * bone.fullWorldInverse;

        foreach (qint16 childId, bone.childrenIds) {
            updateBoneMatrix(skeleton->bones().at(childId), fullWorldMatrix);
        }
    }

    AnimationController::AnimationController(const Skeleton *skeleton, const Animation &animation, QObject *parent)
        : QObject(parent), d_ptr(new AnimationControllerPrivate(skeleton, animation))
    {
    }

    AnimationController::~AnimationController()
    {
    }

    void AnimationController::update()
    {
        d_ptr->update();
    }

    void AnimationController::moved(float distance)
    {
        d_ptr->distanceCovered += distance;
    }

    void AnimationController::rotated(float degrees)
    {
        d_ptr->rotationCovered += degrees;
    }

    const QVector<QMatrix4x4> &AnimationController::boneMatrices() const
    {
        return d_ptr->boneMatrices;
    }

}
