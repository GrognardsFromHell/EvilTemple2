
#ifndef PROFILER_H
#define PROFILER_H

#include <QtCore/QScopedPointer>

namespace EvilTemple {

class ProfilerData;

class Profiler {
public:
    enum Category {
        SceneElapseTime = 0,
        SceneRender,
        Count
    };

    struct Report {
        double totalElapsedTime[Count];
        uint totalSamples[Count];
        double meanTime[Count];
    };

    static Report report();

    static void enter(Category category);
    static void leave();

private:
    static QScopedPointer<ProfilerData> d;
};

}

#endif // PROFILER_H
