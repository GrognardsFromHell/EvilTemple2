
#include <QScriptEngine>

#include "scriptables.h"
#include "modelfile.h"

namespace EvilTemple {
    typedef QSharedPointer<ModelInstance> SharedModelInstance;
}

using EvilTemple::SharedModel;
using EvilTemple::SharedSceneNode;
using EvilTemple::SharedRenderable;
using EvilTemple::SharedModelInstance;
using EvilTemple::SharedLight;

Q_DECLARE_METATYPE(SharedSceneNode)
Q_DECLARE_METATYPE(SharedModel)
Q_DECLARE_METATYPE(SharedRenderable)
Q_DECLARE_METATYPE(SharedModelInstance)
Q_DECLARE_METATYPE(SharedLight)

Q_DECLARE_METATYPE(Vector4)
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

float Vector4Scriptable::x() const
{
    return (qscriptvalue_cast<Vector4>(thisObject())).x();
}

float Vector4Scriptable::y() const
{
    return (qscriptvalue_cast<Vector4>(thisObject())).y();
}

float Vector4Scriptable::z() const
{
    return (qscriptvalue_cast<Vector4>(thisObject())).z();
}

float Vector4Scriptable::w() const
{
    return (qscriptvalue_cast<Vector4>(thisObject())).w();
}

void Vector4Scriptable::setX(float value)
{
    Vector4 vector = qscriptvalue_cast<Vector4>(thisObject());
    vector.setX(value);
    thisObject().setData(engine()->newVariant(qVariantFromValue(vector)));
}

void Vector4Scriptable::setY(float value)
{
    Vector4 vector = qscriptvalue_cast<Vector4>(thisObject());
    vector.setY(value);
    thisObject().setData(engine()->newVariant(qVariantFromValue(vector)));
}

void Vector4Scriptable::setZ(float value)
{
    Vector4 vector = qscriptvalue_cast<Vector4>(thisObject());
    vector.setZ(value);
    thisObject().setData(engine()->newVariant(qVariantFromValue(vector)));
}

void Vector4Scriptable::setW(float value)
{
    Vector4 vector = qscriptvalue_cast<Vector4>(thisObject());
    vector.setW(value);
    thisObject().setData(engine()->newVariant(qVariantFromValue(vector)));
}

static QScriptValue Vector4ScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");

    if (context->argumentCount() == 0) {
        return engine->newVariant(context->thisObject(), qVariantFromValue(Vector4(0, 0, 0, 0)));
    } else if (context->argumentCount() == 4) {
        float x = context->argument(0).toNumber();
        float y = context->argument(1).toNumber();
        float z = context->argument(2).toNumber();
        float w = context->argument(3).toNumber();

        return engine->newVariant(context->thisObject(), qVariantFromValue(Vector4(x, y, z, w)));
    } else {
        return context->throwError("vector4 takes 0 or 4 arguments.");
    }
}

void Vector4Scriptable::registerWith(QScriptEngine *engine)
{
    int metaId = qRegisterMetaType<Vector4>();
    QScriptValue prototype = engine->newQObject(new Vector4Scriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);

    // Add a constructor function for scene nodes
    QScriptValue globalObject = engine->globalObject();
    QScriptValue ctor = engine->newFunction(Vector4ScriptableCtor);
    globalObject.setProperty("Vector4", ctor);
}

float QuaternionScriptable::x() const
{
    return (qscriptvalue_cast<Quaternion>(thisObject())).x();
}

float QuaternionScriptable::y() const
{
    return (qscriptvalue_cast<Quaternion>(thisObject())).y();
}

float QuaternionScriptable::z() const
{
    return (qscriptvalue_cast<Quaternion>(thisObject())).z();
}

float QuaternionScriptable::scalar() const
{
    return (qscriptvalue_cast<Quaternion>(thisObject())).w();
}

void QuaternionScriptable::setX(float value)
{
    Quaternion vector = qscriptvalue_cast<Quaternion>(thisObject());
    Quaternion newVector(value, vector.y(), vector.z(), vector.w());
    thisObject().setData(engine()->newVariant(qVariantFromValue(newVector)));
}

void QuaternionScriptable::setY(float value)
{
    Quaternion vector = qscriptvalue_cast<Quaternion>(thisObject());
    Quaternion newVector(vector.x(), value, vector.z(), vector.w());
    thisObject().setData(engine()->newVariant(qVariantFromValue(newVector)));
}

void QuaternionScriptable::setZ(float value)
{
    Quaternion vector = qscriptvalue_cast<Quaternion>(thisObject());
    Quaternion newVector(vector.x(), vector.y(), value, vector.w());
    thisObject().setData(engine()->newVariant(qVariantFromValue(newVector)));
}

void QuaternionScriptable::setScalar(float value)
{
    Quaternion vector = qscriptvalue_cast<Quaternion>(thisObject());
    Quaternion newVector(vector.x(), vector.y(), vector.z(), value);
    thisObject().setData(engine()->newVariant(qVariantFromValue(newVector)));
}

static QScriptValue QuaternionScriptableCtor(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");

    if (context->argumentCount() == 0) {
        return engine->newVariant(context->thisObject(), qVariantFromValue(Quaternion(0, 0, 0, 1)));
    } else if (context->argumentCount() == 4) {
        float x = context->argument(0).toNumber();
        float y = context->argument(1).toNumber();
        float z = context->argument(2).toNumber();
        float w = context->argument(3).toNumber();

        return engine->newVariant(context->thisObject(), qVariantFromValue(Quaternion(x, y, z, w)));
    } else {
        return context->throwError("Quaternion takes 0 or 4 arguments.");
    }
}

void QuaternionScriptable::registerWith(QScriptEngine *engine)
{
    int metaId = qRegisterMetaType<Quaternion>();
    QScriptValue prototype = engine->newQObject(new QuaternionScriptable, QScriptEngine::ScriptOwnership);
    engine->setDefaultPrototype(metaId, prototype);

    // Add a constructor function for scene nodes
    QScriptValue globalObject = engine->globalObject();
    QScriptValue ctor = engine->newFunction(QuaternionScriptableCtor);
    globalObject.setProperty("Quaternion", ctor);
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
