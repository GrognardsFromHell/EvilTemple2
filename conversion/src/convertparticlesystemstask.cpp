#include <QDomDocument>
#include <QDomElement>

#include <virtualfilesystem.h>
#include <messagefile.h>

#include "conversion/convertparticlesystemstask.h"

using namespace Troika;

ConvertParticleSystemsTask::ConvertParticleSystemsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

uint ConvertParticleSystemsTask::cost() const
{
    return 2;
}

QString ConvertParticleSystemsTask::description() const
{
    return "Converting particle systems";
}

static QDomElement makeVector(QDomDocument &document, const QString &name, const QByteArray &x, const QByteArray &y, const QByteArray &z,
                       bool omitEmpty = true, bool omitZeros = false) {
    QDomElement element = document.createElement(name);
    if ((!omitZeros || QString(x) != "0") && (!omitEmpty || !x.isEmpty())) {
        element.setAttribute("x", QString(x));
    }
    if ((!omitZeros || QString(y) != "0") && (!omitEmpty || !y.isEmpty())) {
        element.setAttribute("y", QString(y));
    }
    if ((!omitZeros || QString(z) != "0") && (!omitEmpty || !z.isEmpty())) {
        element.setAttribute("z", QString(z));
    }
    return element;
}

static bool isZeroOrEmpty(const QByteArray &value) {
    return value.trimmed().isEmpty() || value.trimmed() == "0";
}

