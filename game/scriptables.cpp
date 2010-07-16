
#include <QScriptEngine>
#include <QScriptValueIterator>

#include "scriptables.h"
#include "modelfile.h"
#include "particlesystem.h"
#include "materialstate.h"

#include <gamemath.h>
using namespace GameMath;

using EvilTemple::SharedModel;
using EvilTemple::SharedMaterialState;

Q_DECLARE_METATYPE(SharedModel)
Q_DECLARE_METATYPE(SharedMaterialState)

Q_DECLARE_METATYPE(Vector4)
Q_DECLARE_METATYPE(QVector<Vector4>)
Q_DECLARE_METATYPE(Quaternion)
Q_DECLARE_METATYPE(Box3d)

namespace EvilTemple {

GAMEMATH_ALIGNEDTYPE_PRE Vector4 GAMEMATH_ALIGNEDTYPE_MID nullVector GAMEMATH_ALIGNEDTYPE_POST = Vector4(0, 0, 0, 0);
GAMEMATH_ALIGNEDTYPE_PRE Box3d GAMEMATH_ALIGNEDTYPE_MID emptyBoundingBox GAMEMATH_ALIGNEDTYPE_POST = Box3d(nullVector, nullVector);

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

        if (element.isNumber()) {
            out.setX(element.toNumber());
        } else {
            out.setX(0);
            out.setY(0);
            out.setZ(0);
            out.setW(1);
            return;
        }

        element = object.property(1);

        if (element.isNumber()) {
            out.setY(element.toNumber());
        } else {
            out.setY(0);
            out.setZ(0);
            out.setW(1);
            return;
        }

        element = object.property(2);

        if (element.isNumber()) {
            out.setZ(element.toNumber());
        } else {
            out.setZ(0);
            out.setW(1);
            return;
        }

        element = object.property(3);

        if (element.isNumber()) {
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
