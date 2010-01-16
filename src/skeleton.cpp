
#include "skeleton.h"

#include <QDataStream>

namespace EvilTemple
{

    AnimationStream *Animation::openStream(const Skeleton *skeleton) const
    {
        int boneCount = skeleton->bones().size();
        return new AnimationStream(_keyFramesData, _keyFramesDataStart, boneCount);
    }

    class SkeletonData
    {
    private:
        QByteArray data; // SKA data
        QDataStream stream;

        void loadBones(quint32 count, quint32 dataStart)
        {
            stream.device()->seek(dataStart);

            // It's possible that the SKA file defines more bones than the SKM file
            // Compensate for that here
            if (int(count) > bones.count()) {
                qWarning("SKA file defines %d bones, while SKM file defines %d.", count, bones.count());
                bones.reserve(count); // Reserve memory for later appends
            }

            for (quint32 i = 0; i < count; ++i) {
                loadBone(i);
            }
        }

        void loadBone(quint32 boneId)
        {
            qint16 flags; // Bone flags (ignored)
            qint16 parentId; // Id of parent bone
            stream >> flags >> parentId;

            char rawBoneName[49]; // Name of bone + null termination
            rawBoneName[48] = 0;
            stream.readRawData(rawBoneName, 48);

            // Bones in the skeleton file are associated with bones from the original model
            // It's possible that the skeleton defines additional bones.
            if (int(boneId) >= bones.count()) {
                bones.resize(boneId + 1);

                Bone &newBone = bones[boneId];
                newBone.id = boneId;
                newBone.flags = flags;
                newBone.name = QString::fromLatin1(rawBoneName);
                newBone.parentId = parentId;
                if (parentId != -1)
                    bones[parentId].childrenIds.append(boneId);
            }

            QVector4D scale; // The fourth component is discarded
            QVector4D rotation; // Order is: X,Y,Z,Scalar
            QVector4D translation; // The fourth component is discarded

            stream >> scale >> rotation >> translation;

            // Build the relative world matrix for the bone
            QMatrix4x4 &relativeWorld = bones[boneId].relativeWorld;
            relativeWorld.setToIdentity();
            relativeWorld.translate(translation.x(), translation.y(), translation.z());
            relativeWorld.rotate(QQuaternion(rotation.w(), rotation.x(), rotation.y(), rotation.z()));
            relativeWorld.scale(scale.x(), scale.y(), scale.z());

            // Calculate the full world matrix for the default pose
            QMatrix4x4 fullWorld;
            for (parentId = boneId; parentId != -1; parentId = bones[parentId].parentId) {
                fullWorld = bones[parentId].relativeWorld * fullWorld;
            }
            bones[boneId].defaultPoseWorld = fullWorld * bones[boneId].fullWorldInverse;
        }

        void loadAnimations(quint32 count, quint32 dataStart)
        {
            animations.resize(count);

            for (quint32 i = 0; i < count; ++i)
            {
                loadAnimation(i, dataStart);
            }

        }

        void loadAnimation(quint32 animationId, quint32 dataStart)
        {
            // 0xEC is the size of an entire animation header with all streams (of which only one is used).
            quint64 animDataStart = dataStart + animationId * 0xEC;

            stream.device()->seek(animDataStart);

            Animation &animation = animations[animationId];

            char rawName[65];
            rawName[64] = 0;
            stream.readRawData(rawName, 64);

            animation.setName(QString::fromLatin1(rawName));

            quint8 driveType;
            quint8 loopable;
            quint16 eventCount;
            quint32 eventOffset;
            quint32 streamCount;
            qint16 variationId;
            quint32 dataOffset; // Offset to the start of the key-frame stream in relation to animationStart
            quint16 frames;
            float frameRate;
            float dps;

            stream  >> driveType >> loopable >> eventCount >> eventOffset >> streamCount
                    >> frames >> variationId >> frameRate >> dps >> dataOffset;

            Q_ASSERT(streamCount == 1); // Only one stream is supported
            Q_ASSERT(variationId == -1); // Variations are unsupported

            animation.setDriveType( static_cast<Animation::DriveType>(driveType) );
            animation.setLoopable(loopable != 0);
            animation.setKeyFramesData(data, animDataStart + dataOffset);
            animation.setFrames(frames);
            animation.setFrameRate(frameRate);
            animation.setDps(dps);

            animationMap.insert(animation.name().toLower(), &animation);

            // Read Events
            loadEvents(animation, eventCount, animDataStart + eventOffset);
        }

