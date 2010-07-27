
#include "bindingpose.h"
#include "util.h"

namespace EvilTemple {

    QDataStream &operator >>(QDataStream &stream, BoneAttachment &attachment)
    {
        stream >> attachment.mBoneCount;

        for (int i = 0; i < BoneAttachment::MaxCount; ++i)
            stream >> attachment.mBones[i];

        for (int i = 0; i < BoneAttachment::MaxCount; ++i)
            stream >> attachment.mWeights[i];

        return stream;
    }

    QDataStream &operator >>(QDataStream &stream, BindingPose &pose)
    {
        uint count;

        stream >> count;

        pose.mFullWorldInverseMatrices.resize(count);
        pose.mBoneNames.resize(count);

        for (uint i = 0; i < count; ++i) {
            stream >> pose.mBoneNames[i] >> pose.mFullWorldInverseMatrices[i];
        }

        stream >> count;

        pose.mAttachments.resize(count);
        
        for (uint i = 0; i < count; ++i) {
            stream >> pose.mAttachments[i];
        }

        return stream;
    }

}
