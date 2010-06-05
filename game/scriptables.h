#ifndef SCRIPTABLES_H
#define SCRIPTABLES_H

#include <QObject>
#include <QScriptable>

#include "scene.h"
#include "scenenode.h"
#include "lighting.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

    void registerRenderableScriptable(QScriptEngine *engine);

    class ModelScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(const Box3d &boundingBox READ boundingBox);
    Q_PROPERTY(float radius READ radius);
    Q_PROPERTY(float radiusSquared READ radiusSquared)
    public:
        static void registerWith(QScriptEngine *engine);

        const Box3d &boundingBox() const;
        float radius() const;
        float radiusSquared() const;
    private:
        SharedModel data() const;
    };

    class MaterialStateScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    public:
        static void registerWith(QScriptEngine *engine);
    };
    
    class ModelInstanceScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(SharedModel model READ model WRITE setModel);
    Q_PROPERTY(const Box3d &boundingBox READ boundingBox);
    Q_PROPERTY(uint renderCategory READ renderCategory WRITE setRenderCategory);
    public:
        static void registerWith(QScriptEngine *engine);

        const SharedModel &model() const;
        void setModel(const SharedModel &model);


        uint renderCategory() const;
        void setRenderCategory(uint category);       

        const Box3d &boundingBox() const;
    public slots:
        void setClickHandler(const QScriptValue &handler);

        bool overrideMaterial(const QString &name, const SharedMaterialState &state);
        bool clearOverrideMaterial(const QString &name);
        void clearOverrideMaterials();
        
        void addMesh(const SharedModel &model);

    private:
        ModelInstance *data() const;
    };

    class LightScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(uint lightType READ lightType WRITE setLightType)
    Q_PROPERTY(float attenuation READ attenuation WRITE setAttenuation)
    Q_PROPERTY(float phi READ phi WRITE setPhi)
    Q_PROPERTY(float theta READ theta WRITE setTheta)
    Q_PROPERTY(Vector4 color READ color WRITE setColor)
    Q_PROPERTY(Vector4 direction READ direction WRITE setDirection)
    Q_PROPERTY(float range READ range WRITE setRange)
    Q_PROPERTY(bool debugging READ isDebugging WRITE setDebugging)
    public:
        static void registerWith(QScriptEngine *engine);
    private:
        Light *data() const;

        uint lightType() const;
        void setLightType(uint type);

        float attenuation() const;
        void setAttenuation(float attenuation);

        float phi() const;
        void setPhi(float phi);

        float range() const;
        void setRange(float range);

        const Vector4 &color() const;
        void setColor(const Vector4 &color);

        float theta() const;
        void setTheta(float theta);

        const Vector4 &direction() const;
        void setDirection(const Vector4 &direction);

        void setDebugging(bool debugging);
        bool isDebugging() const;
    };

    class SceneNodeScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(Vector4 position READ position WRITE setPosition);
    Q_PROPERTY(Quaternion rotation READ rotation WRITE setRotation);
    Q_PROPERTY(Vector4 scale READ scale WRITE setScale);
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive);
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated);
    Q_PROPERTY(Scene *scene READ scene);
    Q_PROPERTY(const Box3d &worldBoundingBox READ worldBoundingBox);
    Q_PROPERTY(const Box3d &boundingBox READ boundingBox);
    public:
        static void registerWith(QScriptEngine *engine);

        const Vector4 &position() const;
        const Quaternion &rotation() const;
        const Vector4 &scale() const;
        bool isInteractive() const;
        bool isAnimated() const;
        Scene *scene() const;

        void setPosition(const Vector4 &position);
        void setRotation(const Quaternion &rotation);
        void setScale(const Vector4 &scale);
        void setInteractive(bool interactive);
        void setAnimated(bool animated);

        const Box3d &worldBoundingBox() const;
        const Box3d &boundingBox() const;
    public slots:
        void attachObject(const SharedRenderable &renderable);
    private:
        SharedSceneNode sceneNode() const;
    };

    class Vector4Scriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(float x READ x WRITE setX)
    Q_PROPERTY(float y READ y WRITE setY)
    Q_PROPERTY(float z READ z WRITE setZ)
    Q_PROPERTY(float w READ w WRITE setW)
    public:
        float x() const;
        float y() const;
        float z() const;
        float w() const;

        void setX(float value);
        void setY(float value);
        void setZ(float value);
        void setW(float value);

        static void registerWith(QScriptEngine *engine);
    };

    class QuaternionScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(float x READ x WRITE setX)
    Q_PROPERTY(float y READ y WRITE setY)
    Q_PROPERTY(float z READ z WRITE setZ)
    Q_PROPERTY(float scalar READ scalar WRITE setScalar)
    public:
        float x() const;
        float y() const;
        float z() const;
        float scalar() const;

        void setX(float value);
        void setY(float value);
        void setZ(float value);
        void setScalar(float value);

        static void registerWith(QScriptEngine *engine);
    };

    class Box3dScriptable : public QObject, protected QScriptable {
    Q_OBJECT
    Q_PROPERTY(Vector4 minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(Vector4 maximum READ maximum WRITE setMaximum)
    public:
        Vector4 minimum() const;
        Vector4 maximum() const;
        void setMinimum(const Vector4 &minimum);
        void setMaximum(const Vector4 &maximum);

        static void registerWith(QScriptEngine *engine);
    };

}

#endif // SCRIPTABLES_H
