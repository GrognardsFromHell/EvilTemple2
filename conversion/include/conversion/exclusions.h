#ifndef EXCLUSIONS_H
#define EXCLUSIONS_H

#include "global.h"

#include <QStringList>

class CONVERSIONSHARED_EXPORT Exclusions
{
public:
    bool load(const QString &filename);

    bool isExcluded(const QString &filename) const;

private:
    QStringList exclusions;
    QStringList excludedPrefixes;
};

#endif // EXCLUSIONS_H
