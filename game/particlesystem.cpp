
#include <GL/glew.h>

#include <QtCore/QFile>
#include <QtCore/QStringList>

#include <QtCore/QList>
#include <QtCore/QWeakPointer>
#include <QtCore/QTime>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtCore/QMutex>

#include <QtXml/QDomElement>

#include "particlesystem.h"
#include "renderstates.h"
#include "texture.h"
#include "material.h"
#include "materialstate.h"
#include "modelinstance.h"

namespace EvilTemple {

const static float ParticlesTimeUnit = 1 / 30.0f; // All time-based values are in relation to this base value

enum ParticleType {
	Sprite,
	Disc,
	Model
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
    Space_Bones,
};

/**
 * Models a property of an emitter or particle, which can assume one of the following roles:
 * - Fixed value
 * - Random value
 * - Animated value
 */
template<typename T> class ParticleProperty {
public:
	/**
	 * Returns the value of the property, given the life-time of the particle expressed as a range of [0,1].
	 */
	virtual T operator()(float ratio) const = 0;

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

	T operator()(float ratio) const
	{
        Q_UNUSED(ratio)
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
	T operator()(float ratio) const
	{
        Q_UNUSED(ratio);
		return 20;
	}

	bool isAnimated() const
	{
		return false;
	}
};

/**
  * Models a random particle property that will pick a random value when queried from a range (uniform distribution).
  */
template<typename T> class RandomParticleProperty : public ParticleProperty<T> {
public:
	/**
	 * Constructs a random particle property with a minimum and maximum value (both inclusive).
	 */
	RandomParticleProperty(T minValue, T maxValue) : mMinValue(minValue), mSpan(maxValue - mMinValue)
	{
	}

	T operator()(float ratio) const
	{
        Q_UNUSED(ratio)
		return mMinValue + (rand() / (float)RAND_MAX) * mSpan;
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

		return new RandomParticleProperty<float>(minValue, maxValue);
	}

private:
	T mMinValue;
	T mSpan;
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

    T operator()(float ratio) const {
		if (mValues.size() == 1) {
			return mValues[0];
		}

        // Clamp to [0,1]
        ratio = qMin<float>(1, qMax<float>(0, ratio));

        int index = floor(ratio * (mValues.size() - 1));
        int nextIndex = ceil(ratio * (mValues.size() - 1));

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

inline ParticleProperty<float> *propertyFromString(const QString &string)
{
	ParticleProperty<float> *result = NULL;

	if (string.contains('?')) {
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


class Particle {
public:
	Particle() : rotationYaw(0), rotationPitch(0), rotationRoll(0),
		colorRed(255), colorGreen(255), colorBlue(255), colorAlpha(255),
		scale(100),
		accelerationX(0), accelerationY(0), accelerationZ(0),
		velocityX(0), velocityY(0), velocityZ(0)
	{
	}

    Vector4 position;
    float rotationYaw, rotationPitch, rotationRoll;
	float colorRed, colorGreen, colorBlue, colorAlpha;
	float accelerationX, accelerationY, accelerationZ;
	float velocityX, velocityY, velocityZ;
	float scale;
    float expireTime;
    float startTime;
};

class Emitter {
public:

	typedef const ParticleProperty<float> *Property;

	Emitter(float spawnRate, float particleLifetime) 
		: mPartialSpawnedParticles(0), mElapsedTime(0), mExpired(false), 
		mSpawnRate(1/(spawnRate *ParticlesTimeUnit)), mParticleLifetime(particleLifetime), mLifetime(std::numeric_limits<float>::infinity()),
        mEmitterSpace(Space_World)
	{
	}

    void elapseTime(float timeUnits);

    /**
      Spawns a single particle
      */
    void spawnParticle();

	void updateParticles(float elapsedTimeUnits);

	void updateParticle(Particle &particle, float elapsedTimeUnits);

    void render(RenderStates &renderStates, MaterialState *material, const ModelInstance *modelInstance) const;

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

	void setName(const QString &name)
	{
		mName = name;
	}

private:

	Vector4 polarToCartesian(float x, float y, float z);

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

	QString mName;

	// Current position of this emitter
    Property mPositionX, mPositionY, mPositionZ;

    // All particles of an emitter use the same material
	SharedTexture mTexture;

	ParticleBlendMode mBlendMode;
    bool mExpired; // More time elapsed than this emitter's lifetime
    float mElapsedTime;
    float mParticleLifetime; // Lifetime of newly created particles in time units
    float mLifetime; // Number of time units until this emitter stops working. Can be Infinity.
    float mSpawnRate; // One particle per this many time units is spawned
    float mPartialSpawnedParticles; // If the elapsed time is not enough to spawn another particle, it is accumulated.
	
	Q_DISABLE_COPY(Emitter);
};

inline Vector4 Emitter::polarToCartesian(float phi, float theta, float r)
{
	/*
		The formula is taken from http://en.wikipedia.org/wiki/Spherical_coordinate_system
		With coordinates swapped according to the ToEE system.
		In addition, we're using inclination from the reference plane, so cos(theta) needs to be subtracted from 1.
	*/
	float newZ = r * (1 - std::sin(deg2rad(theta))) * std::cos(deg2rad(phi));
	float newX = r * (1 - std::sin(deg2rad(theta))) * std::sin(deg2rad(phi));
	float newY = r * (1 - std::cos(deg2rad(theta)));

	return Vector4(newX, newY, newZ, 0);
}

void Emitter::updateParticles(float elapsedTimeunits)
{
	for (int i = 0; i < mParticles.size(); ++i) {
		updateParticle(mParticles[i], elapsedTimeunits);
	}
}

void Emitter::spawnParticle()
{
	Particle particle;

	if (mRotationYaw) {
		particle.rotationYaw = (*mRotationYaw)(0);
	}
	if (mRotationPitch) {
		particle.rotationPitch = (*mRotationPitch)(0);
	}
	if (mRotationRoll) {
		particle.rotationRoll = (*mRotationRoll)(0);
	}

	if (mAccelerationX) {
		particle.accelerationX = (*mAccelerationX)(0);
	}
	if (mAccelerationY) {
		particle.accelerationY = (*mAccelerationY)(0);
	}
	if (mAccelerationZ) {
		particle.accelerationZ = (*mAccelerationZ)(0);
	}

	if (mParticleVelocityX) {
		particle.velocityX = (*mParticleVelocityX)(0);
	}
	if (mParticleVelocityY) {
		particle.velocityY = (*mParticleVelocityY)(0);
	}
	if (mParticleVelocityZ) {
		particle.velocityZ = (*mParticleVelocityZ)(0);
	}

	if (mScale) {
		particle.scale = (*mScale)(0);
	}

	if (mColorRed) {
		particle.colorRed = (*mColorRed)(0);
	}
	if (mColorGreen) {
		particle.colorGreen = (*mColorGreen)(0);
	}
	if (mColorBlue) {
		particle.colorBlue = (*mColorBlue)(0);
	}
	if (mColorAlpha) {
		particle.colorAlpha = (*mColorAlpha)(0);
	}

	Vector4 positionOffset(0, 0, 0, 0);
	if (mParticlePositionX) {
		positionOffset.setX((*mParticlePositionX)(0));
	}
	if (mParticlePositionY) {
		positionOffset.setY((*mParticlePositionY)(0));
	}
	if (mParticlePositionZ) {
		positionOffset.setZ((*mParticlePositionZ)(0));
	}
	// Convert to cartesian if necessary
	if (mParticlePositionType == Polar) {
		positionOffset = polarToCartesian(positionOffset.x(), positionOffset.y(), positionOffset.z());
	}
	particle.position = Vector4((*mPositionX)(0), (*mPositionY)(0), (*mPositionZ)(0), 1) + positionOffset;

	particle.startTime = mElapsedTime;
	particle.expireTime = mElapsedTime + mParticleLifetime;
	mParticles.append(particle);
}

void Emitter::updateParticle(Particle &particle, float elapsedTimeUnits)
{
	float particleElapsed = mElapsedTime - particle.startTime;
	float particleLifetime = particle.expireTime - particle.startTime;

	float lifecycle = particleElapsed / particleLifetime;

	if (mRotationYaw && mRotationYaw->isAnimated()) {
		particle.rotationYaw = (*mRotationYaw)(lifecycle);
	}
	if (mRotationPitch && mRotationPitch->isAnimated()) {
		particle.rotationPitch = (*mRotationPitch)(lifecycle);
	}
	if (mRotationRoll && mRotationRoll->isAnimated()) {
		particle.rotationRoll = (*mRotationRoll)(lifecycle);
	}

	if (mScale && mScale->isAnimated()) {
		particle.scale = (*mScale)(lifecycle);
	}

	if (mColorRed && mColorRed->isAnimated()) {
		particle.colorRed = (*mColorRed)(lifecycle);
	}
	if (mColorGreen && mColorGreen->isAnimated()) {
		particle.colorGreen = (*mColorGreen)(lifecycle);
	}
	if (mColorBlue && mColorBlue->isAnimated()) {
		particle.colorBlue = (*mColorBlue)(lifecycle);
	}
	if (mColorAlpha && mColorAlpha->isAnimated()) {
		particle.colorAlpha = (*mColorAlpha)(lifecycle);
	}

	if (mAccelerationX && mAccelerationX->isAnimated()) {
		particle.accelerationX = (*mAccelerationX)(lifecycle);
	}
	if (mAccelerationY && mAccelerationY->isAnimated()) {
		particle.accelerationY = (*mAccelerationY)(lifecycle);
	}
	if (mAccelerationZ && mAccelerationZ->isAnimated()) {
		particle.accelerationZ = (*mAccelerationZ)(lifecycle);
	}

	// Increase velocity according to acceleration or animate it using keyframes	
	if (mParticleVelocityX && mParticleVelocityX->isAnimated()) {
		particle.velocityX = (*mParticleVelocityX)(lifecycle);
	} else {
		particle.velocityX += elapsedTimeUnits * particle.accelerationX * ParticlesTimeUnit;
	}
	if (mParticleVelocityY && mParticleVelocityY->isAnimated()) {
		particle.velocityY = (*mParticleVelocityY)(lifecycle);
	} else {
		particle.velocityY += elapsedTimeUnits * particle.accelerationY * ParticlesTimeUnit;
	}
	if (mParticleVelocityZ && mParticleVelocityZ->isAnimated()) {
		particle.velocityZ = (*mParticleVelocityZ)(lifecycle);
	} else {
		particle.velocityZ += elapsedTimeUnits * particle.accelerationZ * ParticlesTimeUnit;
	}

	if (mParticleVelocityType == Polar) {
		float r = particle.position.length();
		if (r != 0) {
			Vector4 rotAxis;
			if (particle.position.x() != 0 || particle.position.z() == 0) {
				Vector4 rotNormal(0, 1, 0, 0);
				rotAxis = rotNormal.cross(particle.position).normalized();
			} else {
				// In case the point is above the origin (x=0,y=0), use the x axis as a rotation axis, doesn't matter.
				rotAxis = Vector4(1, 0, 0, 0);
			}

			Quaternion rot1 = Quaternion::fromAxisAndAngle(rotAxis.x(), rotAxis.y(), rotAxis.z(), deg2rad(particle.velocityY));
			Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(particle.velocityX));
			particle.position = Matrix4::rotation(rot2) * Matrix4::rotation(rot1) * particle.position;

			// Extrude the position outwards
			Vector4 direction = particle.position;
			direction.setW(0);
			particle.position += (particle.velocityZ / direction.length()) * direction;
		}		
	} else {
		particle.position += Vector4(particle.velocityX, particle.velocityY, particle.velocityZ, 0) 
			* elapsedTimeUnits * ParticlesTimeUnit;
	}

	// An animated position overrides any velocity calculations
	if (mParticlePositionX && mParticlePositionX->isAnimated()) {
		particle.position.setX(particle.position.x() + (*mParticlePositionX)(lifecycle));
	}
	if (mParticlePositionY && mParticlePositionY->isAnimated()) {
		particle.position.setY(particle.position.y() + (*mParticlePositionY)(lifecycle));
	}
	if (mParticlePositionZ && mParticlePositionZ->isAnimated()) {
		particle.position.setZ(particle.position.z() + (*mParticlePositionZ)(lifecycle));
	}

}

void Emitter::elapseTime(float timeUnits) {
	Q_ASSERT(mSpawnRate > 0);

	mElapsedTime += timeUnits;

	// Check for expired particles
	QList<Particle>::iterator it = mParticles.begin();
	while (it != mParticles.end()) {
		if (it->expireTime < mElapsedTime)
			it = mParticles.erase(it);
		else
			++it;
	}
	
	// Spawn new particles
	if (!mExpired) {
		float remainingSpawnTime = timeUnits;
		remainingSpawnTime += mPartialSpawnedParticles;

		while (remainingSpawnTime > mSpawnRate) {
			spawnParticle();
			remainingSpawnTime -= mSpawnRate;
		}

		mPartialSpawnedParticles = remainingSpawnTime;

		if (mElapsedTime > mLifetime) {
			mExpired = true;
		}
	}

	updateParticles(timeUnits);
	// TODO: Transform particles
}


void Emitter::render(RenderStates &renderStates, MaterialState *material, const ModelInstance *modelInstance) const
{
	MaterialPassState &pass = material->passes[0];

	int posAttrib = pass.program.attributeLocation("vertexPosition");
	int texAttrib = pass.program.attributeLocation("vertexTexCoord");
	int rotationLoc = pass.program.uniformLocation("rotation");
	int materialColorLoc = pass.program.uniformLocation("materialColor");

	pass.program.bind();			

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
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case Blend_Subtract:
		glBlendColor(-1, -1, -1, -1);
		glBlendFunc(GL_CONSTANT_COLOR, GL_ONE);
		break;
	}

	Matrix4 oldWorld = renderStates.worldMatrix();

    Matrix4 emitterWorld = oldWorld;
    
    // If we have "bones" as the space
    if (modelInstance && mEmitterSpace == Space_Bones) {
        Matrix4 flipZ;
        flipZ.setToIdentity();
        flipZ(2,2) *= -1;

        // Choose a bone at random (?)

        //emitterWorld = emitterWorld * flipZ * modelInstance->getBoneSpace("firer_ref1") * flipZ;
    }
	
	foreach (const Particle &particle, mParticles) {
        Vector4 origin = emitterWorld * particle.position;
		/*renderStates.setWorldMatrix(emitterWorld * Matrix4::translation(particle.position.x(),
			particle.position.y(),
			particle.position.z()));*/
        renderStates.setWorldMatrix(Matrix4::translation(origin.x(), origin.y(), origin.z()));

		// Bind uniforms
		for (int j = 0; j < pass.uniforms.size(); ++j) {
			pass.uniforms[j].bind();
		}
				
		glUniform4f(materialColorLoc, particle.colorRed / 255.0f, 
			particle.colorGreen / 255.0f, 
			particle.colorBlue / 255.0f,
			particle.colorAlpha / 255.0f);
		glUniform1f(rotationLoc, particle.rotationYaw);

		float d = particle.scale;
		glBegin(GL_QUADS);
		switch (mParticleType) {
		case Sprite:
			glVertexAttrib2f(texAttrib, 0, 0);
			glVertexAttrib4fv(posAttrib, Vector4(d/2, d, -d/2, 1).data());
			glVertexAttrib2f(texAttrib, 0, 1);
			glVertexAttrib4fv(posAttrib, Vector4(d/2, -d, -d/2, 1).data());
			glVertexAttrib2f(texAttrib, 1, 1);
			glVertexAttrib4fv(posAttrib, Vector4(-d/2, -d, d/2, 1).data());
			glVertexAttrib2f(texAttrib, 1, 0);
			glVertexAttrib4fv(posAttrib, Vector4(-d/2, d, d/2, 1).data());
			break;
		case Disc:
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
	glBlendFunc(GL_ONE, GL_ZERO);

	pass.program.unbind();
}

class ParticleSystemData
{
public:
    ParticleSystemData(const QString &_id, const QList<Emitter*> &_emitters) 
		: id(_id), emitters(_emitters)
	{
    }

	~ParticleSystemData() {
		qDeleteAll(emitters);
	}
	
	QString id;
    QList<Emitter*> emitters;
    Vector4 position;
    const ModelInstance *modelInstance;
};

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
		qWarning("Unable to open test texture.");
		return SharedTexture(0);
	}

	SharedTexture t(new Texture);
	if (!t->loadTga(f.readAll())) {
		qWarning("Unable to open texture 2.");
		return SharedTexture(0);
	}

	t->bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	spriteCache[filename.toLower()] = QWeakPointer<Texture>(t);

	return t;
}

ParticleSystem::ParticleSystem(const QString &id, const QList<Emitter*> &emitters) 
	: d(new ParticleSystemData(id, emitters))
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::setModelInstance(const ModelInstance *modelInstance)
{
    d->modelInstance = modelInstance;
}

void ParticleSystem::elapseTime(float timeUnits)
{
	foreach (Emitter *emitter, d->emitters) {
		emitter->elapseTime(timeUnits);
	}
}

void ParticleSystem::elapseSeconds(float seconds)
{
    elapseTime(seconds / ParticlesTimeUnit);
}

void ParticleSystem::setPosition(const Vector4 &position)
{
    d->position = position;
}

const Vector4 &ParticleSystem::position() const
{
    return d->position;
}

void ParticleSystem::render(RenderStates &renderStates, MaterialState *material) const {

	renderStates.setWorldMatrix(Matrix4::translation(d->position.x(), d->position.y(), d->position.z(), 0));

    foreach (Emitter *emitter, d->emitters) {
        emitter->render(renderStates, material, d->modelInstance);
    }

	renderStates.setWorldMatrix(Matrix4::identity());
}

//////////////////////////////////////////////////////////////////////////
// Templates
//////////////////////////////////////////////////////////////////////////

class EmitterTemplate {
public:
	// Typedef a scoped property
	typedef QSharedPointer<ParticleProperty<float> > Property;

	bool loadFromXml(const QDomElement &emitterNode);

	/**
	 * Instantiates this template and creates an emitter.
	 */
	Emitter *instantiate() const;
	
private:
	bool readPosition(const QDomElement &element);
	bool readParticles(const QDomElement &element);

	QString mName;
	float mLifespan;
	ParticleBlendMode mBlendMode;
    SpaceType mEmitterSpace;
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

	/**
	 * Creates a particle system from this template.
	 */
	ParticleSystem *instantiate() const;

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
        mEmitterSpace = Space_Bones;
    } else if (space == "world") {
        mEmitterSpace = Space_World;
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
		mParticleType = Sprite;
	} else if (type == "disc") {
		mParticleType = Disc;
	} else if (type == "model") {
		mParticleType = Model;
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

ParticleSystem *ParticleSystemTemplate::instantiate() const
{
	QList<Emitter*> emitters;
	emitters.reserve(mEmitterTemplates.size());

	foreach (const EmitterTemplate &emitterTemplate, mEmitterTemplates) {
		emitters.append(emitterTemplate.instantiate());
	}

	return new ParticleSystem(mId, emitters);
}

Emitter *EmitterTemplate::instantiate() const
{
	Emitter *emitter = new Emitter(mParticleSpawnRate, mParticleLifespan);
	emitter->setName(mName);
    emitter->setEmitterSpace(mEmitterSpace);
	emitter->setColor(mColorRed.data(), mColorGreen.data(), mColorBlue.data(), mColorAlpha.data());
	emitter->setScale(mScale.data());
	emitter->setLifetime(mLifespan);
	emitter->setTexture(loadTexture(mParticleTexture));
	emitter->setPosition(mPositionX.data(), mPositionY.data(), mPositionZ.data());
	emitter->setRotation(mRotationYaw.data(), mRotationPitch.data(), mRotationRoll.data());
	emitter->setAcceleration(mParticleAccelerationX.data(), mParticleAccelerationY.data(), mParticleAccelerationZ.data());
	emitter->setParticleVelocity(mParticleVelocityX.data(), mParticleVelocityY.data(), mParticleVelocityZ.data(), mParticleVelocityType);
	emitter->setParticlePosition(mParticlePositionX.data(), mParticlePositionY.data(), mParticlePositionZ.data(), mParticlePositionType);
	emitter->setParticleType(mParticleType);
	emitter->setBlendMode(mBlendMode);

	return emitter;
}

//////////////////////////////////////////////////////////////////////////
// Active Particle Systems
//////////////////////////////////////////////////////////////////////////

class ParticleSystemsData;

class ParticleSystemsThread : public QThread {
public:
    ParticleSystemsThread(ParticleSystemsData &data) : mData(data) {
    }

protected:
    void run();

private:
    ParticleSystemsData &mData;
};

class ParticleSystemsData {
public:
    ParticleSystemsData(RenderStates &_renderStates)
        : renderStates(_renderStates), thread(*this), aborted(false)
    {
        thread.start();
    }

    ~ParticleSystemsData() {
        aborted = true;
        thread.wait();
    }

	void loadTemplates() 
	{
		QFile templatesFile("particles/templates.xml");

		if (!templatesFile.open(QIODevice::ReadOnly)) {
			qWarning("Unable to open particle systems template file.");
			return;
		}

		QDomDocument document;
		int errorLine;
		QString errorMsg;
		if (!document.setContent(&templatesFile, false, &errorMsg, &errorLine)) {
			qWarning("XML error while loading particle system template file: %s on line %d.", 
				qPrintable(errorMsg), errorLine);
			return;
		}

		QDomElement rootElement = document.documentElement();

		for (QDomElement child = rootElement.firstChildElement("particleSystem"); !child.isNull();
			child = child.nextSiblingElement("particleSystem")) 
		{
			ParticleSystemTemplate tpl;
			if (!tpl.loadFromXml(child)) {
				qWarning("Unable to load partsys id %s.", qPrintable(child.attribute("id")));
				continue;
			}

			templates.insert(tpl.id(), tpl);
		}

		qDebug("Loaded %d particle systems.", templates.size());
	}

	MaterialState spriteMaterial;
    volatile bool aborted;
    QMutex mutex;
    ParticleSystemsThread thread;
    RenderStates &renderStates;
    QList<ParticleSystem*> particleSystems;

	QHash<QString,ParticleSystemTemplate> templates;
};

void ParticleSystemsThread::run() {
    QTime timer;
    timer.start();

    while (!mData.aborted) {
        float elapsedSeconds = timer.restart() / 1000.0f;

        // Convert elapsed seconds into the internal particle system time unit
        float elapsedTimeUnits = elapsedSeconds / ParticlesTimeUnit;

        mData.mutex.lock();
        foreach (ParticleSystem *partSystem, mData.particleSystems) {
            partSystem->elapseTime(elapsedTimeUnits);
        }

        mData.mutex.unlock();

        msleep(33); // Aim for 30 updates per second
    }
}

ParticleSystems::ParticleSystems(RenderStates &renderState) : d(new ParticleSystemsData(renderState))
{
	d->loadTemplates();

	QFile spriteMaterialFile(":/material/sprite_material.xml");

	if (!spriteMaterialFile.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open sprite material for particle systems.");
		return;
	}

	Material material;
	if (!material.loadFromData(spriteMaterialFile.readAll())) {
		qWarning("Unable to create material.");
		return;
	}

	if (!d->spriteMaterial.createFrom(material, d->renderStates, NULL)) {
		qWarning("Unable to create material state: %s", qPrintable(d->spriteMaterial.error()));
		return;
	}
}

ParticleSystems::~ParticleSystems()
{

}

void ParticleSystems::render()
{
    QMutexLocker locker(&d->mutex);

    foreach (const ParticleSystem *particleSystem, d->particleSystems) {
        particleSystem->render(d->renderStates, &d->spriteMaterial);
    }
}

void ParticleSystems::create(const QString &name, const Vector4 &position)
{
    ParticleSystem *system = d->templates[name].instantiate();
    system->setPosition(position);

    QMutexLocker locker(&d->mutex);
    d->particleSystems.append(system);
}

ParticleSystem *ParticleSystems::instantiate(const QString &name)
{
    return d->templates[name].instantiate();
}

MaterialState *ParticleSystems::spriteMaterial()
{
    return &d->spriteMaterial;
}

}