static void convertParticleSystemEmitter(QDomDocument &document, QDomElement &particleSystem,
                                  const QList<QByteArray> &sections)
{
    QDomElement emitter = document.createElement("emitter");
    particleSystem.appendChild(emitter);

    emitter.setAttribute("name", QString(sections[1]));

    if (!isZeroOrEmpty(sections[2]))
        emitter.setAttribute("delay", QString(sections[2]));

    // Point emitters are standard. "Model Vert" is also possible, but no idea what it does
    QString type(sections[3]);
    if (!type.isEmpty() && type.toLower() != "point")
        emitter.setAttribute("type", type);

    QString lifespan(sections[4].trimmed().toLower());
    if (lifespan != "perm")
        emitter.setAttribute("lifespan", lifespan);

    QDomElement particles = document.createElement("particles");

    particles.setAttribute("rate", QString(sections[5]));

    if (!sections[11].isEmpty())
        particles.setAttribute("type", QString(sections[11]));
    if (!sections[12].isEmpty())
        particles.setAttribute("coordinateSystem", QString(sections[12]));
    //if (!sections[13].isEmpty())
    //    particles.setAttribute("positionCoordinates", QString(sections[13]));
    //if (!sections[14].isEmpty())
    //    particles.setAttribute("velocityCoordinates", QString(sections[14]));

    if (!sections[15].isEmpty())
        particles.setAttribute("material", "particles/" + QString(sections[15]) + ".tga");
    lifespan = QString(sections[16]).trimmed().toLower();
    if (!lifespan.isEmpty() && lifespan != "perm")
        particles.setAttribute("lifespan", lifespan);
    if (!sections[20].isEmpty())
        particles.setAttribute("model", "meshes/particle/" + QString(sections[20]) + ".model");

    if (!sections[17].isEmpty() && sections[17].toLower() != "add")
        emitter.setAttribute("blendMode", QString(sections[17]));
    if (!sections[6].isEmpty())
        emitter.setAttribute("boundingSphereRadius", QString(sections[6]));
    if (!sections[7].isEmpty())
        emitter.setAttribute("space", QString(sections[7]));
    if (!sections[8].isEmpty())
        emitter.setAttribute("spaceNode", QString(sections[8]));
    if (!sections[9].isEmpty())
        emitter.setAttribute("coordinateSystem", QString(sections[9]));
    if (!sections[10].isEmpty())
        emitter.setAttribute("offsetCoordinateSystem", QString(sections[10]));
    // if (!sections[18].isEmpty() && sections[18] != "0")
    //    emitter.setAttribute("bounce", QString(sections[18]));
    if (!sections[19].isEmpty())
        emitter.setAttribute("animationSpeed", QString(sections[19]));
    if (!sections[21].isEmpty())
        emitter.setAttribute("animation", QString(sections[21]));

    QDomElement element;

    // Emitter parameters
    if (!isZeroOrEmpty(sections[22]) || !isZeroOrEmpty(sections[23]) || !isZeroOrEmpty(sections[24])) {
        element = makeVector(document, "acceleration", sections[22], sections[23], sections[24], true, true);
        emitter.appendChild(element);
    }

    if (!isZeroOrEmpty(sections[25]) || !isZeroOrEmpty(sections[26]) || !isZeroOrEmpty(sections[27])) {
        element = makeVector(document, "velocity", sections[25], sections[26], sections[27], true, true);
        emitter.appendChild(element);
    }

    if (!isZeroOrEmpty(sections[28]) || !isZeroOrEmpty(sections[29]) || !isZeroOrEmpty(sections[30])) {
        element = makeVector(document, "position", sections[28], sections[29], sections[30], true, true);
        emitter.appendChild(element);
    }

    if (!isZeroOrEmpty(sections[31]) || !isZeroOrEmpty(sections[32]) || !isZeroOrEmpty(sections[33])) {
        element = document.createElement("rotation");
        if (!isZeroOrEmpty(sections[31]))
            element.setAttribute("yaw", QString(sections[31]));
        if (!isZeroOrEmpty(sections[32]))
            element.setAttribute("pitch", QString(sections[32]));
        if (!isZeroOrEmpty(sections[33]))
            element.setAttribute("roll", QString(sections[33]));
        emitter.appendChild(element);
    }

    // Scale was always the same for all components in all emitters
    // This value is used below to provide a default fallback for the particle scale
    if (!isZeroOrEmpty(sections[34]) || !isZeroOrEmpty(sections[35]) || !isZeroOrEmpty(sections[36])) {
        Q_ASSERT(sections[34] == sections[35] && sections[35] == sections[36]);
    }

    if (!isZeroOrEmpty(sections[37]) || !isZeroOrEmpty(sections[38]) || !isZeroOrEmpty(sections[39])) {
        element = makeVector(document, "offset", sections[37], sections[38], sections[39], true, true);
        emitter.appendChild(element);
    }

    if (!isZeroOrEmpty(sections[40]) || !isZeroOrEmpty(sections[41]) || !isZeroOrEmpty(sections[42])) {
        element = makeVector(document, "initialVelocity", sections[40], sections[41], sections[42], true, true);
        emitter.appendChild(element);
    }

    /**
      It is unclear what the purpose of this is.
      We use it as default values for the particle color that is defined below.
      */
    // element = document.createElement("initialColor");
    // element.setAttribute("red", QString(sections[44]));
    // element.setAttribute("green", QString(sections[45]));
    // element.setAttribute("blue", QString(sections[46]));
    // element.setAttribute("alpha", QString(sections[43]));
    // emitter.appendChild(element);

    // Particle parameters
    emitter.appendChild(particles);

    if (!isZeroOrEmpty(sections[47]) || !isZeroOrEmpty(sections[48]) || !isZeroOrEmpty(sections[49])) {
        element = makeVector(document, "acceleration", sections[47], sections[48], sections[49]);
        particles.appendChild(element);
    }

    if (!isZeroOrEmpty(sections[50]) || !isZeroOrEmpty(sections[51]) || !isZeroOrEmpty(sections[52])) {
        element = makeVector(document, "velocity", sections[50], sections[51], sections[52]);

        // Add coordinate-system type
        if (!sections[14].isEmpty() && sections[14].toLower() != "cartesian") {
            element.setAttribute("coordinates", QString(sections[14].toLower()));
        }

        particles.appendChild(element);
    }

    if (!isZeroOrEmpty(sections[53]) || !isZeroOrEmpty(sections[54]) || !isZeroOrEmpty(sections[55])) {
        element = makeVector(document, "position", sections[53], sections[54], sections[55]);

        // Add coordinate-system type
        if (!sections[13].isEmpty() && sections[13].toLower() != "cartesian") {
            element.setAttribute("coordinates", QString(sections[13].toLower()));
        }

        particles.appendChild(element);
    }

    if (!sections[56].isEmpty()) {
        QDomElement scaleChild = document.createElement("scale");
        scaleChild.appendChild(document.createTextNode(sections[56].trimmed()));
        particles.appendChild(scaleChild);
    } else if (!isZeroOrEmpty(sections[34])) {
        // Fall back to default scale from emitter
        QDomElement scaleChild = document.createElement("scale");
        scaleChild.appendChild(document.createTextNode(sections[34].trimmed()));
        particles.appendChild(scaleChild);
    }

    if (!isZeroOrEmpty(sections[59]) || !isZeroOrEmpty(sections[60]) || !isZeroOrEmpty(sections[61])) {
        element = document.createElement("rotation");
        if (!isZeroOrEmpty(sections[59]))
            element.setAttribute("yaw", QString(sections[59]));
        if (!isZeroOrEmpty(sections[60]))
            element.setAttribute("pitch", QString(sections[60]));
        if (!isZeroOrEmpty(sections[61]))
            element.setAttribute("roll", QString(sections[61]));
        particles.appendChild(element);
    }

    QByteArray colorRed = "255";
    QByteArray colorGreen = "255";
    QByteArray colorBlue = "255";
    QByteArray colorAlpha = "255";

    // Base emitter color?
    if (!isZeroOrEmpty(sections[44])) {
        colorRed = sections[44];
    }
    if (!isZeroOrEmpty(sections[45])) {
        colorGreen = sections[45];
    }
    if (!isZeroOrEmpty(sections[46])) {
        colorBlue = sections[46];
    }
    if (!isZeroOrEmpty(sections[43])) {
        colorAlpha = sections[43];
    }

    // Particle override colors ?
    if (!isZeroOrEmpty(sections[63])) {
        colorRed = sections[63];
    }
    if (!isZeroOrEmpty(sections[64])) {
        colorGreen = sections[64];
    }
    if (!isZeroOrEmpty(sections[65])) {
        colorBlue = sections[65];
    }
    if (!isZeroOrEmpty(sections[62])) {
        colorAlpha = sections[62];
    }

    QRegExp bracketRemover("\\(.+\\)");
    bracketRemover.setMinimal(true);

    element = document.createElement("color");
    element.setAttribute("red", QString(colorRed).replace(bracketRemover, ""));
    element.setAttribute("green", QString(colorGreen).replace(bracketRemover, ""));
    element.setAttribute("blue", QString(colorBlue).replace(bracketRemover, ""));
    element.setAttribute("alpha", QString(colorAlpha).replace(bracketRemover, ""));
    particles.appendChild(element);

    if (!sections[66].isEmpty()) {
        element = document.createElement("attractorBlend");
        element.appendChild(document.createTextNode(sections[66]));
        particles.appendChild(element);
    }
}

