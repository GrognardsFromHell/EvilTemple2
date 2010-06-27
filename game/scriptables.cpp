
#include <QScriptEngine>
#include <QScriptValueIterator>

#include "scriptables.h"
#include "modelfile.h"
#include "particlesystem.h"

namespace EvilTemple {
    typedef QSharedPointer<ModelInstance> SharedModelInstance;
    typedef QSharedPointer<LineRenderable> SharedLineRenderable;
}

using EvilTemple::SharedModel;
using EvilTemple::SharedSceneNode;
using EvilTemple::SharedRenderable;
using EvilTemple::SharedModelInstance;
using EvilTemple::SharedLight;
using EvilTemple::SharedMaterialState;
using EvilTemple::SharedParticleSystem;
using EvilTemple::SharedLineRenderable;

Q_DECLARE_METATYPE(SharedSceneNode)
Q_DECLARE_METATYPE(SharedModel)
Q_DECLARE_METATYPE(SharedRenderable)
Q_DECLARE_METATYPE(SharedModelInstance)
Q_DECLARE_METATYPE(SharedLight)
Q_DECLARE_METATYPE(SharedMaterialState)
Q_DECLARE_METATYPE(SharedParticleSystem)
Q_DECLARE_METATYPE(SharedLineRenderable)

Q_DECLARE_METATYPE(Vector4)
Q_DECLARE_METATYPE(QVector<Vector4>)
Q_DECLARE_METATYPE(Quaternion)
Q_DECLARE_METATYPE(Box3d)

