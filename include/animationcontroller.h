#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QMatrix4x4>

namespace EvilTemple
{

    class AnimationControllerPrivate;
    class Skeleton;
    struct Animation;

    class AnimationController : public QObject
    {
        Q_OBJECT
    public:
        explicit AnimationController(const Skeleton *skeleton, const Animation &animation, QObject *parent = 0);
        ~AnimationController();

    public slots:
        /**
          Updates the progress of this animation controller either based
          on time, rotation or distance. Whatever the animation managed by
          this controller requires. Depending on the maximum frames per second
          permitted, you can use a timer to call this method.

          Please note that you still need to call @ref moved(float) and @ref rotated(float)
          to update the progress of rotation and distance driven animations. This
          method will then update the bone matrices for this animation.
          */
        void update();

        /**
          Use this method to notify this controller of character movement.
          This is used to progress animations that depend on movement.

          @param distance How much the character moved. Use incremental updates.
          */
        void moved(float distance);

        /**
          Use this method to notify this controller of character rotation.
          This is used to progress animations that depend on character rotation.

          @param degrees How much the character was rotated in degrees. Use incremental updates.
          */
        void rotated(float degrees);

        /**
          Gets the full bone transformation matrix for every bone.
          The matrix for every bone is equal to fullWorldMatrix(animated) * fullWorldInverseMatrix
          and can be multiplied with a vertex position directly to receive an animated vertex.
          */
        const QVector<QMatrix4x4> &boneMatrices() const;

    private:

        QScopedPointer<AnimationControllerPrivate> d_ptr;

        Q_DISABLE_COPY(AnimationController);

    };

}

#endif // ANIMATIONCONTROLLER_H
