#ifndef MODELVIEWERPLUGIN_H
#define MODELVIEWERPLUGIN_H

#include <QDeclarativeExtensionPlugin>

class ModelViewerPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    void registerTypes(const char *uri);
};

#endif // MODELVIEWERPLUGIN_H
