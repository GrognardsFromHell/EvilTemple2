#ifndef EXCLUSIONS_H
#define EXCLUSIONS_H

#include <QStringList>

class Exclusions
{
public:
    bool load(const QString &filename);

    bool isExcluded(const QString &filename) const;

private:
    QStringList exclusions;
    QStringList excludedPrefixes;
};

#endif // EXCLUSIONS_H
