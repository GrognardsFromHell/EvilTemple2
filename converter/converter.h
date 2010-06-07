#ifndef CONVERTER_H
#define CONVERTER_H

#include <QObject>
#include <QScopedPointer>

class ConverterData;

class Converter : public QObject {
Q_OBJECT
friend class ConverterData;
public:
    Converter(const QString &inputPath, const QString &outputPath);
    ~Converter();

    void setExternal(bool ext);

    void cancel();

    bool isCancelled() const;

public slots:
    bool convert();

signals:
    void progressUpdate(int current, int max, const QString &description);

private:
    QScopedPointer<ConverterData> d_ptr;
};

#endif // CONVERTER_H
