
#ifndef MOVABLEOBJECT_H
#define MOVABLEOBJECT_H

namespace EvilTemple {

class MovableObject : QObject {
Q_OBJECT
public:
    virtual ~MovableObject();


protected:
    Scene *mScene; // The scene this object is associated with

    SceneNode *mAttachedTo; // The node this object is attached to

    bool mVisible; // Is this object currently visible? (This does not override the parent's visibility)

    bool mDebugging; // Is this object being debugged? (This toggles the display of debug geometry)

private:
    Q_DISABLE_COPY(MovableObject);
};

typedef QSharedPointer<MovableObject> SharedMovableObject;

}

#endif // MOVABLEOBJECT_H