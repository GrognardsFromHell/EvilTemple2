
#include "skeleton.h"
#include "util.h"

namespace EvilTemple {

QDataStream &operator >>(QDataStream &stream, Skeleton &skeleton)
{
    uint bonesCount;
    stream >> bonesCount;

    skeleton.mBones.resize(bonesCount);

    Matrix4 fullWorldInverse;
    Matrix4 relativeWorld;

    for (int j = 0; j < bonesCount; ++j) {
        Bone &bone = skeleton.mBones[j];
        QByteArray boneName;
        int parentId;

        stream >> boneName >> parentId >> fullWorldInverse >> relativeWorld;

        Q_ASSERT(parentId >= -1 && parentId < skeleton.mBones.size());

        bone.setBoneId(j);
        bone.setFullWorldInverse(fullWorldInverse);
        bone.setRelativeWorld(relativeWorld);
        bone.setName(boneName);

        if (parentId == -1)
            continue;

        bone.setParent(skeleton.mBones.constData() + parentId);

        if (skeleton.mBoneMap.contains(boneName)) {
            qWarning("Duplicate bone %s in skeleton %s", boneName.constData(), qPrintable(skeleton.mName));
        }

        // Insert the bone into the name->bone map
        skeleton.mBoneMap.insert(boneName, &bone);
    }

    return stream;
}

}
