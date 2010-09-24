
#include <QFile>
#include <QTextStream>
#include <QDir>

#include "conversion/exclusions.h"
#include "conversion/util.h"

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
        if (line.startsWith('#') || line.isEmpty())
            continue;

        // Normalize path
        line = normalizePath(line);

        if (line.endsWith('*')) {
            qDebug("Adding exclusion prefix %s.", qPrintable(line));
            excludedPrefixes.append(line.left(line.length() - 1));
        } else {
            qDebug("Adding exclusion %s.", qPrintable(line));
            exclusions.append(line);
        }
    }

    exclusionText.close();
    return true;
}

bool Exclusions::isExcluded(const QString &filename) const
{
    QString normalizedFilename = normalizePath(filename);

    // Check for exact matches
    foreach (const QString &exclusion, exclusions) {
        if (exclusion == normalizedFilename) {
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