        void loadEvents(Animation &animation, quint32 count, quint32 dataStart)
        {
            stream.device()->seek(dataStart);

            QVector<AnimationEvent> events(count);

            char type[49];
            type[48] = 0;
            char action[129];
            action[128] = 0;

            for (quint32 i = 0; i < count; ++i) {
                AnimationEvent &event = events[i];

                stream >> event.frameId;
                stream.readRawData(type, 48);
                stream.readRawData(action, 128);

                event.type = QString::fromLatin1(type);
                event.action = QString::fromLatin1(action);
            }

            animation.setEvents(events);
        }

    public:
        QVector<Bone> bones;
        QVector<Animation> animations;
        QHash<QString, Animation*> animationMap;

        SkeletonData(const QVector<Bone> &bones, const QByteArray &_data) : data(_data), stream(data), bones(bones)
        {
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        }

        /**
          Load the initial data from the stream.
          */
        void load()
        {
            quint32 boneCount; // Number of bones in skeleton
            quint32 bonesDataStart; // Start of bones data in bytes
            quint32 variationCount; // Unknown + Unused
            quint32 variationsDataStart; // Unknown + Unused
            quint32 animationCount; // Number of animations in this skeletons
            quint32 animationsDataStart; // Offset to first animation header

            stream  >> boneCount >> bonesDataStart
                    >> variationCount >> variationsDataStart
                    >> animationCount >> animationsDataStart;

            // Assert that this file doesn't contain any variations,
            // since their format is unknown.
            Q_ASSERT(variationCount == 0);

            loadBones(boneCount, bonesDataStart);
            loadAnimations(animationCount, animationsDataStart);
        }
    };

    Skeleton::Skeleton(const QVector<Bone> &bones, const QByteArray &data) : d_ptr(new SkeletonData(bones, data))
    {
        d_ptr->load();
    }

    Skeleton::~Skeleton()
    {
    }

    const QVector<Bone> &Skeleton::bones() const
    {
        return d_ptr->bones;
    }

    const QVector<Animation> &Skeleton::animations() const
    {
        return d_ptr->animations;
    }

    const Animation *Skeleton::findAnimation(const QString &name) const
    {
        QHash<QString,Animation*>::const_iterator it = d_ptr->animationMap.find(name.toLower());
        if (it == d_ptr->animationMap.end()) {
            return 0;
        } else {
            return it.value();
        }
    }

    void Skeleton::draw() const {
        const QVector<Bone> &bones = d_ptr->bones;

        QVector<QVector3D> jointPositions(bones.size());

        for (int i = 0; i < bones.size(); ++i) {
            QMatrix4x4 m; // = bones[i].fullWorldInverse.inverted();
            for (int boneId = i; boneId != -1; boneId = bones[boneId].parentId) {
                m = bones[boneId].relativeWorld * m;
            }
            jointPositions[i] = m * QVector3D();
        }

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, 1);

        glBegin(GL_LINES);

        for (int i = 0; i < bones.size(); ++i) {
            const Bone &bone = bones[i];
            QVector3D position = jointPositions[bone.id];

            foreach (int childId, bone.childrenIds) {
                QVector3D childPosition = jointPositions[childId];

                glVertex3f(position.x(), position.y(), position.z());
                glVertex3f(childPosition.x(), childPosition.y(), childPosition.z());
            }
        }

        glEnd();

        glPointSize(2);

        glBegin(GL_POINTS);
        for (int i = 0; i < bones.size(); ++i) {
            const Bone &bone = bones[i];
            QVector3D position = jointPositions[bone.id];
            glVertex3f(position.x(), position.y(), position.z());
        }
        glEnd();

        glPointSize(3);
        glBegin(GL_POINTS);
        glColor4f(1, 0, 0, 1);
        for (int i = 0; i < bones.size(); ++i) {
            const Bone &bone = bones[i];
            QVector3D position = bone.fullWorldInverse.inverted() * QVector3D();
            glVertex3f(position.x(), position.y(), position.z());
        }
        glEnd();

