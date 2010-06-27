#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include <QMetaType>
#include <QObject>

namespace EvilTemple {

class TranslationsData;

class Translations : public QObject
{
    Q_OBJECT
public:
    explicit Translations(QObject *parent = 0);
    ~Translations();

signals:

public slots:
    /**
      Loads a translation file. Please keep in mind that existing translations
      are not cleared by this. So this method can be used to load multiple translation
      files.
      */
    bool load(const QString &filename);

    /**
      Retrieves a translation for the given key, or a placeholder text if the key doesn't exist.
      */
    QString get(const QString &key) const;

private:
    QScopedPointer<TranslationsData> d;

    Q_DISABLE_COPY(Translations);
};

}

Q_DECLARE_METATYPE(EvilTemple::Translations*)

#endif // TRANSLATIONS_H
