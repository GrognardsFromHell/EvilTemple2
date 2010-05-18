
#include <GL/glew.h>

#include <QtCore/QFile>
#include <QtCore/QStringList>

#include <QtCore/QList>
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

namespace EvilTemple {

enum ParticleBlendMode {
	Blend_Add,
	Blend_Subtract,
	Blend_Blend,
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
		return 40;
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

/**
  Models a particle system in world space and it's emitters.
  */
class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem();

    void setPosition(const Vector4 &position);
    const Vector4 &position() const;

    void elapseTime(float seconds);

    void render(RenderStates &renderStates, MaterialState *material) const;
	
private:
    QScopedPointer<ParticleSystemData> d;
};

const float TimeUnit = 0.1; // All time-based values are in relation to this base value

class Particle {
public:
    Vector4 position;
    float rotation;
    float expireTime;
    float startTime;
};

/**
  Interpolates between evenly spaced key-frames.
  */
template<typename T> class KeyframedValueProvider {
public:
    KeyframedValueProvider(const QVector<T> &values)
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
private:
    float mStep;
    QVector<T> mValues;
};

class Emitter {
public:

	Emitter(float spawnRate, float particleLifetime, const QVector<float> &scale, const QVector<float> &alpha, SharedTexture texture) 
		: mPartialSpawnedParticles(0), mElapsedTime(0), mExpired(false), 
		mSpawnRate(spawnRate), mParticleLifetime(particleLifetime), mTexture(texture), mScale(scale), mAlpha(alpha),
		mMinRotation(0), mMaxRotation(0)
    {
    }

    void elapseTime(float timeUnits)
    {
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
            timeUnits += mPartialSpawnedParticles;

            while (timeUnits > mSpawnRate) {
                spawnParticle();
                timeUnits -= mSpawnRate;
            }

            mPartialSpawnedParticles = timeUnits;

            if (mElapsedTime > mLifetime) {
                mExpired = true;
            }
        }

        // TODO: Transform particles
    }

    /**
      Spawns a single particle
      */
    void spawnParticle()
    {
        Particle particle;
        particle.position = mPosition;
        particle.rotation = mMinRotation + (rand() / (float)RAND_MAX) * (mMaxRotation - mMinRotation);
        particle.startTime = mElapsedTime;
        particle.expireTime = mElapsedTime + mParticleLifetime;
        mParticles.append(particle);
    }

    void render(RenderStates &renderStates, MaterialState *material) const
    {
		MaterialPassState &pass = material->passes[0];

		pass.program.bind();			

		mTexture->bind();

		glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
						
        foreach (const Particle &particle, mParticles) {			
			renderStates.setWorldMatrix(Matrix4::translation(particle.position.x(),
				particle.position.y(),
				particle.position.z()));

			// Bind uniforms
			for (int j = 0; j < pass.uniforms.size(); ++j) {
				pass.uniforms[j].bind();
			}

			float particleElapsed = mElapsedTime - particle.startTime;
			float particleLifetime = particle.expireTime - particle.startTime;

            float lifecycle = particleElapsed / particleLifetime;

            float alpha = mAlpha(lifecycle);

			int loc = pass.program.uniformLocation("materialColor");
			glUniform4f(loc, 1, 0.5, 0.25, alpha);

			loc = pass.program.uniformLocation("rotation");
			glUniform1f(loc, particle.rotation);
			
			int posAttrib = pass.program.attributeLocation("vertexPosition");
			int texAttrib = pass.program.attributeLocation("vertexTexCoord");

            float d = mScale(lifecycle);
            glBegin(GL_QUADS);
            glVertexAttrib2f(texAttrib, 0, 0);
            glVertexAttrib4fv(posAttrib, Vector4(d/2, d, -d/2, 1).data());
            glVertexAttrib2f(texAttrib, 0, 1);
            glVertexAttrib4fv(posAttrib, Vector4(d/2, -d, -d/2, 1).data());
            glVertexAttrib2f(texAttrib, 1, 1);
            glVertexAttrib4fv(posAttrib, Vector4(-d/2, -d, d/2, 1).data());
            glVertexAttrib2f(texAttrib, 1, 0);
			glVertexAttrib4fv(posAttrib, Vector4(-d/2, d, d/2, 1).data());
            glEnd();			
        }

		renderStates.setWorldMatrix(Matrix4::identity());

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);

		pass.program.unbind();
    }

    void setLifetime(float lifetime)
    {
        mLifetime = lifetime;
        if (mElapsedTime > mLifetime) {
            mExpired = true;
        }
    }

    void setPosition(const Vector4 &position)
    {
        mPosition = position;
    }

	void setRotation(float minDegrees, float maxDegrees) {
		mMinRotation = minDegrees;
		mMaxRotation = maxDegrees;
	}

private:

    QList<Particle> mParticles;

    Vector4 mPosition; // Current position of this emitter (should this be dynamic?)

	KeyframedValueProvider<float> mScale;
	KeyframedValueProvider<float> mAlpha;

    // All particles of an emitter use the same material
	SharedTexture mTexture;

    bool mExpired; // More time elapsed than this emitter's lifetime
    float mElapsedTime;
    float mParticleLifetime; // Lifetime of newly created particles in time units
    float mLifetime; // Number of time units until this emitter stops working. Can be Infinity.
    float mSpawnRate; // One particle per this many time units is spawned
    float mPartialSpawnedParticles; // If the elapsed time is not enough to spawn another particle, it is accumulated.

	float mMinRotation;
	float mMaxRotation;
};