namespace EvilTemple {

static const Vector4 nullVector(0, 0, 0, 0);
static const Quaternion nullRotation(0, 0, 0, 0);
static const Box3d emptyBoundingBox(nullVector, nullVector);
static const SharedModel nullSharedModel;

template<typename T>
QScriptValue valueToScriptValue(QScriptEngine *engine, T const &in)
{
    return engine->newVariant(qVariantFromValue(in));
    // TODO: Do we need to set the prototype object here?
}

template<typename T>
void valueFromScriptValue(const QScriptValue &object, T &out)
{
    out = qvariant_cast<T>(object.toVariant());
}

template<typename T>
void registerValueType(QScriptEngine *engine, const char *name)
{
    qRegisterMetaType<T>(name);
    qScriptRegisterMetaType<T>(engine, valueToScriptValue<T>, valueFromScriptValue<T>);
}

template<typename T>
QScriptValue vectorToScriptValue(QScriptEngine *engine, QVector<T> const &in)
{
    QScriptValue result = engine->newArray(in.size());

    for (size_t i = 0; i < in.size(); ++i) {
        result.setProperty(i, engine->newVariant(qVariantFromValue(in[i])));
    }

    return result;
}

template<typename T>
void vectorFromScriptValue(const QScriptValue &object, QVector<T> &out)
{
    out.clear();

    QScriptValueIterator it(object);

    while (it.hasNext()) {
        it.next();
        out.append(qvariant_cast<T>(it.value().toVariant()));
    }
}

template<typename T>
void registerVectorType(QScriptEngine *engine, const char *name)
{
    qRegisterMetaType< QVector<T> >(name);
    qScriptRegisterMetaType< QVector<T> >(engine, vectorToScriptValue<T>, vectorFromScriptValue<T>);
}

QScriptValue renderableToScriptValue(QScriptEngine *engine, SharedRenderable const &in)
{
    QScriptValue result = engine->newVariant(qVariantFromValue(in));

    // Find correct prototype
    if (!in.objectCast<ModelInstance>().isNull()) {
        qWarning("Need to set type id? Or is this automatic?");
    } else {
        qWarning("Unknown subclass of Renderable encountered.");
    }

    return result;
}

void renderableFromScriptValue(const QScriptValue &object, SharedRenderable &out)
{
    // Try all the known types of renderables
    QVariant variant = object.toVariant();
    int userType = variant.userType();

    if (userType == qMetaTypeId<SharedModelInstance>()) {
        SharedModelInstance modelInstance = qvariant_cast<SharedModelInstance>(variant);
        out = modelInstance.objectCast<Renderable>();
    } else if (userType == qMetaTypeId<SharedLight>()) {
        SharedLight modelInstance = qvariant_cast<SharedLight>(variant);
        out = modelInstance.objectCast<Renderable>();
    } else if (userType == qMetaTypeId<SharedParticleSystem>()) {
        SharedParticleSystem particleSystemInstance = qvariant_cast<SharedParticleSystem>(variant);
        out = particleSystemInstance.objectCast<Renderable>();
    } else if (userType == qMetaTypeId<SharedLineRenderable>()) {
        SharedLineRenderable lineRenderable = qvariant_cast<SharedLineRenderable>(variant);
        out = lineRenderable.objectCast<Renderable>();
    } else {
        qWarning("Unable to convert to SharedRenderable.");
    }
}


/**
  Since Renderable is an abstract class, we try to find the concrete class here and associate
  the corresponding prototype object.
  */
void registerRenderableScriptable(QScriptEngine *engine)
{
    qRegisterMetaType<SharedRenderable>("SharedRenderable");
    qScriptRegisterMetaType<SharedRenderable>(engine, renderableToScriptValue, renderableFromScriptValue);
}

const Box3d &ModelScriptable::boundingBox() const
{
    SharedModel model = data();
    return model ? model->boundingBox() : emptyBoundingBox;
}

float ModelScriptable::radius() const
{
    SharedModel model = data();
    return model ? model->radius() : 0;
}

bool ModelScriptable::hasAnimation(const QString &name) const
{
    SharedModel model = data();
    return model ? model->hasAnimation(name) : false;
}

QScriptValue ModelScriptable::animations() const
{
    SharedModel model = data();

    if (model) {
        QStringList animations = model->animations();
        QScriptValue result = engine()->newArray(animations.size());
        int i = 0;
        foreach (const QString &anim, animations) {
            result.setProperty(i++, QScriptValue(anim));
        }
        return result;
    } else {
        return engine()->undefinedValue();
    }
}

QScriptValue ModelScriptable::animationDps(const QString &name) const
{
    SharedModel model = data();

    if (model) {
        const Animation *animation = model->animation(name);
        if (animation) {
            return QScriptValue(animation->dps());
        } else {
            return engine()->undefinedValue();
        }
    } else {
        return engine()->undefinedValue();
    }
}

float ModelScriptable::radiusSquared() const
{
    SharedModel model = data();
    return model ? model->radiusSquared() : 0;
}

SharedModel ModelScriptable::data() const
{
    SharedModel data = qscriptvalue_cast<SharedModel>(thisObject());
    if (!data)
        context()->throwError("Scene node object is not associated with a valid scene node.");
    return data;
}

void ModelScriptable::registerWith(QScriptEngine *engine)
{
    registerValueType<SharedModel>(engine, "SharedModel");
    int metaId = qMetaTypeId<SharedModel>();
    QScriptValue prototype = engine->newQObject(new ModelScriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);
}

void MaterialStateScriptable::registerWith(QScriptEngine *engine)
{
    registerValueType<SharedMaterialState>(engine, "SharedMaterialState");
}

void ParticleSystemScriptable::registerWith(QScriptEngine *engine)
{
    registerValueType<SharedParticleSystem>(engine, "SharedParticleSystem");
    int metaId = qMetaTypeId<SharedParticleSystem>();
    QScriptValue prototype = engine->newQObject(new ParticleSystemScriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);
}

SharedModelInstance ParticleSystemScriptable::modelInstance() const
{
    return SharedModelInstance(0);
}

void ParticleSystemScriptable::setModelInstance(const SharedModelInstance &sharedModelInstance)
{
    ParticleSystem *particleSystem = data();
    if (particleSystem) {
        particleSystem->setModelInstance(sharedModelInstance.data());
    }
}

ParticleSystem *ParticleSystemScriptable::data() const
{
    SharedRenderable renderable = qscriptvalue_cast<SharedRenderable>(thisObject());

    ParticleSystem *data = renderable.objectCast<ParticleSystem>().data();

    if (!data) {
        context()->throwError("Particle system instance object not associated with a shared renderable.");
        return NULL;
    }

    return data;
}

QScriptValue LineRenderableScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");
    SharedLineRenderable node(new LineRenderable);
    QScriptValue result = engine->newVariant(context->thisObject(), qVariantFromValue(node));
    result.setPrototype(engine->defaultPrototype(qMetaTypeId<SharedLineRenderable>()));
    return result;
}