static void convertParticleSystem(QDomDocument &document,
                           QDomElement &particleSystems,
                           const QString &id,
                           const QList<QList<QByteArray> > &emitters)
{
    QDomElement particleSystem = document.createElement("particleSystem");
    particleSystems.appendChild(particleSystem);

    particleSystem.setAttribute("id", id);

    // Convert all emitters
    foreach (const QList<QByteArray> &sections, emitters) {
        convertParticleSystemEmitter(document, particleSystem, sections);
    }
}

void ConvertParticleSystemsTask::run()
{
    qDebug("Converting particle systems.");

    Troika::VirtualFileSystem *vfs = service()->virtualFileSystem();

    QByteArray particleSystemData = vfs->openFile("rules/partsys0.tab")
                                    .append('\n')
                                    .append(vfs->openFile("rules/partsys1.tab"))
                                    .append('\n')
                                    .append(vfs->openFile("rules/partsys2.tab"));

    QDomDocument particleSystemsDoc;
    QDomElement particleSystems = particleSystemsDoc.createElement("particleSystems");
    particleSystemsDoc.appendChild(particleSystems);

    QList<QByteArray> lines = particleSystemData.split('\n');
    QHash<QByteArray, QList<QList<QByteArray> > > groupedEmitters; // Emitters grouped by particle system id

    foreach (QByteArray line, lines) {
        if (line.trimmed().isEmpty())
            continue; // Skip empty lines.

        line.replace((char)11, ""); // Replace "virtual tabs"

        QList<QByteArray> sections = line.split('\t');

        groupedEmitters[sections[0]].append(sections);
    }

    foreach (const QByteArray &particleSystemId, groupedEmitters.keys()) {
        service()->addParticleSystem(particleSystemId);

        convertParticleSystem(particleSystemsDoc, particleSystems,
                              particleSystemId, groupedEmitters[particleSystemId]);
    }

    QScopedPointer<IFileWriter> writer(service()->createOutput("particles"));
    writer->addFile("particles/templates.xml", particleSystemsDoc.toByteArray());

    // Copy over a list of necessary textures
    QFile particleFiles(":/particlefiles.txt");

    if (!particleFiles.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qWarning("Missing list of additional particle files.");
    }

    QStringList particleFileList = QString(particleFiles.readAll()).split('\n');

    foreach (QString particleFile, particleFileList) {
        particleFile = particleFile.trimmed();

        if (particleFile.startsWith('#') || particleFile.isEmpty())
            continue;

        QByteArray content = vfs->openFile("art/meshes/particle/" + particleFile);

        if (!content.isNull()) {
            writer->addFile("particles/" + particleFile, content);
        } else {
            qWarning("Missing file: %s", qPrintable(particleFile));
        }
    }

    writer->close();
}