        glPointSize(1);

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
    }

    SkeletonState::SkeletonState(const Skeleton *skeleton) : _skeleton(skeleton) {
        boneMatrices.reset(new QMatrix4x4[skeleton->bones().size()]);
    }

    SkeletonState::~SkeletonState() {
    }

    const float AnimationStream::rotationFactor = 1 / 32766.0f;

    AnimationStream::AnimationStream(const QByteArray &data, int dataStart, int boneCount)
        : _dataStart(dataStart), _boneCount(boneCount), _boneMap(new AnimationBoneState*[boneCount]), stream(data)
    {
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        stream.device()->seek(dataStart);

        // Read float scaling factors once
        stream >> scaleFactor >> translationFactor;

        _boneStates.resize(countBones()); // Avoid memory reallocation
        memset(_boneMap.data(), 0, sizeof(AnimationBoneState*) * boneCount);

        rewind();
    }

    int AnimationStream::countBones()
    {
        qint16 boneId;
        int bones = 0;

        stream >> boneId;

        while (boneId != -2) {
            ++bones;

            stream.skipRawData((3 + 3 + 4) * 2); // Skip the bone data

            stream >> boneId;
        }

        return bones;
    }

    void AnimationStream::readNextFrame()
    {
        if (nextFrameId == -1) {
            // Make all "next states" current and keep them forever
            for (int i = 0; i < _boneStates.size(); ++i) {
                AnimationBoneState &state = _boneStates[i];
                state.scale = state.nextScale;
                state.scaleFrame = state.nextScaleFrame;
                state.rotation = state.nextRotation;
                state.rotationFrame = state.nextRotationFrame;
                state.translation = state.nextTranslation;
                state.translationFrame = state.nextTranslationFrame;
            }

            return;
        }

        qint16 boneHeader;

        // Read Bone Chunks of Key Frame
        stream >> boneHeader;

        // The LSB is the flag whether this is a bone-chunk or the id of the next frame
        while ((boneHeader & 1) == 1) {
            int boneId = boneHeader >> 4;

            // Get a pointer to the bone state affected by this key frame chunk
            AnimationBoneState *state = _boneMap[boneId];

            Q_ASSERT(state);

            // Scale Delta Frame
            if ((boneHeader & 8) == 8) {
                state->scale = state->nextScale;
                state->scaleFrame = state->nextScaleFrame;

                qint16 nextFrame, x, y, z;
                stream >> nextFrame >> x >> y >> z;

                state->nextScaleFrame = nextFrame;
                state->nextScale.setX(x * scaleFactor);
                state->nextScale.setY(y * scaleFactor);
                state->nextScale.setZ(z * scaleFactor);
            }

            // Rotation Delta Frame
            if ((boneHeader & 4) == 4) {
                state->rotation = state->nextRotation;
                state->rotationFrame = state->nextRotationFrame;

                qint16 nextFrame, x, y, z, w;                
                stream >> nextFrame >> x >> y >> z >> w;

                state->nextRotationFrame = nextFrame;
                state->rotation.setX(x * rotationFactor);
                state->rotation.setY(y * rotationFactor);
                state->rotation.setZ(z * rotationFactor);
                state->rotation.setScalar(w * rotationFactor);
            }

            // Translation Delta Frame
            if ((boneHeader & 2) == 2) {
                state->translation = state->nextTranslation;
                state->translationFrame = state->nextTranslationFrame;

                qint16 nextFrame, x, y, z;
                stream >> nextFrame >> x >> y >> z;

                state->nextTranslationFrame = nextFrame;
                state->nextTranslation.setX(x * translationFactor);
                state->nextTranslation.setY(y * translationFactor);
                state->nextTranslation.setZ(z * translationFactor);
            }

            // Read header of next chunk
            stream >> boneHeader;
        }

        // Store the header for the next key frame
        nextFrameId = boneHeader >> 1;
    }

    void AnimationStream::rewind()
    {
        stream.device()->seek(_dataStart + 2 * sizeof(float)); // Seek to the start of the bone data

        qint16 boneId;
        qint16 x, y, z, w; // Vector components

        stream >> boneId;

        int i = 0;

        while (boneId != -2) {

            AnimationBoneState &boneState = _boneStates[i++];
            _boneMap[boneId] = &boneState;

            boneState.boneId = boneId;
            
            stream >> x >> y >> z; // Scale
            boneState.scale.setX(x * scaleFactor);
            boneState.scale.setY(y * scaleFactor);
            boneState.scale.setZ(z * scaleFactor);            
            boneState.scaleFrame = 0;
            boneState.nextScaleFrame = 0;
            boneState.nextScale = boneState.scale;

            stream >> x >> y >> z >> w; // Rotation
            boneState.rotation.setVector(x * rotationFactor, y * rotationFactor, z * rotationFactor);
            boneState.rotation.setScalar(w * rotationFactor);
            boneState.rotationFrame = 0;
            boneState.nextRotationFrame = 0;
            boneState.nextRotation = boneState.rotation;

            stream >> x >> y >> z; // Translation
            boneState.translation.setX(x * translationFactor);
            boneState.translation.setY(y * translationFactor);
            boneState.translation.setZ(z * translationFactor);            
            boneState.translationFrame = 0;
            boneState.nextTranslationFrame = 0;
            boneState.nextTranslation = boneState.translation;

            stream >> boneId;
        }

        stream >> nextFrameId;

        readNextFrame();
    }

    void AnimationStream::seek(int frame)
    {
        while (nextFrameId < frame && nextFrameId > 0) {
            readNextFrame();
        }
    }

}