void LineRenderableScriptable::registerWith(QScriptEngine *engine)
{
    registerValueType<SharedLineRenderable>(engine, "SharedLineRenderable");
    int metaId = qMetaTypeId<SharedLineRenderable>();
    QScriptValue prototype = engine->newQObject(new LineRenderableScriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);

    QScriptValue ctor = engine->newFunction(LineRenderableScriptableCtor);
    engine->globalObject().setProperty("LineRenderable", ctor);
}

void LineRenderableScriptable::addLine(const Vector4 &start, const Vector4 &end)
{
    LineRenderable *renderable = data();
    if (renderable)
        renderable->addLine(start, end);
}

LineRenderable *LineRenderableScriptable::data() const
{
    SharedRenderable renderable = qscriptvalue_cast<SharedRenderable>(thisObject());

    LineRenderable *data = renderable.objectCast<LineRenderable>().data();

    if (!data) {
        context()->throwError("LineRenderable system instance object not associated with a shared renderable.");
        return NULL;
    }

    return data;
}

QScriptValue ModelInstanceScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");
    SharedModelInstance node(new ModelInstance);
    QScriptValue result = engine->newVariant(context->thisObject(), qVariantFromValue(node));
    result.setPrototype(engine->defaultPrototype(qMetaTypeId<SharedModelInstance>()));
    return result;
}

void ModelInstanceScriptable::registerWith(QScriptEngine *engine)
{
    QScriptValue prototype = engine->newQObject(new ModelInstanceScriptable, QScriptEngine::ScriptOwnership);

    int typeId = qRegisterMetaType<SharedModelInstance>("SharedModelInstance");
    engine->setDefaultPrototype(typeId, prototype);

    QScriptValue ctor = engine->newFunction(ModelInstanceScriptableCtor);
    engine->globalObject().setProperty("ModelInstance", ctor);
}

const SharedModel &ModelInstanceScriptable::model() const
{
    ModelInstance *modelInstance = data();
    return modelInstance ? modelInstance->model() : nullSharedModel;
}

void ModelInstanceScriptable::setModel(const SharedModel &model)
{
    ModelInstance *modelInstance = data();
    if (modelInstance)
        modelInstance->setModel(model);
}

void ModelInstanceScriptable::addMesh(const SharedModel &model)
{
    ModelInstance *modelInstance = data();
    if (modelInstance)
        modelInstance->addMesh(model);
}

const Box3d &ModelInstanceScriptable::boundingBox() const
{
    ModelInstance *modelInstance = data();
    return modelInstance ? modelInstance->boundingBox() : emptyBoundingBox;
}

void ModelInstanceScriptable::setClickHandler(const QScriptValue &handler)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        qScriptConnect(modelInstance, SIGNAL(mousePressed()), QScriptValue(), handler);
    }
}

void ModelInstanceScriptable::setAnimationEventHandler(const QScriptValue &handler)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        qScriptConnect(modelInstance, SIGNAL(animationEvent(int,QString)), QScriptValue(), handler);
    }
}

uint ModelInstanceScriptable::renderCategory() const
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        return modelInstance->renderCategory();
    }
    return 0;
}

void ModelInstanceScriptable::setRenderCategory(uint category)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->setRenderCategory((RenderQueue::Category)category);
    }
}

const QString &ModelInstanceScriptable::idleAnimation() const
{
    ModelInstance *modelInstance = data();
    return modelInstance ? modelInstance->idleAnimation() : QString::null;
}

void ModelInstanceScriptable::setIdleAnimation(const QString &name)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->setIdleAnimation(name);
    }
}

void ModelInstanceScriptable::elapseDistance(float distance)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->elapseDistance(distance);
    }
}

void ModelInstanceScriptable::elapseRotation(float rotation)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->elapseRotation(rotation);
    }
}

