
#include <QScriptEngine>

#include "messagefile.h"
#include "virtualfilesystem.h"
#include "convertscriptstask.h"
#include "prototypeconverter.h"

using namespace Troika;

ConvertScriptsTask::ConvertScriptsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

uint ConvertScriptsTask::cost() const
{
    return 5;
}

QString ConvertScriptsTask::description() const
{
    return "Converting scripts";
}

static QScriptValue readMesFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs) {
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readMesFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QHash<uint,QString> mapping = MessageFile::parse(vfs->openFile(filename));

    QScriptValue result = engine->newObject();

    foreach (uint key, mapping.keys()) {
        result.setProperty(QString("%1").arg(key), QScriptValue(mapping[key]));
    }

    return result;
}

static QScriptValue addFile(QScriptContext *ctx, QScriptEngine *engine, IFileWriter *writer) {
    if (ctx->argumentCount() != 3) {
        return ctx->throwError("addFile takes three arguments: filename, content, compression.");
    }

    QString filename = ctx->argument(0).toString();
    QString content = ctx->argument(1).toString();
    int compression = ctx->argument(2).toInt32();

    writer->addFile(filename, content.toUtf8(), compression > 0);

    return QScriptValue(true);
}

static QScriptValue readTabFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs) {
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readTabFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QByteArray content = vfs->openFile(filename);
    QList<QByteArray> lines = content.split('\n');
    QList< QList<QByteArray> > tabFileContent;

    for (int i = 0; i < lines.length(); ++i) {
        QByteArray line = lines[i];
        if (line.endsWith('\r')) {
            line = line.left(line.length() - 1);
        }

        if (line.isEmpty())
            continue;

        tabFileContent.append(line.split('\t'));
    }

    QScriptValue result = engine->newArray(tabFileContent.length());

    for (int i = 0; i < tabFileContent.length(); ++i) {
        QList<QByteArray> line = tabFileContent[i];
        QScriptValue record = engine->newArray(line.length());
        for (int j = 0; j < line.length(); ++j) {
            record.setProperty(j, QScriptValue(QString(line[j])));
        }
        result.setProperty(i, record);
    }

    return result;
}

void ConvertScriptsTask::convertPrototypes(IFileWriter *writer, QScriptEngine *engine)
{
    PrototypeConverter converter(service()->virtualFileSystem());

    QVariantMap result = converter.convertPrototypes(service()->prototypes());

    QScriptValue postprocess = engine->globalObject().property("postprocess");

    QScriptValueList args;
    args.append(engine->toScriptValue<QVariantMap>(result));

    postprocess.call(QScriptValue(), args);

    if (engine->hasUncaughtException()) {
        qWarning("JS Error: %s", qPrintable(engine->uncaughtException().toString()));
        return;
    }
}

void ConvertScriptsTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("scripts"));

    QScriptEngine engine;

    VirtualFileSystem *vfs = service()->virtualFileSystem();

    QScriptValue readMesFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readMesFile, vfs);
    engine.globalObject().setProperty("readMes", readMesFn);

    QScriptValue readTabFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readTabFile, vfs);
    engine.globalObject().setProperty("readTab", readTabFn);

    QScriptValue addFileFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)addFile, writer.data());
    engine.globalObject().setProperty("addFile", addFileFn);

    QFile scriptFile("scripts/converter.js");

    if (!scriptFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qFatal("Unable to open converter script: scripts/converter.js");
    }

    QString scriptCode = scriptFile.readAll();

    engine.evaluate(scriptCode, "scripts/converter.js");

    convertPrototypes(writer.data(), &engine);

    writer->close();
}
