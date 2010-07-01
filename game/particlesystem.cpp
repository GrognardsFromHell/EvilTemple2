
#include <GL/glew.h>

#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QStringList>

#include <QtCore/QList>
#include <QtCore/QWeakPointer>
#include <QtCore/QTime>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QPointer>

#include <QtXml/QDomElement>

#include "texture.h"

#include "particlesystem.h"
#include "renderstates.h"
#include "material.h"
#include "materialstate.h"
#include "materials.h"
#include "models.h"
#include "modelinstance.h"
#include "scenenode.h"
#include "drawhelper.h"

#include <time.h>

namespace EvilTemple {

    class Emitter;
    class Particle;

    /**
      Converts from spherical (polar) coordinates to cartesian coordinates.
      Uses the following convention:

      - theta is the angle in the x,z plane, measured from the z axis
      - phi is the angle from the y axis
      */
    inline Vector4 polarToCartesian(float theta, float phi, float r)
    {
        // In the usual sense, theta is measured from the X axis, in our case from the Z axis
        // No conversion yet.
        float newX = std::sin(theta) * std::cos(phi);
        float newZ = std::cos(theta) * std::cos(phi);
        float newY = std::sin(phi);

        return r * Vector4(newX, newY, newZ, 0);
    }

    inline void cartesianToPolar(const Vector4 &pos, double &theta, double &phi, double &r)
    {
        r = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y() + pos.z() * pos.z());
        theta = std::atan2(pos.x() / r, pos.z() / r);
        phi = std::asin(pos.y() / r);
    }

    const static float ParticlesTimeUnit = 1 / 30.0f; // All time-based values are in relation to this base value

    const static uint ParticleLimit = 1000; // The maximum number of particles a single particle system may have

    enum ParticleType {
        Type_Sprite,
        Type_Disc,
        Type_Model,
        Type_Point
    };

    enum CoordinateType {
        Cartesian,
        Polar
    };

    enum ParticleBlendMode {
        Blend_Add,
        Blend_Subtract,
        Blend_Blend,
    };

    enum SpaceType {
        Space_World,
        Space_Bone,
        Space_RandomBone
    };

    /**
     * Models a property of an emitter or particle, which can assume one of the following roles:
     * - Fixed value
     * - Random value
     * - Animated value
     */
    template<typename T> class ParticleProperty {
    public:
        virtual ~ParticleProperty()
        {
        }

        /**
         * Returns the value of the property, given the life-time of the particle expressed as a range of [0,1].
         */
        virtual T operator()(const Emitter *emitter, uint randomSeed, float ratio) const = 0;

        /**
         * Indicates whether this property is animated and needs to be queried constantly. If not,
         * the value will only be queried whenever a particle system or emitter is created.
         */
        virtual bool isAnimated() const = 0;
    };

    /**
      * Models a constant particle property without any animation.
      */
    template<typename T> class ConstantParticleProperty : public ParticleProperty<T> {
    public:
        ConstantParticleProperty(T value) : mValue(value)
        {
        }

        T operator()(const Emitter *emitter, uint randomSeed, float ratio) const
        {
            Q_UNUSED(emitter);
            Q_UNUSED(randomSeed);
            Q_UNUSED(ratio);
            return mValue;
        }

        bool isAnimated() const
        {
            return false;
        }

    private:
        T mValue;
    };

    /**
     * This property will always return the radius of the object this particle system is attached to.
     * TODO: Implement this properly
     */
    template<typename T> class RadiusProperty : public ParticleProperty<T> {
    public:
        T operator()(const Emitter *emitter, uint randomSeed, float ratio) const
        {
            Q_UNUSED(ratio);
            Q_UNUSED(emitter);
            Q_UNUSED(randomSeed);
            return 20;
        }

        bool isAnimated() const
        {
            return false;
        }
    };

    static uint nextRandomPropertyId = 0;

    /**
      * Models a random particle property that will pick a random value when queried from a range (uniform distribution).
      */
    template<typename T> class RandomParticleProperty : public ParticleProperty<T> {
    public:

        /**
         * Constructs a random particle property with a minimum and maximum value (both inclusive).
         */
        RandomParticleProperty(T minValue, T maxValue)
            : mMinValue(minValue), mSpan(maxValue - mMinValue), mPropertyId(nextRandomPropertyId++)
        {
        }

        T operator()(const Emitter *emitter, uint randomSeed, float ratio) const
        {
            Q_UNUSED(emitter);
            Q_UNUSED(ratio);

            //srand(randomSeed + mPropertyId);
            float result = mMinValue + (rand() / (float)RAND_MAX) * mSpan;
            //srand(time(NULL)); // TODO: This should probably be removed, and the PRNG polynom be used directly
            return result;
        }

        bool isAnimated() const
        {
            return false;
        }

        static RandomParticleProperty<float> *fromString(const QString &string) {
            QStringList parts = string.split('?');
            if (parts.length() > 2) {
                qWarning("Random properties may only contain a single question mark: %s.", qPrintable(string));
                return NULL;
            }

            bool ok;
            float minValue = parts[0].toFloat(&ok);

            if (!ok) {
                qWarning("The minimum range of random value %s is non-numeric.", qPrintable(string));
                return NULL;
            }

            float maxValue = parts[1].toFloat(&ok);

            if (!ok) {
                qWarning("The maximum range of random value %s is non-numeric.", qPrintable(string));
                return NULL;
            }

            if (minValue > maxValue)
                std::swap(minValue, maxValue);

            return new RandomParticleProperty<float>(minValue, maxValue);
        }

    private:
        T mMinValue;
        T mSpan;
        uint mPropertyId;
    };

    /**
      * Interpolates between evenly spaced key-frames.
      */
    template<typename T> class AnimatedParticleProperty : public ParticleProperty<T> {
    public:
        AnimatedParticleProperty(const QVector<T> &values)
            : mStep(1.0f / (values.size() - 1)), mValues(values)
        {
        }

        T operator()(const Emitter *emitter, uint randomSeed, float ratio) const {
            Q_UNUSED(emitter);
            Q_UNUSED(randomSeed);

            if (mValues.size() == 1) {
                return mValues[0];
            }

            // Clamp to [0,1]
            ratio = qMin<float>(1, qMax<float>(0, ratio));

            int index = qMax<int>(0, floor(ratio * (mValues.size() - 1)));
            int nextIndex = qMin<int>(mValues.size() - 1, ceil(ratio * (mValues.size() - 1)));

            Q_ASSERT(index < mValues.size());

            if (nextIndex >= mValues.size()) {
                return mValues[index];
            }

            T first = mValues[index];
            T second = mValues[nextIndex];

            float i = (ratio - index * mStep) / mStep;

            return first + (second - first) * i;
        }

        bool isAnimated() const
        {
            return true;
        }

        static AnimatedParticleProperty<float> *fromString(const QString &string)
        {
            QStringList parts = string.split(',', QString::SkipEmptyParts);
            QVector<float> values;
            values.reserve(parts.size());

            foreach (const QString &part, parts) {
                bool ok;
                float value = part.trimmed().toFloat(&ok);

                if (!ok) {
                    qWarning("Animated value list contains invalid value: %s.", qPrintable(part));
                } else {
                    values.append(value);
                }
            }

            return new AnimatedParticleProperty<float>(values);
        }

    private:
        float mStep;
        QVector<T> mValues;
    };

    /**
      * Interpolates between evenly spaced key-frames. But may also contain random entries,
      * which are computed on a per-particle base.
      */
    template<typename T> class AnimatedRandomParticleProperty : public ParticleProperty<T> {
    public:
        AnimatedRandomParticleProperty(const QVector< QPair<T,T> > &values)
            : mStep(1.0f / (values.size() - 1)), mValues(values), mPropertyId(nextRandomPropertyId++)
        {
        }

        inline T getValue(const Emitter *emitter, uint randomSeed, uint i) const {
            Q_UNUSED(emitter);
            Q_UNUSED(randomSeed);

            QPair<T,T> result = mValues[i];

            // If second element of pair is NAN, return the first
            if (result.second != result.second)
                return result.first;

            // Otherwise we have a random element
            //srand(randomSeed + mPropertyId * 1000 + i);
            T value = result.first + (rand() / (float)RAND_MAX) * result.second;
            //srand(time(NULL));
            return value;
        }

        T operator()(const Emitter *emitter, uint randomSeed, float ratio) const {
            if (mValues.size() == 1) {
                return getValue(emitter, randomSeed, 0);
            }

            // Clamp to [0,1]
            ratio = qMin<float>(1, qMax<float>(0, ratio));

            int index = floor(ratio * (mValues.size() - 1));
            int nextIndex = ceil(ratio * (mValues.size() - 1));

            Q_ASSERT(index < mValues.size());

            if (nextIndex >= mValues.size()) {
                return getValue(emitter, randomSeed, index);
            }

            T first = getValue(emitter, randomSeed, index);
            T second = getValue(emitter, randomSeed, nextIndex);

            float i = (ratio - index * mStep) / mStep;

            return first + (second - first) * i;
        }

        bool isAnimated() const
        {
            return true;
        }

        static AnimatedRandomParticleProperty<float> *fromString(const QString &string)
        {
            QStringList parts = string.split(',', QString::SkipEmptyParts);
            QVector< QPair<float,float> > values;
            values.reserve(parts.size());

            foreach (const QString &part, parts) {
                bool ok;
                QPair<float,float> value;

                // Random entries in the animated list are denoted by a pair whose second entry is not NaN
                if (part.contains('?')) {
                    QStringList subParts = part.split('?');
                    Q_ASSERT(subParts.size() == 2);

                    value.first = subParts[0].trimmed().toFloat(&ok);
                    value.second = subParts[1].trimmed().toFloat(&ok) - value.first;
                } else {
                    value.first = part.trimmed().toFloat(&ok);
                    value.second = std::numeric_limits<float>::quiet_NaN();
                }

                if (!ok) {
                    qWarning("Animated value list contains invalid value: %s.", qPrintable(part));
                } else {
                    values.append(value);
                }
            }

            return new AnimatedRandomParticleProperty<float>(values);
        }

    private:
        float mStep;
        QVector< QPair<T,T> > mValues;
        uint mPropertyId;
    };

    inline ParticleProperty<float> *propertyFromString(const QString &string)
    {
        ParticleProperty<float> *result = NULL;

        if (string.contains('?') && string.contains(',')) {
            result = AnimatedRandomParticleProperty<float>::fromString(string);
        } else if (string.contains('?')) {
            result = RandomParticleProperty<float>::fromString(string);
        } else if (string.contains(',')) {
            result = AnimatedParticleProperty<float>::fromString(string);
        } else if (string == "#radius") {
            result = new RadiusProperty<float>();
        } else {
            bool ok;
            float value = string.toFloat(&ok);

            if (!ok) {
                qWarning("Invalid floating point constant: %s.", qPrintable(string));
            } else {
                result = new ConstantParticleProperty<float>(value);
            }
        }

        // Fall back to 0 constant
        if (!result) {
            result = new ConstantParticleProperty<float>(0);
        }

        return result;
    }


    class Particle : public AlignedAllocation {
    public:
        Particle() : rotationYaw(0), rotationPitch(0), rotationRoll(0),
            colorRed(255), colorGreen(255), colorBlue(255), colorAlpha(255),
            scale(100),
            accelerationX(0), accelerationY(0), accelerationZ(0),
            velocityX(0), velocityY(0), velocityZ(0)
        {
            randomSeed = rand();
        }

        Vector4 position;
        float rotationYaw, rotationPitch, rotationRoll;
        float colorRed, colorGreen, colorBlue, colorAlpha;
        float accelerationX, accelerationY, accelerationZ;
        float velocityX, velocityY, velocityZ;
        float scale;
        float expireTime;
        float startTime;
        uint randomSeed; // Used for per-particle randomness
    };

    class Emitter {
    public:

        typedef const ParticleProperty<float> *Property;

        Emitter(ParticleSystem *particleSystem, float spawnRate, float particleLifetime)
            : mParticleSystem(particleSystem), mPartialSpawnedParticles(0), mElapsedTime(0), mExpired(false),
            mSpawnRate(1/(spawnRate *ParticlesTimeUnit)), mParticleLifetime(particleLifetime), mLifetime(std::numeric_limits<float>::infinity()),
            mEmitterSpace(Space_World)
        {
        }

        void elapseTime(float timeUnits);

        /**
          Spawns a single particle
         */
        void spawnParticle(float atTime);

        void updateParticles(float elapsedTimeUnits);

        void updateParticle(Particle &particle, float elapsedTimeUnits);

        void render(RenderStates &renderStates);

        void renderModel(RenderStates &renderStates);

        /**
          Dead = No more particles will spawn and no particles are still active.
          */
        bool isDead() const
        {
            return mElapsedTime > mLifetime && mParticles.isEmpty();
        }

        void setLifetime(float lifetime)
        {
            mLifetime = lifetime;
            if (mElapsedTime > mLifetime) {
                mExpired = true;
            }
        }

        void setScale(Property scale)
        {
            mScale = scale;
        }

        void setColor(Property colorRed, Property colorGreen, Property colorBlue, Property colorAlpha)
        {
            mColorRed = colorRed;
            mColorGreen = colorGreen;
            mColorBlue = colorBlue;
            mColorAlpha = colorAlpha;
        }

        void setEmitterSpace(SpaceType spaceType)
        {
            mEmitterSpace = spaceType;
        }

        void setRotation(Property rotationYaw, Property rotationPitch, Property rotationRoll)
        {
            mRotationYaw = rotationYaw;
            mRotationPitch = rotationPitch;
            mRotationRoll = rotationRoll;
        }

        void setParticleVelocity(Property velocityX, Property velocityY, Property velocityZ, CoordinateType type)
        {
            mParticleVelocityX = velocityX;
            mParticleVelocityY = velocityY;
            mParticleVelocityZ = velocityZ;
            mParticleVelocityType = type;
        }

        void setParticlePosition(Property positionX, Property positionY, Property positionZ, CoordinateType type)
        {
            mParticlePositionX = positionX;
            mParticlePositionY = positionY;
            mParticlePositionZ = positionZ;
            mParticlePositionType = type;
        }

        void setAcceleration(Property accelerationX, Property accelerationY, Property accelerationZ)
        {
            mAccelerationX = accelerationX;
            mAccelerationY = accelerationY;
            mAccelerationZ = accelerationZ;
        }

        void setBlendMode(ParticleBlendMode blendMode)
        {
            mBlendMode = blendMode;
        }

        void setTexture(const SharedTexture &texture)
        {
            mTexture = texture;
        }

        void setPosition(Property positionX, Property positionY, Property positionZ)
        {
            mPositionX = positionX;
            mPositionY = positionY;
            mPositionZ = positionZ;
        }

        void setParticleType(ParticleType type)
        {
            mParticleType = type;
        }

        void setBoneName(const QString &boneName)
        {
            mBoneName = boneName;
        }

        void setName(const QString &name)
        {
            mName = name;
        }

        void setMaterial(const SharedMaterialState &material)
        {
            mMaterial = material;
        }

        void setModel(const SharedModel &model)
        {
            mModel = model;
        }

        ParticleSystem *particleSystem() const
        {
            return mParticleSystem;
        }

    private:

        SharedMaterialState mMaterial;

        Property mScale;

        Property mColorAlpha, mColorRed, mColorGreen, mColorBlue;

        Property mRotationYaw, mRotationPitch, mRotationRoll;

        Property mAccelerationX, mAccelerationY, mAccelerationZ;

        Property mParticleVelocityX, mParticleVelocityY, mParticleVelocityZ;
        CoordinateType mParticleVelocityType;

        Property mParticlePositionX, mParticlePositionY, mParticlePositionZ;
        CoordinateType mParticlePositionType;

        ParticleType mParticleType;

        QList<Particle> mParticles;

        SpaceType mEmitterSpace;

        QString mBoneName; // For direct bone refs

        QString mName;

        // Current position of this emitter
        Property mPositionX, mPositionY, mPositionZ;

        // All particles of an emitter use the same material
        SharedTexture mTexture;

        // They may also use a model
        SharedModel mModel;

        ParticleSystem *mParticleSystem;

        ParticleBlendMode mBlendMode;
        bool mExpired; // More time elapsed than this emitter's lifetime
        float mElapsedTime;
        float mParticleLifetime; // Lifetime of newly created particles in time units
        float mLifetime; // Number of time units until this emitter stops working. Can be Infinity.
        float mSpawnRate; // One particle per this many time units is spawned
        float mPartialSpawnedParticles; // If the elapsed time is not enough to spawn another particle, it is accumulated.

        Q_DISABLE_COPY(Emitter);
    };

    void Emitter::updateParticles(float elapsedTimeunits)
    {
        for (int i = 0; i < mParticles.size(); ++i) {
            updateParticle(mParticles[i], elapsedTimeunits);
        }
    }

    void Emitter::spawnParticle(float atTime)
    {
        Particle particle;

        if (mRotationYaw) {
            particle.rotationYaw = (*mRotationYaw)(this, particle.randomSeed, 0);
        }
        if (mRotationPitch) {
            particle.rotationPitch = (*mRotationPitch)(this, particle.randomSeed,0);
        }
        if (mRotationRoll) {
            particle.rotationRoll = (*mRotationRoll)(this, particle.randomSeed,0);
        }

        if (mAccelerationX) {
            particle.accelerationX = (*mAccelerationX)(this, particle.randomSeed,0);
        }
        if (mAccelerationY) {
            particle.accelerationY = (*mAccelerationY)(this, particle.randomSeed,0);
        }
        if (mAccelerationZ) {
            particle.accelerationZ = (*mAccelerationZ)(this, particle.randomSeed,0);
        }

        if (mParticleVelocityX) {
            particle.velocityX = (*mParticleVelocityX)(this, particle.randomSeed,0);
            if (mParticleVelocityType == Polar)
                particle.velocityX = deg2rad(particle.velocityX);
        }
        if (mParticleVelocityY) {
            particle.velocityY = (*mParticleVelocityY)(this, particle.randomSeed,0);
            if (mParticleVelocityType == Polar)
                particle.velocityY = deg2rad(particle.velocityY);
        }
        if (mParticleVelocityZ) {
            particle.velocityZ = (*mParticleVelocityZ)(this, particle.randomSeed,0);
        }

        if (mScale) {
            particle.scale = (*mScale)(this, particle.randomSeed,0);
        }

        if (mColorRed) {
            particle.colorRed = (*mColorRed)(this, particle.randomSeed, 0);
        }
        if (mColorGreen) {
            particle.colorGreen = (*mColorGreen)(this, particle.randomSeed, 0);
        }
        if (mColorBlue) {
            particle.colorBlue = (*mColorBlue)(this, particle.randomSeed, 0);
        }
        if (mColorAlpha) {
            particle.colorAlpha = (*mColorAlpha)(this, particle.randomSeed, 0);
        }

        Vector4 positionOffset(0, 0, 0, 0);
        if (mParticlePositionX) {
            positionOffset.setX((*mParticlePositionX)(this, particle.randomSeed,0));
        }
        if (mParticlePositionY) {
            positionOffset.setY((*mParticlePositionY)(this, particle.randomSeed,0));
        }
        if (mParticlePositionZ) {
            positionOffset.setZ((*mParticlePositionZ)(this, particle.randomSeed,0));
        }
        // Convert to cartesian if necessary
        if (mParticlePositionType == Polar) {
            positionOffset = polarToCartesian(positionOffset.x(), positionOffset.y(), positionOffset.z());
        }
        particle.position = Vector4((*mPositionX)(this, particle.randomSeed, 0),
                                    (*mPositionY)(this, particle.randomSeed, 0),
                                    (*mPositionZ)(this, particle.randomSeed, 0),
                                    1) + positionOffset;

        /*
         In case the emitter space is "Bones", a bone is randomly selected to spawn the particle
         */
        ModelInstance *modelInstance = mParticleSystem->modelInstance();
        if (modelInstance) {
            Matrix4 flipZ;
            flipZ.setToIdentity();
            flipZ(2,2) *= -1;

            if (mEmitterSpace == Space_RandomBone) {
                while (true) {
                    // Choose a bone at random (?)
                    const QVector<Bone> &bones = modelInstance->model()->bones();
                    Q_ASSERT(bones.size() > 1);
                    const Bone &bone = bones[1 + (rand() % (bones.size() - 1))];

                    // Skip a certain set of "ref" bones
                    if (bone.name() == "groundParticleRef" || bone.name() == "Chest_ref" || bone.name() == "HandR_ref"
                        || bone.name() == "HandL_ref" || bone.name() == "Head_ref" || bone.name() == "FootR_ref"
                        || bone.name() == "FootL_ref" || bone.name() == "Bip01 Footsteps"
                        || bone.name() == "Bip01" || bone.name() == "origin"
                        || bone.name() == "EarthElemental_reg" || bone.name() == "Casting_ref"
                        || bone.name() == "Origin" || bone.name() == "Footstep" || bone.name() == "Pony")
                        continue;

                    Matrix4 boneSpace = flipZ * modelInstance->getBoneSpace(bone.boneId()) * flipZ;
                    Vector4 trans = boneSpace.column(3);
                    trans.setW(0);
                    particle.position += trans;
                    break;
                }
            } else if (mEmitterSpace == Space_Bone) {
                int boneId = modelInstance->model()->bone(mBoneName);

                if (boneId != -1) {
                    Matrix4 boneSpace = flipZ * modelInstance->getBoneSpace(boneId) * flipZ;
                    Vector4 trans = boneSpace.column(3);
                    trans.setW(0);
                    particle.position += trans;
                }
            }
        }

        particle.startTime = atTime;
        particle.expireTime = atTime + mParticleLifetime;
        mParticles.append(particle);
    }

    void Emitter::updateParticle(Particle &particle, float elapsedTimeUnits)
    {
        // How many time units have elapsed since the particle was created
        float particleElapsed = mElapsedTime - particle.startTime;
        // The number of time units the particle will live after its creation
        float particleLifetime = particle.expireTime - particle.startTime;
        // A factor between 0 and 1 that indicates how much of the particles lifetime has elapsed
        float lifecycle = particleElapsed / particleLifetime;

        float scalingFactor = elapsedTimeUnits * ParticlesTimeUnit;

        if (mRotationYaw && mRotationYaw->isAnimated()) {
            particle.rotationYaw = (*mRotationYaw)(this, particle.randomSeed, lifecycle);
        }
        if (mRotationPitch && mRotationPitch->isAnimated()) {
            particle.rotationPitch = (*mRotationPitch)(this, particle.randomSeed, lifecycle);
        }
        if (mRotationRoll && mRotationRoll->isAnimated()) {
            particle.rotationRoll = (*mRotationRoll)(this, particle.randomSeed, lifecycle);
        }

        if (mScale && mScale->isAnimated()) {
            particle.scale = (*mScale)(this, particle.randomSeed, lifecycle);
        }

        if (mColorRed && mColorRed->isAnimated()) {
            particle.colorRed = (*mColorRed)(this, particle.randomSeed, lifecycle);
        }
        if (mColorGreen && mColorGreen->isAnimated()) {
            particle.colorGreen = (*mColorGreen)(this, particle.randomSeed, lifecycle);
        }
        if (mColorBlue && mColorBlue->isAnimated()) {
            particle.colorBlue = (*mColorBlue)(this, particle.randomSeed, lifecycle);
        }
        if (mColorAlpha && mColorAlpha->isAnimated()) {
            particle.colorAlpha = (*mColorAlpha)(this, particle.randomSeed, lifecycle);
        }

        if (mAccelerationX && mAccelerationX->isAnimated()) {
            particle.accelerationX = (*mAccelerationX)(this, particle.randomSeed, lifecycle);
        }
        if (mAccelerationY && mAccelerationY->isAnimated()) {
            particle.accelerationY = (*mAccelerationY)(this, particle.randomSeed, lifecycle);
        }
        if (mAccelerationZ && mAccelerationZ->isAnimated()) {
            particle.accelerationZ = (*mAccelerationZ)(this, particle.randomSeed, lifecycle);
        }

        // Increase velocity according to acceleration or animate it using keyframes
        if (mParticleVelocityX && mParticleVelocityX->isAnimated()) {
            particle.velocityX = (*mParticleVelocityX)(this, particle.randomSeed, lifecycle);
            if (mParticleVelocityType == Polar)
                particle.velocityX = deg2rad((*mParticleVelocityX)(this, particle.randomSeed, lifecycle));
            else
                particle.velocityX = (*mParticleVelocityX)(this, particle.randomSeed, lifecycle);
        } else {
            if (mParticleVelocityType == Polar)
               particle.velocityX += deg2rad(particle.accelerationX) * scalingFactor;
            else
               particle.velocityX += particle.accelerationX * scalingFactor;
        }
        if (mParticleVelocityY && mParticleVelocityY->isAnimated()) {
            if (mParticleVelocityType == Polar)
                particle.velocityY = deg2rad((*mParticleVelocityY)(this, particle.randomSeed, lifecycle));
            else
                particle.velocityY = (*mParticleVelocityY)(this, particle.randomSeed, lifecycle);
        } else {
             if (mParticleVelocityType == Polar)
                particle.velocityY += deg2rad(particle.accelerationY) * scalingFactor;
             else
                 particle.velocityY += particle.accelerationY * scalingFactor;
        }
        if (mParticleVelocityZ && mParticleVelocityZ->isAnimated()) {
            particle.velocityZ = (*mParticleVelocityZ)(this, particle.randomSeed, lifecycle);
        } else {
            particle.velocityZ += particle.accelerationZ * scalingFactor;
        }

        if (mParticleVelocityType == Polar) {
            Vector4 direction = particle.position;
            direction.setW(0);
            float r = direction.length();

            if (!qFuzzyIsNull(r)) {
                Vector4 rotAxis;
                if (particle.position.x() != 0 || particle.position.z() != 0) {
                    Vector4 rotNormal(0, 1, 0, 0);
                    rotAxis = rotNormal.cross(direction).normalized();
                } else {
                    // In case the point is above the origin (x=0,y=0), use the x axis as a rotation axis, doesn't matter.
                    rotAxis = Vector4(1, 0, 0, 0);
                }

                // Extrude the position outwards
                particle.position += (particle.velocityZ * scalingFactor) * direction.normalized();

                //Quaternion rot1 = Quaternion::fromAxisAndAngle(rotAxis.x(), rotAxis.y(), rotAxis.z(), particle.velocityY * elapsedTimeUnits);
                Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, particle.velocityX * elapsedTimeUnits);
                particle.position = Matrix4::rotation(rot2) * particle.position;
            }
        } else {
            particle.position += scalingFactor * Vector4(particle.velocityX, particle.velocityY, particle.velocityZ, 0);
        }

        // An animated position overrides any velocity calculations
        if (mParticlePositionType == Cartesian) {
            // This is very simple for cartesian coordinates
            if (mParticlePositionX && mParticlePositionX->isAnimated()) {
                particle.position.setX((*mParticlePositionX)(this, particle.randomSeed, lifecycle));
            }
            if (mParticlePositionY && mParticlePositionY->isAnimated()) {
                particle.position.setY((*mParticlePositionY)(this, particle.randomSeed, lifecycle));
            }
            if (mParticlePositionZ && mParticlePositionZ->isAnimated()) {
                particle.position.setZ((*mParticlePositionZ)(this, particle.randomSeed, lifecycle));
            }
        } else if (mParticlePositionType == Polar) {
            // Convert current coordinate from cartesian back to polar
            double r, theta, phi;
            cartesianToPolar(particle.position, theta, phi, r);

            // Get the difference between this lifecycle and the previous one
            particleElapsed = (mElapsedTime - elapsedTimeUnits) - particle.startTime;

            float prevLifecycle = qMax<float>(0, particleElapsed / particleLifetime);

            if (mParticlePositionX && mParticlePositionX->isAnimated()) {
                theta += deg2rad((*mParticlePositionX)(this, particle.randomSeed, lifecycle) - (*mParticlePositionX)(this, particle.randomSeed, prevLifecycle));
            }
            if (mParticlePositionY && mParticlePositionY->isAnimated()) {
                phi += deg2rad((*mParticlePositionY)(this, particle.randomSeed, lifecycle) - (*mParticlePositionY)(this, particle.randomSeed, prevLifecycle));
            }
            if (mParticlePositionZ && mParticlePositionZ->isAnimated()) {
                r += (*mParticlePositionZ)(this, particle.randomSeed, lifecycle) - (*mParticlePositionZ)(this, particle.randomSeed, prevLifecycle);
            }

            particle.position = polarToCartesian(theta, phi, r);
        }
    }

    void Emitter::elapseTime(float timeUnits) {
        Q_ASSERT(mSpawnRate > 0);

        mElapsedTime += timeUnits;

        // Check for expired particles
        QList<Particle>::iterator it = mParticles.begin();
        while (it != mParticles.end()) {
            if (mElapsedTime >= it->expireTime)
                it = mParticles.erase(it);
            else
                ++it;
        }

        // Special case: If this emitter depends on a bone that doesn't exist,
        // don't do anything.
        if (mEmitterSpace == Space_Bone) {
            ModelInstance *modelInstance = mParticleSystem->modelInstance();

            if (!modelInstance || !modelInstance->model() || modelInstance->model()->bone(mBoneName) == -1)
                return;
        }

        // Spawn new particles
        if (!mExpired) {
            float remainingSpawnTime = mPartialSpawnedParticles + timeUnits;

            while (remainingSpawnTime > mSpawnRate) {
                spawnParticle(mElapsedTime - remainingSpawnTime + mSpawnRate);
                remainingSpawnTime -= mSpawnRate;
            }

            // A slight hack that prevents spawn-spikes
            mPartialSpawnedParticles = remainingSpawnTime;

            if (mElapsedTime > mLifetime) {
                mExpired = true;
            }
        }

        updateParticles(timeUnits);
    }

    void Emitter::renderModel(RenderStates &renderStates)
    {
        Q_ASSERT(mParticleType == Type_Model);

        Matrix4 oldWorld = renderStates.worldMatrix();

        ModelBufferSource bufferSource(mModel->positionBuffer.bufferId(),
                                       mModel->normalBuffer.bufferId(),
                                       mModel->texcoordBuffer.bufferId());

        foreach (const Particle &particle, mParticles) {
            Vector4 origin = oldWorld.mapPosition(particle.position);

            Matrix4 translation = Matrix4::translation(origin.x(), origin.y(), origin.z());

            Matrix4 rotation = Matrix4::rotation(Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(particle.rotationYaw)))
                               * Matrix4::rotation(Quaternion::fromAxisAndAngle(1, 0, 0, deg2rad(particle.rotationPitch)))
                               * Matrix4::rotation(Quaternion::fromAxisAndAngle(0, 0, 1, deg2rad(particle.rotationRoll)));

            Matrix4 world = translation * rotation;

            renderStates.setWorldMatrix(world);

            Vector4 materialColor(particle.colorRed / 255.0f,
                                  particle.colorGreen / 255.0f,
                                  particle.colorBlue / 255.0f,
                                  particle.colorAlpha / 255.0f);

            for (int j = 0; j < mModel->faces; ++j) {
                FaceGroup &faceGroup = mModel->faceGroups[j];

                ModelDrawStrategy drawStrategy(faceGroup.buffer.bufferId(), faceGroup.indices.size());

                MaterialState *material = faceGroup.material;

                for (int i = 0; i < material->passCount; ++i) {
                    MaterialPassState &pass = material->passes[i];

                    pass.program->bind();

                    // Bind texture samplers
                    for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                        pass.textureSamplers[j].bind();
                    }

                    // Bind uniforms
                    for (int j = 0; j < pass.uniforms.size(); ++j) {
                        pass.uniforms[j]->bind();
                    }

                    GLint materialColorLocation = pass.program->uniformLocation("materialColor");
                    if (materialColorLocation != -1) {
                        glUniform4fv(materialColorLocation, 1, materialColor.data());
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
                    drawStrategy.draw(renderStates, pass);

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

                    pass.program->unbind();
                }
            }
        }

        renderStates.setWorldMatrix(oldWorld);
    }

    void Emitter::render(RenderStates &renderStates)
    {
        if (mParticleType == Type_Model) {
            renderModel(renderStates);
            return;
        } else if (mParticleType == Type_Point) {
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(renderStates.projectionMatrix().data());

            glMatrixMode(GL_MODELVIEW);

            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            switch (mBlendMode)
            {
            case Blend_Add:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            case Blend_Blend:
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
                break;
            case Blend_Subtract:
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
                glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
                break;
            }

            Matrix4 oldWorld = renderStates.worldMatrix();

            glBegin(GL_POINTS);
            foreach (const Particle &particle, mParticles) {
                Vector4 origin = oldWorld.mapPosition(particle.position);
                renderStates.setWorldMatrix(Matrix4::translation(origin.x(), origin.y(), origin.z()));

                glColor4f(particle.colorRed / 255.0f, particle.colorGreen / 255.0f, particle.colorBlue / 255.0f,
                            particle.colorAlpha / 255.0f);
                glVertex4fv(origin.data());
            }
            glEnd();

            renderStates.setWorldMatrix(oldWorld);

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            return;
        }

        MaterialPassState &pass = mMaterial->passes[0];

        int posAttrib = pass.program->attributeLocation("vertexPosition");
        int texAttrib = pass.program->attributeLocation("vertexTexCoord");
        int rotationLoc = pass.program->uniformLocation("rotation");
        int materialColorLoc = pass.program->uniformLocation("materialColor");

        pass.program->bind();

        glActiveTexture(GL_TEXTURE0);
        mTexture->bind();
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        switch (mBlendMode)
        {
        case Blend_Add:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case Blend_Blend:
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
            break;
        case Blend_Subtract:
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
            glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
            break;
        }

        Matrix4 oldWorld = renderStates.worldMatrix();

        foreach (const Particle &particle, mParticles) {
            Vector4 origin = oldWorld.mapPosition(particle.position);
            renderStates.setWorldMatrix(Matrix4::translation(origin.x(), origin.y(), origin.z()));

            // Bind uniforms
            for (int j = 0; j < pass.uniforms.size(); ++j) {
                pass.uniforms[j]->bind();
            }

            glUniform4f(materialColorLoc, particle.colorRed / 255.0f,
                        particle.colorGreen / 255.0f,
                        particle.colorBlue / 255.0f,
                        particle.colorAlpha / 255.0f);
            glUniform1f(rotationLoc, particle.rotationYaw);

            float d = particle.scale / 100.0 * 128;
            glBegin(GL_QUADS);
            switch (mParticleType) {
            case Type_Sprite:
                glVertexAttrib2f(texAttrib, 0, 0);
                glVertexAttrib4fv(posAttrib, Vector4(d/2, d, -d/2, 1).data());
                glVertexAttrib2f(texAttrib, 0, 1);
                glVertexAttrib4fv(posAttrib, Vector4(d/2, -d, -d/2, 1).data());
                glVertexAttrib2f(texAttrib, 1, 1);
                glVertexAttrib4fv(posAttrib, Vector4(-d/2, -d, d/2, 1).data());
                glVertexAttrib2f(texAttrib, 1, 0);
                glVertexAttrib4fv(posAttrib, Vector4(-d/2, d, d/2, 1).data());
                break;
            case Type_Disc:
                glVertexAttrib2f(texAttrib, 0, 0);
                glVertexAttrib4fv(posAttrib, Vector4(d, 0, 0, 1).data());
                glVertexAttrib2f(texAttrib, 0, 1);
                glVertexAttrib4fv(posAttrib, Vector4(0, 0, d, 1).data());
                glVertexAttrib2f(texAttrib, 1, 1);
                glVertexAttrib4fv(posAttrib, Vector4(-d, 0, 0, 1).data());
                glVertexAttrib2f(texAttrib, 1, 0);
                glVertexAttrib4fv(posAttrib, Vector4(0, 0, -d, 1).data());
                break;
            }
            glEnd();
        }

        renderStates.setWorldMatrix(oldWorld);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        pass.program->unbind();
    }

    class ParticleSystemData : public AlignedAllocation
    {
    public:
        ParticleSystemData(const QString &_id) : id(_id), dead(false)
        {
        }

        ~ParticleSystemData() {
            qDeleteAll(emitters);
        }

        QString id;
        QList<Emitter*> emitters;
        Box3d boundingBox;
        bool dead;
        QPointer<ModelInstance> modelInstance;
    };

    void ParticleSystem::addEmitter(Emitter *emitter)
    {
        d->emitters.append(emitter);
    }

    static QHash<QString, QWeakPointer<Texture> > spriteCache;

    SharedTexture loadTexture(const QString &filename) {

        if (spriteCache.contains(filename.toLower())) {
            SharedTexture cachedResult = SharedTexture(spriteCache[filename.toLower()]);
            if (cachedResult) {
                return cachedResult;
            }
        }

        QFile f(filename);
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning("Unable to open texture: %s.", qPrintable(filename));
            return SharedTexture(0);
        }

        SharedTexture t(new Texture);
        if (!t->loadTga(f.readAll())) {
            qWarning("Unable to read texture: %s.", qPrintable(filename));
            return SharedTexture(0);
        }

        t->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glBindTexture(GL_TEXTURE_2D, 0);

        spriteCache[filename.toLower()] = QWeakPointer<Texture>(t);

        return t;
    }

    ParticleSystem::ParticleSystem(const QString &id)
        : d(new ParticleSystemData(id))
    {
    }

    ParticleSystem::~ParticleSystem()
    {
    }

    void ParticleSystem::setModelInstance(ModelInstance *modelInstance)
    {
        d->modelInstance = modelInstance;
    }

    ModelInstance *ParticleSystem::modelInstance() const
    {
        return d->modelInstance;
    }

    void ParticleSystem::elapseTime(float seconds)
    {
        if (d->dead)
            return;

        float timeUnits = seconds / ParticlesTimeUnit;

        bool allEmittersDead = true;

        foreach (Emitter *emitter, d->emitters) {
            emitter->elapseTime(timeUnits);
            allEmittersDead &= emitter->isDead();
        }

        if (allEmittersDead) {
            d->dead = true;
            emit finished();

            // TODO: Properly implement this. And/or check if the system is really deleted
            if (mParentNode) {
                mParentNode->detachObject(this);
            }
        }
    }

    bool ParticleSystem::isDead() const
    {
        return d->dead;
    }

    void ParticleSystem::render(RenderStates &renderStates) {
        bool allDead = true;

        foreach (Emitter *emitter, d->emitters) {
            emitter->render(renderStates);
            allDead &= emitter->isDead();
        }

        if (allDead && mParentNode) {
            mParentNode->detachObject(this);
        }
    }

    const Box3d &ParticleSystem::boundingBox()
    {
        return d->boundingBox;
    }

    //////////////////////////////////////////////////////////////////////////
    // Templates
    //////////////////////////////////////////////////////////////////////////

    class EmitterTemplate {
    public:
        // Typedef a scoped property
        typedef QSharedPointer<ParticleProperty<float> > Property;

        bool loadFromXml(const QDomElement &emitterNode);

        bool readPosition(const QDomElement &element);
        bool readParticles(const QDomElement &element);

        QString mName;
        float mLifespan;
        ParticleBlendMode mBlendMode;
        SpaceType mEmitterSpace;
        QString mBoneName; // For mEmitterSpace == Space_Bone
        Property mScale;
        float mDelay;

        static const Property ZeroProperty;
        Property mPositionX;
        Property mPositionY;
        Property mPositionZ;

        // Required particle properties
        float mParticleSpawnRate;
        float mParticleLifespan;
        QString mParticleTexture;
        QString mParticleModel;

        // A velocity is optional (each component is), but the values should be zero in case they're unused
        Property mParticleVelocityX;
        Property mParticleVelocityY;
        Property mParticleVelocityZ;
        CoordinateType mParticleVelocityType;

        // Particle acceleration affects velocity
        Property mParticleAccelerationX;
        Property mParticleAccelerationY;
        Property mParticleAccelerationZ;

        // Particle position deviation. If present this overrides velocity (component-wise)
        Property mParticlePositionX;
        Property mParticlePositionY;
        Property mParticlePositionZ;
        CoordinateType mParticlePositionType;

        // Particle rotation. For sprites, only Yaw matters
        Property mRotationYaw;
        Property mRotationPitch;
        Property mRotationRoll;

        ParticleType mParticleType;

        // Particle color
        Property mColorRed;
        Property mColorGreen;
        Property mColorBlue;
        Property mColorAlpha;
    };

    /**
 * Models a template for particle systems. New particle systems
 * can be created from templates, using their properties.
 */
    class ParticleSystemTemplate {
    public:

        /**
         * Returns templates for the emitters contained in this particle system template.
         */
        const QList<EmitterTemplate> &emitterTemplates() const
        {
            return mEmitterTemplates;
        }

        /**
         * Returns the unique id of this particle system template.
         */
        const QString &id() const
        {
            return mId;
        }

        /**
         * Tries to load the definition of this particle system template from an XML node.
         */
        bool loadFromXml(const QDomElement &element);

    private:
        QList<EmitterTemplate> mEmitterTemplates;
        QString mId;
    };

    const EmitterTemplate::Property EmitterTemplate::ZeroProperty(new ConstantParticleProperty<float>(0));

    bool EmitterTemplate::loadFromXml(const QDomElement &element)
    {
        if (element.nodeName() != "emitter") {
            qWarning("Node name of emitter element must be emitter.");
            return false;
        }

        bool ok;

        mName = element.attribute("name");
        if (element.hasAttribute("lifespan")) {
            mLifespan = element.attribute("lifespan").toFloat(&ok); // This seems to be really "fixed" length, no variations
            if (!ok) {
                qWarning("Invalid lifetime for emitter template: %s", qPrintable(element.attribute("lifespan")));
                return false;
            }
        } else {
            mLifespan = std::numeric_limits<float>::infinity();
        }

        mDelay = element.attribute("delay", "0").toFloat(&ok);
        if (!ok) {
            qWarning("Invalid delay for emitter template: %s", qPrintable(element.attribute("delay")));
            return false;
        }

        QString space = element.attribute("space", "world").toLower();

        if (space == "bones") {
            mEmitterSpace = Space_RandomBone;
        } else if (space == "world") {
            mEmitterSpace = Space_World;
        } else if (space == "node pos") {
            mEmitterSpace = Space_Bone;
            mBoneName = element.attribute("spaceNode");
        } else if (space == "node ypr") {
            mEmitterSpace = Space_Bone;
            mBoneName = element.attribute("spaceNode");
        } else {
            // TODO: Implement them all
            mEmitterSpace = Space_World;
        }

        QString blendMode = element.attribute("blendMode", "add").toLower();

        if (blendMode == "add") {
            mBlendMode = Blend_Add;
        } else if (blendMode == "subtract") {
            mBlendMode = Blend_Subtract;
        } else if (blendMode == "blend") {
            mBlendMode = Blend_Blend;
        } else {
            mBlendMode = Blend_Add;
            qWarning("Unknown blend mode: %s", qPrintable(blendMode));
            return false;
        }

        // Scaling (first)
        QDomElement scaleElement = element.firstChildElement("scale");
        if (!scaleElement.isNull()) {
            QString scale = element.text();
            mScale = Property(propertyFromString(scale));
            if (!mScale) {
                mScale = ZeroProperty;
                return false;
            }
        }

        // Emitter Position (x,y,z)
        mPositionX = ZeroProperty;
        mPositionY = ZeroProperty;
        mPositionZ = ZeroProperty;

        QDomElement position = element.firstChildElement("position");
        if (!position.isNull()) {
            if (!readPosition(position))
                return false;
        }

        // Particle properties
        QDomElement particles = element.firstChildElement("particles");

        if (particles.isNull()) {
            qWarning("A particles element is required.");
            return false;
        }

        if (!readParticles(particles)) {
            return false;
        }

        return true;

    }

    bool EmitterTemplate::readPosition(const QDomElement &position)
    {
        if (position.hasAttribute("x")) {
            mPositionX = Property(propertyFromString(position.attribute("x")));
            if (!mPositionX) {
                mPositionX = ZeroProperty;
                return false;
            }
        }
        if (position.hasAttribute("y")) {
            mPositionY = Property(propertyFromString(position.attribute("y")));
            if (!mPositionY) {
                mPositionY = ZeroProperty;
                return false;
            }
        }
        if (position.hasAttribute("z")) {
            mPositionZ = Property(propertyFromString(position.attribute("z")));
            if (!mPositionZ) {
                mPositionZ = ZeroProperty;
                return false;
            }
        }

        return true;
    }

    inline static bool readVector(const QDomElement &element,
                                  EmitterTemplate::Property &x, EmitterTemplate::Property &y, EmitterTemplate::Property &z) {
        if (element.hasAttribute("x")) {
            x = EmitterTemplate::Property(propertyFromString(element.attribute("x")));
            if (!x)
                return false;
        }
        if (element.hasAttribute("y")) {
            y = EmitterTemplate::Property(propertyFromString(element.attribute("y")));
            if (!y)
                return false;
        }
        if (element.hasAttribute("z")) {
            z = EmitterTemplate::Property(propertyFromString(element.attribute("z")));
            if (!z)
                return false;
        }
        return true;
    }

    bool EmitterTemplate::readParticles(const QDomElement &element)
    {
        bool ok;

        QString type = element.attribute("type", "sprite").toLower();

        if (type == "sprite") {
            mParticleType = Type_Sprite;
        } else if (type == "disc") {
            mParticleType = Type_Disc;
        } else if (type == "model") {
            mParticleType = Type_Model;
        } else if (type == "point") {
            mParticleType = Type_Point;
        } else {
            qWarning("Invalid particle type: %s.", qPrintable(type));
            return false;
        }

        mParticleSpawnRate = element.attribute("rate").toFloat(&ok);

        if (!ok) {
            qWarning("Emitter has invalid particle spawn rate: %s.", qPrintable(element.attribute("rate")));
            return false;
        }

        if (element.hasAttribute("lifespan")) {
            mParticleLifespan = element.attribute("lifespan").toFloat(&ok);

            if (!ok) {
                qWarning("Emitter has invalid particle lifetime: %s.", qPrintable(element.attribute("lifespan")));
                return false;
            }
        } else {
            mParticleLifespan = std::numeric_limits<float>::infinity();
        }

        if (element.hasAttribute("material")) {
            mParticleTexture = element.attribute("material");
        }

        if (element.hasAttribute("model")) {
            mParticleModel = element.attribute("model");
        }

        mParticleVelocityType = Cartesian;
        mParticlePositionType = Cartesian;

        for (QDomElement child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {

            if (child.nodeName() == "velocity") {
                if (!readVector(child, mParticleVelocityX, mParticleVelocityY, mParticleVelocityZ))
                    return false;

                QString type = child.attribute("coordinates", "cartesian");
                if (type == "cartesian")
                    mParticleVelocityType = Cartesian;
                else if (type == "polar")
                    mParticleVelocityType = Polar;
                else {
                    qWarning("Invalid coordinate type: %s.", qPrintable(type));
                    return false;
                }

            } else if (child.nodeName() == "acceleration") {
                if (!readVector(child, mParticleAccelerationX, mParticleAccelerationY, mParticleAccelerationZ))
                    return false;
            } else if (child.nodeName() == "position") {
                if (!readVector(child, mParticlePositionX, mParticlePositionY,mParticlePositionZ))
                    return false;

                QString type = child.attribute("coordinates", "cartesian");
                if (type == "cartesian")
                    mParticlePositionType = Cartesian;
                else if (type == "polar")
                    mParticlePositionType = Polar;
                else {
                    qWarning("Invalid coordinate type: %s.", qPrintable(type));
                    return false;
                }

            } else if (child.nodeName() == "rotation") {
                if (child.hasAttribute("yaw")) {
                    mRotationYaw = Property(propertyFromString(child.attribute("yaw")));
                    if (!mRotationYaw)
                        return false;
                }
                if (child.hasAttribute("pitch")) {
                    mRotationPitch = Property(propertyFromString(child.attribute("pitch")));
                    if (!mRotationPitch)
                        return false;
                }
                if (child.hasAttribute("roll")) {
                    mRotationRoll = Property(propertyFromString(child.attribute("roll")));
                    if (!mRotationRoll)
                        return false;
                }
            } else if (child.nodeName() == "scale") {
                mScale = Property(propertyFromString(child.text()));
                if (!mScale)
                    return false;
            } else if (child.nodeName() == "color") {
                mColorRed = Property(propertyFromString(child.attribute("red", "255")));
                mColorGreen = Property(propertyFromString(child.attribute("green", "255")));
                mColorBlue = Property(propertyFromString(child.attribute("blue", "255")));
                mColorAlpha = Property(propertyFromString(child.attribute("alpha", "255")));

                if (!mColorRed || !mColorBlue || !mColorGreen || !mColorAlpha)
                    return false;
            }
        }

        return true;
    }

    bool ParticleSystemTemplate::loadFromXml(const QDomElement &element)
    {
        if (element.nodeName() != "particleSystem") {
            qWarning("Name of particle system nodes must be particleSystem.");
            return false;
        }

        mId = element.attribute("id");

        if (mId.isNull()) {
            qWarning("Particle system is missing id.");
            return false;
        }

        QDomElement emitterNode = element.firstChildElement("emitter");
        while (!emitterNode.isNull()) {
            EmitterTemplate emitterTemplate;

            if (emitterTemplate.loadFromXml(emitterNode)) {
                mEmitterTemplates.append(emitterTemplate);
            }

            emitterNode = emitterNode.nextSiblingElement("emitter");
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    // Active Particle Systems
    //////////////////////////////////////////////////////////////////////////

    class ParticleSystemsData {
    public:
        ParticleSystemsData(Models *_models, Materials *_materials)
            : models(_models), materials(_materials)
        {
            mSpriteMaterial = materials->load(":/material/sprite_material.xml");
        }

        bool loadTemplates()
        {
            QElapsedTimer timer;
            timer.start();

            error.clear();

            QFile templatesFile("particles/templates.xml");

            if (!templatesFile.open(QIODevice::ReadOnly)) {
                error.append("Unable to open particle systems template file.");
                return false;
            }

            QDomDocument document;
            int errorLine;
            QString errorMsg;
            if (!document.setContent(&templatesFile, false, &errorMsg, &errorLine)) {
                error.append(QString("XML error while loading particle system template file: %1 on line %2.")
                             .arg(errorMsg).arg(errorLine));
                return false;
            }

            QDomElement rootElement = document.documentElement();

            for (QDomElement child = rootElement.firstChildElement("particleSystem"); !child.isNull();
            child = child.nextSiblingElement("particleSystem"))
            {
                ParticleSystemTemplate tpl;
                if (!tpl.loadFromXml(child)) {
                    error.append(QString("Unable to load partsys id %1.").arg(child.attribute("id")));
                    continue;
                }

                templates.insert(tpl.id().toLower(), tpl);
            }

            qDebug("Loaded %d particle systems in %d ms.", templates.size(), timer.elapsed());
            return true;
        }

        /**
         * Instantiates this template and creates an emitter.
         */
        Emitter *instantiate(ParticleSystem *particleSystem, const EmitterTemplate &tpl) const;

        /**
         * Creates a particle system from this template.
         */
        ParticleSystem *instantiate(const ParticleSystemTemplate &tpl) const;

        Models *models;

        Materials *materials;

        SharedMaterialState mSpriteMaterial;

        QHash<QString,ParticleSystemTemplate> templates;
        QString error;
    };

    ParticleSystems::ParticleSystems(Models *models, Materials *materials)
        : d(new ParticleSystemsData(models, materials))
    {
    }

    ParticleSystems::~ParticleSystems()
    {
    }

    Emitter *ParticleSystemsData::instantiate(ParticleSystem *particleSystem, const EmitterTemplate &tpl) const
    {
        Emitter *emitter = new Emitter(particleSystem, tpl.mParticleSpawnRate, tpl.mParticleLifespan);
        emitter->setName(tpl.mName);
        emitter->setEmitterSpace(tpl.mEmitterSpace);
        emitter->setColor(tpl.mColorRed.data(), tpl.mColorGreen.data(), tpl.mColorBlue.data(), tpl.mColorAlpha.data());
        emitter->setScale(tpl.mScale.data());
        emitter->setLifetime(tpl.mLifespan);
        emitter->setTexture(loadTexture(tpl.mParticleTexture));
        emitter->setPosition(tpl.mPositionX.data(), tpl.mPositionY.data(), tpl.mPositionZ.data());
        emitter->setRotation(tpl.mRotationYaw.data(), tpl.mRotationPitch.data(), tpl.mRotationRoll.data());
        emitter->setAcceleration(tpl.mParticleAccelerationX.data(), tpl.mParticleAccelerationY.data(),
                                 tpl.mParticleAccelerationZ.data());
        emitter->setParticleVelocity(tpl.mParticleVelocityX.data(), tpl.mParticleVelocityY.data(),
                                     tpl.mParticleVelocityZ.data(), tpl.mParticleVelocityType);
        emitter->setParticlePosition(tpl.mParticlePositionX.data(), tpl.mParticlePositionY.data(),
                                     tpl.mParticlePositionZ.data(), tpl.mParticlePositionType);
        emitter->setParticleType(tpl.mParticleType);
        emitter->setBlendMode(tpl.mBlendMode);
        emitter->setBoneName(tpl.mBoneName);

        if (tpl.mParticleType == Type_Model) {
            emitter->setModel(models->load(tpl.mParticleModel));
        } else {
            emitter->setMaterial(mSpriteMaterial);
        }

        return emitter;
    }

    ParticleSystem *ParticleSystemsData::instantiate(const ParticleSystemTemplate &tpl) const
    {
        ParticleSystem *result = new ParticleSystem(tpl.id());

        foreach (const EmitterTemplate &emitterTemplate, tpl.emitterTemplates()) {
            result->addEmitter(instantiate(result, emitterTemplate));
        }

        return result;
    }

    bool ParticleSystems::loadTemplates()
    {
        return d->loadTemplates();
    }

    const QString &ParticleSystems::error() const
    {
        return d->error;
    }

    ParticleSystem *ParticleSystems::instantiate(const QString &name)
    {
        QHash<QString,ParticleSystemTemplate>::const_iterator it = d->templates.find(name.toLower());

        if (it == d->templates.end()) {
            qWarning("Unknown particle systems: %s.", qPrintable(name));
            return NULL;
        } else {
            return d->instantiate(it.value());
        }
    }

}