bool ModelInstanceScriptable::playAnimation(const QString &name, bool looping)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        return modelInstance->playAnimation(name, looping);
    } else {
        return false;
    }
}

void ModelInstanceScriptable::stopAnimation()
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->stopAnimation();
    }
}

bool ModelInstanceScriptable::isIdling() const
{
    ModelInstance *modelInstance = data();
    return modelInstance ? modelInstance->isIdling() : false;
}

bool ModelInstanceScriptable::overrideMaterial(const QString &name, const SharedMaterialState &state)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->overrideMaterial(name, state);
    }
    return false;
}

bool ModelInstanceScriptable::clearOverrideMaterial(const QString &name)
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        return modelInstance->clearOverrideMaterial(name);
    }
    return false;
}

void ModelInstanceScriptable::clearOverrideMaterials()
{
    ModelInstance *modelInstance = data();
    if (modelInstance) {
        modelInstance->clearOverrideMaterials();
    }
}

ModelInstance *ModelInstanceScriptable::data() const
{
    SharedRenderable renderable = qscriptvalue_cast<SharedRenderable>(thisObject());

    ModelInstance *data = renderable.objectCast<ModelInstance>().data();

    if (!data) {
        context()->throwError("Model instance object not associated with a shared renderable.");
        return NULL;
    }

    return data;
}

QScriptValue LightScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");
    SharedLight node(new Light);
    QScriptValue result = engine->newVariant(context->thisObject(), qVariantFromValue(node));
    result.setPrototype(engine->defaultPrototype(qMetaTypeId<SharedLight>()));
    return result;
}

void LightScriptable::registerWith(QScriptEngine *engine)
{
    QScriptValue prototype = engine->newQObject(new LightScriptable, QScriptEngine::ScriptOwnership);

    int typeId = qRegisterMetaType<SharedLight>("SharedLight");
    engine->setDefaultPrototype(typeId, prototype);

    QScriptValue ctor = engine->newFunction(LightScriptableCtor);
    engine->globalObject().setProperty("Light", ctor);
}

Light *LightScriptable::data() const
{
    SharedRenderable renderable = qscriptvalue_cast<SharedRenderable>(thisObject());

    Light *data = renderable.objectCast<Light>().data();

    if (!data) {
        context()->throwError("Light object not associated with a shared renderable.");
        return NULL;
    }

    return data;
}

uint LightScriptable::lightType() const
{
    Light *light = data();
    return light ? light->type() : 0;
}

void LightScriptable::setLightType(uint type)
{
    Light *light = data();
    if (light)
        light->setType((Light::Type)type);
}

float LightScriptable::attenuation() const
{
    Light *light = data();
    return light ? light->attenuation() : 0;
}

void LightScriptable::setAttenuation(float attenuation)
{
    Light *light = data();
    if (light)
        light->setAttenuation(attenuation);
}

float LightScriptable::phi() const
{
    Light *light = data();
    return light ? light->phi() : 0;
}

void LightScriptable::setPhi(float phi)
{
    Light *light = data();
    if (light)
        light->setPhi(phi);
}

float LightScriptable::range() const
{
    Light *light = data();
    return light ? light->range() : 0;
}

void LightScriptable::setRange(float range)
{
    Light *light = data();
    if (light)
        light->setRange(range);
}

const Vector4 &LightScriptable::color() const
{
    Light *light = data();
    return light ? light->color() : nullVector;
}

void LightScriptable::setColor(const Vector4 &color)
{
    Light *light = data();
    if (light)
        light->setColor(color);
}

float LightScriptable::theta() const
{
    Light *light = data();
    return light ? light->theta() : 0;
}

void LightScriptable::setTheta(float theta)
{
    Light *light = data();
    if (light)
        light->setTheta(theta);
}

const Vector4 &LightScriptable::direction() const
{
    Light *light = data();
    return light ? light->direction() : nullVector;
}

void LightScriptable::setDirection(const Vector4 &direction)
{
    Light *light = data();
    if (light)
        light->setDirection(direction);
}

void LightScriptable::setDebugging(bool debugging)
{
    Light *light = data();
    if (light)
        light->setDebugging(debugging);
}

