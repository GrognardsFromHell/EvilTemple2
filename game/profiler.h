
#ifndef PROFILER_H
#define PROFILER_H

#include <QtCore/QScopedPointer>

namespace EvilTemple {

class ProfilerData;

class Profiler {
public:
    enum Category {
        SceneElapseTime = 0,
        ModelInstanceElapseTime,
        ParticleSystemElapseTime,
        ModelInstanceRender,
        ParticleSystemRender,
        SceneRender,
        FrameRender,
        Count
    };

    struct Report {
        double totalElapsedTime[Count];
        uint totalSamples[Count];
        double meanTime[Count];
        uint totalFrames;
    };

    static Report report();

    static void enter(Category category);

    static void leave();

    static void clear();

    static void newFrame();

private:
    static QScopedPointer<ProfilerData> d;
};

template<Profiler::Category category>
class ProfileScope {
public:
    inline ProfileScope()
    {
        Profiler::enter(category);
    }

    inline ~ProfileScope()
    {
        Profiler::leave();
    }
};

}

#endif // PROFILER_H
