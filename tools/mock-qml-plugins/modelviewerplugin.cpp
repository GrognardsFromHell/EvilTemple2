#include "modelviewerplugin.h"

#include "modelviewer.h"

void ModelViewerPlugin::registerTypes(const char *uri)
{
    qDebug("Loading model viewer plugin.");
    qmlRegisterType<ModelViewer>(uri, 1, 0, "ModelViewer");
}

Q_EXPORT_PLUGIN2(modelviewerplugin, ModelViewerPlugin);