bool LightScriptable::isDebugging() const
{
    Light *light = data();
    return light ? light->isDebugging() : false;
}

SharedSceneNode SceneNodeScriptable::sceneNode() const
{
    SharedSceneNode node = qscriptvalue_cast<SharedSceneNode>(thisObject());
    if (!node)
        context()->throwError("Scene node object is not associated with a valid scene node.");
    return node;
}

const Vector4 &SceneNodeScriptable::position() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->position();
    }
    return nullVector;
}

const Quaternion &SceneNodeScriptable::rotation() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->rotation();
    }
    return nullRotation;
}

const Vector4 &SceneNodeScriptable::scale() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->scale();
    }
    return nullVector;
}

bool SceneNodeScriptable::isInteractive() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->isInteractive();
    }
    return false;
}

bool SceneNodeScriptable::isAnimated() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->isAnimated();
    }
    return false;
}

Scene *SceneNodeScriptable::scene() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->scene();
    }
    return NULL;
}

void SceneNodeScriptable::setPosition(const Vector4 &position)
{
    SharedSceneNode node = sceneNode();
    if (node) {
        node->setPosition(position);
    }
}

void SceneNodeScriptable::setRotation(const Quaternion &rotation)
{
    SharedSceneNode node = sceneNode();
    if (node) {
        node->setRotation(rotation);
    }
}

void SceneNodeScriptable::setScale(const Vector4 &scale)
{
    SharedSceneNode node = sceneNode();
    if (node) {
        node->setScale(scale);
    }
}

void SceneNodeScriptable::setInteractive(bool interactive)
{
    SharedSceneNode node = sceneNode();
    if (node) {
        node->setInteractive(interactive);
    }
}

void SceneNodeScriptable::setAnimated(bool animated)
{
    SharedSceneNode node = sceneNode();
    if (node) {
        node->setAnimated(animated);
    }
}

const Box3d &SceneNodeScriptable::worldBoundingBox() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->worldBoundingBox();
    } else {
        return emptyBoundingBox;
    }
}

const Box3d &SceneNodeScriptable::boundingBox() const
{
    SharedSceneNode node = sceneNode();
    if (node) {
        return node->boundingBox();
    } else {
        return emptyBoundingBox;
    }
}

void SceneNodeScriptable::attachObject(const SharedRenderable &renderable)
{
    SharedSceneNode node = sceneNode();
    if (node) {
        if (!renderable) {
            context()->throwError("Trying to attach a null-renderable to a scene node.");
        } else {
            node->attachObject(renderable);
        }
    }
}

static QScriptValue SceneNodeScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");
    SharedSceneNode node(new SceneNode);
    QScriptValue result = engine->newVariant(context->thisObject(), qVariantFromValue(node));
    result.setPrototype(engine->defaultPrototype(qMetaTypeId<SharedSceneNode>()));
    return result;
}

void SceneNodeScriptable::registerWith(QScriptEngine *engine)
{
     registerValueType<SharedSceneNode>(engine, "SharedSceneNode");

    int metaId = qMetaTypeId<SharedSceneNode>();
    QScriptValue prototype = engine->newQObject(new SceneNodeScriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);

    // Add a constructor function for scene nodes
    QScriptValue globalObject = engine->globalObject();
    QScriptValue ctor = engine->newFunction(SceneNodeScriptableCtor);
    globalObject.setProperty("SceneNode", ctor);
}

template<typename T>
QScriptValue float4ToScriptValue(QScriptEngine *engine, const T &in)
{
    QScriptValue result = engine->newArray(4);
    result.setProperty(0, QScriptValue(in.x()));
    result.setProperty(1, QScriptValue(in.y()));
    result.setProperty(2, QScriptValue(in.z()));
    result.setProperty(3, QScriptValue(in.w()));
    return result;
}

