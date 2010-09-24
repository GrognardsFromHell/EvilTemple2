#ifndef CONVERTER_H
#define CONVERTER_H

#include "global.h"

#include <QObject>
#include <QScopedPointer>

class ConverterData;

class CONVERSIONSHARED_EXPORT Converter : public QObject {
Q_OBJECT
friend class ConverterData;
public:
    Converter(const QString &inputPath, const QString &outputPath);
    ~Converter();

    void setExternal(bool ext);

    void cancel();

public slots:
    bool convert();

signals:
    void progressUpdate(int current, int max, const QString &description);

private:
    QScopedPointer<ConverterData> d_ptr;
};

#endif // CONVERTER_H
