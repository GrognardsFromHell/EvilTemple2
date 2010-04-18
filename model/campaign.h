#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "modelglobal.h"

#include <QObject>

class MODEL_EXPORT Campaign : public QObject
{
Q_OBJECT
public:
    explicit Campaign(QObject *parent = 0);

signals:

public slots:

};

#endif // CAMPAIGN_H
