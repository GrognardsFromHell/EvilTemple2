
#include "animation.h"

namespace EvilTemple {

QDataStream &operator >>(QDataStream &stream, Animation &animation)
{
    uint driveType;
    uint animationBonesCount;
    stream >> animation.mName >> animation.mFrames >> animation.mFrameRate >> animation.mDps
        >> driveType >> animation.mLoopable >> animation.mEvents >> animationBonesCount;

    delete [] animation.mAnimationBones;
    animation.mAnimationBonesMap.clear();
    animation.mAnimationBonesMap.reserve(animationBonesCount);
    animation.mAnimationBones = new AnimationBone[animationBonesCount];

    for (uint i = 0; i < animationBonesCount; ++i) {
        uint boneId;
        stream >> boneId >> animation.mAnimationBones[i];
        animation.mAnimationBonesMap.insert(boneId, animation.mAnimationBones + i);
    }

    Q_ASSERT(animation.mFrameRate >= 0);
    Q_ASSERT(driveType == Animation::Time || driveType == Animation::Rotation || driveType == Animation::Distance);

    animation.mDriveType = static_cast<Animation::DriveType>(driveType);
    
    return stream;
}

QDataStream &operator >>(QDataStream &stream, AnimationEvent &event)
{
    QByteArray action;
    uint type;
    stream >> event.mFrame >> type >> action;

    Q_ASSERT(type == AnimationEvent::Action || type == AnimationEvent::Script);

    event.mType = static_cast<AnimationEvent::Type>(type);
    event.mContent = QString::fromUtf8(action.constData(), action.size());

    return stream;
}

QDataStream &operator >>(QDataStream &stream, AnimationBone &bone)
{
    stream >> bone.rotationStream >> bone.scaleStream >> bone.translationStream;
    return stream;
}

}