template<typename T>
void float4FromScriptValue(const QScriptValue &object, T &out)
{
    if (object.isArray()) {
        QScriptValue element = object.property(0);

        if (!element.isUndefined()) {
            out.setX(element.toNumber());
        } else {
            out.setX(0);
            out.setY(0);
            out.setZ(0);
            out.setW(1);
            return;
        }

        element = object.property(1);

        if (!element.isUndefined()) {
            out.setY(element.toNumber());
        } else {
            out.setY(0);
            out.setZ(0);
            out.setW(1);
            return;
        }

        element = object.property(2);

        if (!element.isUndefined()) {
            out.setZ(element.toNumber());
        } else {
            out.setZ(0);
            out.setW(1);
            return;
        }

        element = object.property(3);

        if (!element.isUndefined()) {
            out.setW(element.toNumber());
        } else {
            out.setW(1);
            return;
        }

    } else {
        qWarning("Trying to convert a non-array to a Vector4 object.");

        out.setX(std::numeric_limits<float>::quiet_NaN());
        out.setY(std::numeric_limits<float>::quiet_NaN());
        out.setZ(std::numeric_limits<float>::quiet_NaN());
        out.setW(std::numeric_limits<float>::quiet_NaN());
    }
}

QScriptValue vectorOfVector4ToScriptValue(QScriptEngine *engine, const QVector<Vector4> &in)
{
    QScriptValue result = engine->newArray(in.size());

    for (int i = 0; i < in.size(); ++i) {
        result.setProperty(i, float4ToScriptValue(engine, in.at(i)));
    }

    return result;
}

void vectorOfVector4FromScriptValue(const QScriptValue &object, QVector<Vector4> &out)
{
    if (object.isArray()) {
        QScriptValue length = object.property("length");

        if (length.isNumber()) {
            out.resize(length.toUInt32());
        }

        uint i = 0;
        forever {
            QScriptValue element = object.property(i++);

            if (element.isUndefined())
                break;

            float4FromScriptValue(element, out[i]);
        }
    } else {
        qWarning("Trying to convert a non-array to a vector of vector4 objects.");
    }
}

void Vector4Scriptable::registerWith(QScriptEngine *engine)
{
    qScriptRegisterMetaType< QVector<Vector4> >(engine, vectorOfVector4ToScriptValue, vectorOfVector4FromScriptValue);
    qScriptRegisterMetaType<Vector4>(engine, float4ToScriptValue<Vector4>, float4FromScriptValue<Vector4>);
}

void QuaternionScriptable::registerWith(QScriptEngine *engine)
{
    qScriptRegisterMetaType<Quaternion>(engine, float4ToScriptValue<Quaternion>, float4FromScriptValue<Quaternion>);
}

Vector4 Box3dScriptable::minimum() const
{
    return (qscriptvalue_cast<Box3d>(thisObject())).minimum();
}

Vector4 Box3dScriptable::maximum() const
{
    return (qscriptvalue_cast<Box3d>(thisObject())).maximum();
}

void Box3dScriptable::setMinimum(const Vector4 &minimum)
{
    Box3d box = qscriptvalue_cast<Box3d>(thisObject());
    box.setMinimum(minimum);
    thisObject().setData(engine()->newVariant(qVariantFromValue(box)));
}

void Box3dScriptable::setMaximum(const Vector4 &maximum)
{
    Box3d box = qscriptvalue_cast<Box3d>(thisObject());
    box.setMaximum(maximum);
    thisObject().setData(engine()->newVariant(qVariantFromValue(box)));
}

static QScriptValue Box3dScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");

    if (context->argumentCount() == 2) {
        Vector4 minimum = qscriptvalue_cast<Vector4>(context->argument(0));
        Vector4 maximum = qscriptvalue_cast<Vector4>(context->argument(1));

        return engine->newVariant(context->thisObject(), qVariantFromValue(Box3d(minimum, maximum)));
    } else {
        return context->throwError("Box3d takes 2 arguments.");
    }
}

void Box3dScriptable::registerWith(QScriptEngine *engine)
{
    int metaId = qRegisterMetaType<Box3d>();
    QScriptValue prototype = engine->newQObject(new Box3dScriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);

    // Add a constructor function for scene nodes
    QScriptValue globalObject = engine->globalObject();
    QScriptValue ctor = engine->newFunction(Box3dScriptableCtor);
    globalObject.setProperty("Box3d", ctor);
}

}