class ParticleSystemData
{
public:
    ParticleSystemData() {
    }

	~ParticleSystemData() {
		qDeleteAll(emitters);
	}

    QList<Emitter*> emitters;
    Vector4 position;
};

SharedTexture loadTexture(const QString &filename) {
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

	return t;
}

ParticleSystem::ParticleSystem() : d(new ParticleSystemData)
{
	QVector<float> scale;
	QVector<float> alpha;
	
	scale.append(20);
	alpha.append(64/255.0f);
	Emitter *emitter = new Emitter(30/1, 30, scale, alpha, loadTexture("particles/flare-big.tga"));
	emitter->setLifetime(std::numeric_limits<float>::infinity());
    emitter->setPosition(d->position);
	d->emitters.append(emitter);

	scale.clear();
	alpha.clear();
	foreach (QString p, QString("25,27,26,30,28,26,25,27,26,30,28,26,25,27,26,30,28,26,25,27,26,30,28,26,25,27,26,30,28,26,25,27,26,30,28,26,25,27,26,30,28,26").split(","))
	{
		scale.append(p.toFloat());
	}
	alpha.append(16/255.0f);
	alpha.append(0/255.0f);
	emitter = new Emitter(30/3, 60, scale, alpha, loadTexture("particles/flare-3.tga"));
	emitter->setLifetime(std::numeric_limits<float>::infinity());
    emitter->setPosition(d->position);
	emitter->setRotation(0, 360);
	d->emitters.append(emitter);
	
	scale.clear();
	alpha.clear();
	scale.append(40);
	alpha.append(0/255.0f);
	alpha.append(32/255.0f);
	alpha.append(0/255.0f);
	emitter = new Emitter(30/5, 30, scale, alpha, loadTexture("particles/flare-1.tga"));
	emitter->setLifetime(std::numeric_limits<float>::infinity());
	emitter->setPosition(d->position);
	emitter->setRotation(0, 360);
	d->emitters.append(emitter);
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::elapseTime(float timeUnits)
{
	foreach (Emitter *emitter, d->emitters) {
        emitter->elapseTime(timeUnits);
    }
}

void ParticleSystem::setPosition(const Vector4 &position)
{
    d->position = position;

	foreach (Emitter *emitter, d->emitters) {
		emitter->setPosition(d->position);
	}
}

const Vector4 &ParticleSystem::position() const
{
    return d->position;
}

void ParticleSystem::render(RenderStates &renderStates, MaterialState *material) const {
    foreach (Emitter *emitter, d->emitters) {
        emitter->render(renderStates, material);
    }
}

//////////////////////////////////////////////////////////////////////////
// Templates
//////////////////////////////////////////////////////////////////////////

class EmitterTemplate {
public:
	// Typedef a scoped property
	typedef QSharedPointer<ParticleProperty<float> > Property;

	bool loadFromXml(const QDomElement &emitterNode);
	
private:
	bool readPosition(const QDomElement &element);
	bool readParticles(const QDomElement &element);

	QString mName;
	float mLifespan;
	ParticleBlendMode mBlendMode;
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

	// Particle acceleration affects velocity
	Property mParticleAccelerationX;
	Property mParticleAccelerationY;
	Property mParticleAccelerationZ;

	// Particle position deviation. If present this overrides velocity (component-wise)
	Property mParticlePositionX;
	Property mParticlePositionY;
	Property mParticlePositionZ;

	// Particle rotation. For sprites, only Yaw matters
	Property mRotationYaw;
	Property mRotationPitch;
	Property mRotationRoll;

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
	if (!position.isNull() && !readPosition(position)) {
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

	mParticleSpawnRate = element.attribute("rate").toFloat(&ok);

	if (!ok) {
		qWarning("Emitter has invalid particle spawn rate.");
		return false;
	}

	mParticleLifespan = element.attribute("lifespan").toFloat(&ok);

	if (!ok) {
		qWarning("Emitter has invalid particle lifespan.");
		return false;
	}

	if (element.hasAttribute("material")) {
		mParticleTexture = element.attribute("material");
	}

	for (QDomElement child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
	
		if (child.nodeName() == "velocity") {
			if (!readVector(child, mParticleVelocityX, mParticleVelocityY, mParticleVelocityZ))
				return false;
		} else if (child.nodeName() == "acceleration") {
			if (!readVector(child, mParticleAccelerationX, mParticleAccelerationY, mParticleAccelerationZ))
				return false;
		} else if (child.nodeName() == "position") {
			if (!readVector(child, mParticlePositionX, mParticlePositionY,mParticlePositionZ))
				return false;
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
        float elapsedTimeUnits = elapsedSeconds / TimeUnit;

        mData.mutex.lock();
        foreach (ParticleSystem *partSystem, mData.particleSystems) {
            partSystem->elapseTime(elapsedTimeUnits);
        }

        mData.mutex.unlock();

        msleep(10); // Aim for 30 updates per second
    }
}

ParticleSystems::ParticleSystems(RenderStates &renderState) : d(new ParticleSystemsData(renderState))
{

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

void ParticleSystems::create()
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
	
    ParticleSystem *system = new ParticleSystem;
    system->setPosition(Vector4(13978.1, 75.988, 13036.2, 1));

    QMutexLocker locker(&d->mutex);
    d->particleSystems.append(system);
}

}
