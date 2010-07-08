
#include <QFile>
#include <QTextStream>
#include <QDir>

#include "exclusions.h"

bool Exclusions::load(const QString &filename)
{
    QFile exclusionText(filename);

    if (!exclusionText.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qWarning("Cannot open exclusions.txt. This file should've been included in this binary as a resource.");
        return false;
    }

    QTextStream stream(&exclusionText);

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed(); // Remove whitespace, newline is already removed

        // Allow comments
        if (line.startsWith('#'))
            continue;

        // Normalize path
        line = QDir::toNativeSeparators(line).toLower();

        if (line.endsWith('*'))
            excludedPrefixes.append(line.left(line.length() - 1));
        else
            exclusions.append(line);
    }

    exclusionText.close();
    return true;
}

bool Exclusions::isExcluded(const QString &filename) const
{

    QString normalizedFilename = QDir::toNativeSeparators(filename).toLower();

    // Check for exact matches
    foreach (const QString &exclusion, exclusions) {
        if (exclusion.compare(normalizedFilename) == 0) {
            return true;
        }
    }

    // Check for prefix exclusions
    foreach (const QString &exclusion, excludedPrefixes) {
        if (normalizedFilename.startsWith(exclusion)) {
            return true;
        }
    }

    return false;

}
