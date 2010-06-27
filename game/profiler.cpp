
#include "profiler.h"

#include <QLinkedList>
#include <QElapsedTimer>

namespace EvilTemple {

struct Section {
    Profiler::Category category;
    QElapsedTimer start;
};

static const uint TotalSamples = 1000;

class ProfilerData {
public:
    ProfilerData()
    {
        for (int i = 0; i < Profiler::Count; ++i) {
            totalMsElapsed[i] = 0;
            totalSamplesTaken[i] = 0;
            for (int j = 0; j < TotalSamples; ++j) {
                samples[i][j] = 0;
            }
        }
    }

    QLinkedList<Section> activeSections;
    
    double totalMsElapsed[Profiler::Count];
    uint totalSamplesTaken[Profiler::Count];
    double samples[Profiler::Count][TotalSamples];
};

void Profiler::enter(Category category)
{
    Section section;
    section.category = category;
    section.start.start();    
    d->activeSections.append(section);
}

void Profiler::leave()
{
    Section section = d->activeSections.takeLast();
    QElapsedTimer end;
    end.start();
    double milisecondsElapsed = end.msecsTo(section.start);

    uint i = (d->totalSamplesTaken[section.category]++) % TotalSamples;
    d->samples[section.category][i] = milisecondsElapsed;
    d->totalMsElapsed[section.category] += milisecondsElapsed;
}

Profiler::Report Profiler::report()
{
    Report report;

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

QScopedPointer<ProfilerData> Profiler::d(new ProfilerData);

}

