
#include "profiler.h"

#include <QLinkedList>
#include <QElapsedTimer>

namespace EvilTemple {

struct Section {
    Profiler::Category category;
    double start;
};

static const uint TotalSamples = 1000;

#ifdef Q_OS_WIN

#include <windows.h>

static double frequency = -1;

static double getTicks()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    if (frequency == -1) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        frequency = freq.QuadPart;
    }

    return now.QuadPart * 1000 / frequency;
}

#else

static double getTicks()
{
    QElapsedTimer timer;
    timer.start();
    return timer.msecsSinceReference();
}

#endif

class ProfilerData {
public:
    ProfilerData()
    {
        clear();
    }

    void clear();

    QLinkedList<Section> activeSections;

    double totalMsElapsed[Profiler::Count];
    uint totalSamplesTaken[Profiler::Count];
    double samples[Profiler::Count][TotalSamples];

    uint totalFrames;
};

void ProfilerData::clear()
{
    for (int i = 0; i < Profiler::Count; ++i) {
        totalMsElapsed[i] = 0;
        totalSamplesTaken[i] = 0;
        for (int j = 0; j < TotalSamples; ++j) {
            samples[i][j] = 0;
        }
    }

    totalFrames = 0;
}

void Profiler::enter(Category category)
{
    Section section;
    section.category = category;
    section.start = getTicks();
    d->activeSections.append(section);
}

void Profiler::leave()
{
    Section section = d->activeSections.takeLast();

    double milisecondsElapsed = getTicks() - section.start;

    uint i = (d->totalSamplesTaken[section.category]++) % TotalSamples;
    d->samples[section.category][i] = milisecondsElapsed;
    d->totalMsElapsed[section.category] += milisecondsElapsed;
}

Profiler::Report Profiler::report()
{
    Report report;

    report.totalFrames = d->totalFrames;

    for (int i = 0; i < Count; ++i) {
        report.totalSamples[i] = d->totalSamplesTaken[i];
        report.totalElapsedTime[i] = d->totalMsElapsed[i];

        // Build a mean over the last X samples
        int count = qMin<int>(TotalSamples, d->totalSamplesTaken[i]);
        if (count > 0) {
            float sum = 0.0f;
            for (int j = 0; j < count; ++j) {
                sum += d->samples[i][j];
            }
            report.meanTime[i] = sum / (float)count;
        } else {
            report.meanTime[i] = 0;
        }
    }

    return report;
}

void Profiler::clear()
{
    d->clear();
}

void Profiler::newFrame()
{
    d->totalFrames++;
}

QScopedPointer<ProfilerData> Profiler::d(new ProfilerData);

}

