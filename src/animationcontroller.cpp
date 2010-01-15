
#include <QTime>

#include "animationcontroller.h"
#include "skeleton.h"

namespace EvilTemple
{

    class AnimationControllerPrivate
    {
    public:
        const Skeleton *skeleton;
        const Animation &animation;
        QTime lastUpdate; // Time when the last update was performed.
        float distanceCovered; // Distance accumulated between updates
        float rotationCovered; // Rotation accumulated between updates (in degrees)
        int currentFrame; // Which frame in the animation is current

        AnimationControllerPrivate(const Skeleton *skeleton, const Animation &animation);

        void update();

        void nextFrame();
    };

    inline AnimationControllerPrivate::AnimationControllerPrivate(const Skeleton *_skeleton,
                                                                  const Animation &_animation)
        : skeleton(_skeleton), animation(_animation), distanceCovered(0), rotationCovered(0)
    {
    }

    inline void AnimationControllerPrivate::update()
    {
        /*
            The timer starts when the first frame update is requested
            This avoids a "jump" in the animation when there is a delay between
            construction of this controller and the first frame update.
        */
        if (lastUpdate.isNull()) {
            lastUpdate.start();
            return;
        }

        int elapsed = lastUpdate.restart(); // Miliseconds


    }

    void AnimationControllerPrivate::nextFrame()
    {

    }

    AnimationController::AnimationController(const Skeleton *skeleton, const Animation &animation, QObject *parent)
        : QObject(parent), d_ptr(new AnimationControllerPrivate(skeleton, animation))
    {
    }

    AnimationController::~AnimationController()
    {
    }

    void AnimationController::update()
    {
        d_ptr->update();
    }

    void AnimationController::moved(float distance)
    {
        d_ptr->distanceCovered += distance;
    }

    void AnimationController::rotated(float degrees)
    {
        d_ptr->rotationCovered += degrees;
    }

}
