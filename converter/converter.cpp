
#include <QtGlobal>
#include <QtXml>
#include <QDataStream>
#include <QScopedPointer>
#include <QFileInfo>
#include <QTextStream>
#include <QBuffer>

#include <iostream>

#include "dagreader.h"
#include "zipwriter.h"
#include "virtualfilesystem.h"
#include "messagefile.h"
#include "troikaarchive.h"
#include "prototypes.h"
#include "materials.h"
#include "zonetemplate.h"
#include "zonetemplatereader.h"
#include "zonetemplates.h"
#include "zonebackgroundmap.h"
#include "skmreader.h"
#include "model.h"
#include "material.h"
#include "exclusions.h"
#include "mapconverter.h"
#include "prototypeconverter.h"

#include "util.h"
#include "converter.h"
#include "materialconverter.h"
#include "interfaceconverter.h"
#include "modelwriter.h"

void printEventTypes();

/**
  The original game applies an additional base rotation to everything in order to align it
  with the isometric grid. This is the radians value of that rotation.
  */
const float LegacyBaseRotation = 0.77539754f;

struct MeshReference {
    MeshReference() : meshId(0), source(0) {}

    enum Sources {
        MeshesMes = 0x1,
        StaticGeometry = 0x2, // from a map
		AddMesh = 0x4, // Added from addmesh.mes
        FORCE_DWORD = 0x7FFFFFFF, // force 32-bit size (whatever)
    };

    uint meshId; // From meshes.mes
    uint source; // bitfield
};

class ConverterData {
public:
    static const int maxArchives = 10; // Try ToEE0.dat to ToEE(maxArchives-1).dat

    QString mInputPath, mOutputPath;

    QScopedPointer<Troika::VirtualFileSystem> vfs;

    QScopedPointer<Troika::Prototypes> prototypes;

    QScopedPointer<Troika::ZoneTemplates> zoneTemplates;

    QScopedPointer<Troika::Materials> materials;

    QScopedPointer<ZipWriter> commonFiles;

    QHash<uint,QString> particleSystemHashes;

	QHash<QString,bool> mWrittenTextures;

	QHash<QString,bool> mWrittenMaterials;

	bool external;

    // Maps lower-case mesh filenames (normalized separators) to an information structure
    QHash<QString, MeshReference> meshReferences;

    Exclusions exclusions;

    ConverterData(const QString &inputPath, const QString &outputPath) :
            mInputPath(inputPath), mOutputPath(outputPath), external(false)
    {
        // Force both paths to be in the system specific format and end with a separator.
        mInputPath = QDir::toNativeSeparators(mInputPath);
        if (!mInputPath.endsWith(QDir::separator()))
            mInputPath.append(QDir::separator());

        mOutputPath = QDir::toNativeSeparators(mOutputPath);
        if (!mOutputPath.endsWith(QDir::separator()))
            mOutputPath.append(QDir::separator());
        
        exclusions.load();
    }

    void loadArchives() {
        // Add base archives
        for (int i = 0; i < maxArchives; ++i) {
            QString archivePath = QString("%1ToEE%2.dat").arg(mInputPath).arg(i);

            if (QFile::exists(archivePath)) {
                qDebug("Adding archive %s", qPrintable(archivePath));
                vfs->add(new Troika::TroikaArchive(archivePath, vfs.data()));
            }
        }

        QString modulePath = QString("%1modules%2ToEE.dat").arg(mInputPath).arg(QDir::separator());
        qDebug("Adding archive %s", qPrintable(modulePath));
        vfs->add(new Troika::TroikaArchive(modulePath, vfs.data()));
    }

    bool convertMaps()
    {
        ZipWriter writer(mOutputPath + "maps.zip");
        MapConverter converter(vfs.data(), &writer);

        // Convert all maps
        foreach (quint32 mapId, zoneTemplates->mapIds()) {
            QScopedPointer<Troika::ZoneTemplate> zoneTemplate(zoneTemplates->load(mapId));

            converter.convert(zoneTemplate.data());

            if (zoneTemplate) {
                Troika::ZoneBackgroundMap *background = zoneTemplate->dayBackground();

                if (background) {
                    qDebug("Loaded map %d. Name: %s. Dir: %s", zoneTemplate->id(),
                           qPrintable(zoneTemplate->name()), qPrintable(background->directory()));
                } else {
                    qWarning("Zone has no daylight background: %d.", mapId);
                }

                QByteArray objectPosData;
                QTextStream objectPosStream(&objectPosData, QIODevice::WriteOnly);

                // Add all static geometry files to the list of referenced models
                foreach (Troika::GeometryObject *object, zoneTemplate->staticGeometry()) {
                    QVector3D objectPos = object->position();
                    const char SPACE = ' ';
                    objectPosStream << objectPos.x() << SPACE << objectPos.y() << SPACE << objectPos.z() << SPACE
                            << getNewModelFilename(object->mesh()) << SPACE
                            << object->rotation().x() << SPACE << object->rotation().y() << SPACE << object->rotation().z() << SPACE
                            << object->rotation().scalar() << SPACE
                            << object->scale().x() << SPACE << object->scale().y() << SPACE << object->scale().z() << endl;

                    MeshReference &reference = meshReferences[QDir::toNativeSeparators(object->mesh()).toLower()];
                    reference.source |= MeshReference::StaticGeometry;
                }

                objectPosStream.flush();

                writer.addFile(zoneTemplate->directory() + "staticGeometry.txt", objectPosData, 9);

                convertStaticObjects(zoneTemplate.data(), &writer);

                convertLighting(zoneTemplate.data(), &writer);

                convertClippingMeshes(zoneTemplate.data(), &writer);

                convertParticleSystemInstances(zoneTemplate.data(), &writer);
            } else {
                qWarning("Unable to load zone template for map id %d.", mapId);
            }
        }

        writer.close();

        return true;
    }

    void convertStaticObjects(ZoneTemplate *zoneTemplate, ZipWriter *writer)
    {
        QHash<uint,QString> descriptions = MessageFile::parse(vfs->openFile("mes/description.mes"));

        QByteArray data;
        QXmlStreamWriter xml(&data);
        xml.setAutoFormatting(true);

        xml.writeStartDocument("1.0");

        xml.writeStartElement("objects");

        QList<GameObject> objects;
        objects.append(zoneTemplate->staticObjects());
        objects.append(zoneTemplate->mobiles());

        foreach (const GameObject &object, objects) {
            if (object.descriptionId.isDefined())
                xml.writeComment(descriptions[object.descriptionId.value()]);
            else if (object.prototype->descriptionId)
                xml.writeComment(descriptions[object.prototype->descriptionId]);

            xml.writeStartElement("object");

            if (!object.id.isNull())
                xml.writeAttribute("id", object.id);

            xml.writeAttribute("prototype", QString("%1").arg(object.prototype->id));

            xml.writeEmptyElement("position");
            xml.writeAttribute("x", QString("%1").arg(object.position.x()));
            xml.writeAttribute("y", QString("%1").arg(object.position.y()));
            xml.writeAttribute("z", QString("%1").arg(object.position.z()));

            PropertyWriter props(xml);
            props.write("name", object.name);
            props.write("flags", object.flags);
            props.write("scale", object.scale);
            props.write("rotation", object.rotation);
            props.write("radius", object.radius);
            props.write("height", object.renderHeight);
            props.write("sceneryFlags", object.sceneryFlags);
            props.write("descriptionId", object.descriptionId);
            props.write("secretDoorFlags", object.secretDoorFlags);
            props.write("portalFlags", object.portalFlags);
            props.write("portalLockDc", object.portalLockDc);
            props.write("teleportTarget", object.teleportTarget);
            props.write("parentItemId", object.parentItemId);
            props.write("substituteInventoryId", object.substituteInventoryId);
            props.write("itemInventoryLocation", object.itemInventoryLocation);
            props.write("hitPoints", object.hitPoints);
            props.write("hitPointsAdjustment", object.hitPointsAdjustment);
            props.write("hitPointsDamage", object.hitPointsDamage);
            props.write("walkSpeedFactor", object.walkSpeedFactor);
            props.write("runSpeedFactor", object.runSpeedFactor);
            props.write("dispatcher", object.dispatcher);
            props.write("secretDoorEffect", object.secretDoorEffect);
            props.write("notifyNpc", object.notifyNpc);
            props.write("containerFlags", object.containerFlags);
            props.write("containerLockDc", object.containerLockDc);
            props.write("containerKeyId", object.containerKeyId);
            props.write("containerInventoryId", object.containerInventoryId);
            props.write("containerInventoryListIndex", object.containerInventoryListIndex);
            props.write("containerInventorySource", object.containerInventorySource);
            props.write("itemFlags", object.itemFlags);
            props.write("itemWeight", object.itemWeight);
            props.write("itemWorth", object.itemWorth);
            props.write("itemQuantity", object.itemQuantity);
            props.write("weaponFlags", object.weaponFlags);
            props.write("ammoQuantity", object.ammoQuantity);
            props.write("armorFlags", object.armorFlags);
            props.write("armorAcAdjustment", object.armorAcAdjustment);
            props.write("armorMaxDexBonus", object.armorMaxDexBonus);
            props.write("armorCheckPenalty", object.armorCheckPenalty);
            props.write("moneyQuantity", object.moneyQuantity);
            props.write("keyId", object.keyId);
            props.write("critterFlags", object.critterFlags);
            props.write("critterFlags2", object.critterFlags2);
            props.write("critterRace", object.critterRace);
            props.write("critterGender", object.critterGender);
            props.write("critterMoneyIndex", object.critterMoneyIndex);
            props.write("critterInventoryNum", object.critterInventoryNum);
            props.write("critterInventorySource", object.critterInventorySource);
            props.write("npcFlags", object.npcFlags);

            xml.writeEndElement();

            xml.writeCharacters("\n\n    "); // For better readability
        }

        xml.writeEndElement();

        xml.writeEndDocument();

        writer->addFile(zoneTemplate->directory() + "staticObjects.xml", data, 9);
    }

	void convertLighting(ZoneTemplate *zoneTemplate, ZipWriter *writer)
	{
		QDomDocument document;
		QDomElement root = document.createElement("lighting");
		document.appendChild(root);

		// Insert global lighting first
		QDomElement globalLight = document.createElement("global");
		root.appendChild(globalLight);
		QDomElement color = document.createElement("color");
		color.setAttribute("red", zoneTemplate->globalLight().r);
		color.setAttribute("green", zoneTemplate->globalLight().g);
		color.setAttribute("blue", zoneTemplate->globalLight().b);
		globalLight.appendChild(color);

		QDomElement direction = document.createElement("direction");
		direction.setAttribute("x", zoneTemplate->globalLight().dirX);
		direction.setAttribute("y", zoneTemplate->globalLight().dirY);
		direction.setAttribute("z", zoneTemplate->globalLight().dirZ);
		globalLight.appendChild(direction);

		QDomElement lightSources = document.createElement("lightSources");
		root.appendChild(lightSources);

		foreach (const Light &light, zoneTemplate->lights())
		{
			QDomElement element;
			switch (light.type) {
			case 1:
				element = document.createElement("pointLight");
				break;
			case 2:
				element = document.createElement("spotLight");
				break;
			case 3:
				element = document.createElement("directionalLight");				
				break;
			default:
				qWarning("Invalid light type: %d.", light.type);
				continue;
			}

			element.setAttribute("range", light.range);
			if (light.type != 3) {
				// ToEE does not allow custom attenuation per light. We might add this later, so convert it here and add it as a modifiable attribute
				element.setAttribute("attenuation", 4 / (light.range * light.range));
			}
			if (light.type == 2) {
				element.setAttribute("phi", light.phi);
				element.setAttribute("theta", light.phi * 0.6f);
			}
			
			QDomElement pos = document.createElement("position");
			element.appendChild(pos);
			pos.setAttribute("x", light.position.x());
			pos.setAttribute("y", light.position.y());
			pos.setAttribute("z", light.position.z());

			// Makes only sense for spot/directional, right?
			if (light.type != 1) {
				QDomElement dir = document.createElement("direction");
				element.appendChild(dir);
				dir.setAttribute("x", light.dirX);
				dir.setAttribute("y", light.dirY);
				dir.setAttribute("z", light.dirZ);
			}

			if (light.r > 0 || light.g > 0 || light.b > 0) {
				QDomElement color = document.createElement("color");
				element.appendChild(color);
				color.setAttribute("red", light.r);
				color.setAttribute("green", light.g);
				color.setAttribute("blue", light.b);
			}

			/**
				This seems to be truly unused.
				if (light.ur > 0 || light.ug > 0 || light.ub > 0) {
					color = document.createElement("specularColor");
					element.appendChild(color);
					color.setAttribute("red", light.ur);
					color.setAttribute("green", light.ug);
					color.setAttribute("blue", light.ub);
				}
			*/
			
			lightSources.appendChild(element);
		}

		writer->addFile(zoneTemplate->directory() + "lighting.xml", document.toByteArray(), 9);
	}
    
    bool compareScale(const Vector4 &a, const Vector4 &b) {
        float scaleEpsilon = 0.0001f;

        Vector4 diff = (b - a).absolute();

        return (diff.x() <= scaleEpsilon && diff.y() <= scaleEpsilon && diff.z() <= scaleEpsilon);
    }

    void convertClippingMeshes(ZoneTemplate *zoneTemplate, ZipWriter *writer)
    {
        QByteArray clippingData;
        QDataStream clippingStream(&clippingData, QIODevice::WriteOnly);
        clippingStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        clippingStream.setByteOrder(QDataStream::LittleEndian);

        // Create a list of all clipping geometry meshes
        QStringList clippingGeometryFiles;
        QList< QList<Vector4> > scalingPerFile;
        foreach (GeometryObject *object, zoneTemplate->clippingGeometry()) {
            QString normalizedFilename = QDir::toNativeSeparators(object->mesh()).toLower();

            if (!clippingGeometryFiles.contains(normalizedFilename)) {
                clippingGeometryFiles.append(normalizedFilename);
                scalingPerFile.append(QList<Vector4>());
            }
        }
        
        /**
        For each geometry file we will collect all the scales in which it is used. 
        This is important since ToEE uses non-uniform scaling here (it scales in 2d space),
        and we want to remove that scaling entirely. Instead, we will create a copy of the geometry
        for each scaling level used.
          */
        foreach (GeometryObject *object, zoneTemplate->clippingGeometry()) {
            QString normalizedFilename = QDir::toNativeSeparators(object->mesh()).toLower();
            int fileIndex = (uint)clippingGeometryFiles.indexOf(normalizedFilename);

            Vector4 scale(object->scale().x(), object->scale().y(), object->scale().z(), 1);

            bool scaleExists = false;
            foreach (Vector4 existingScale, scalingPerFile[fileIndex]) {
                if (compareScale(existingScale, scale)) {
                    scaleExists = true;
                    break;
                }
            }

            if (!scaleExists)
                scalingPerFile[fileIndex].append(scale);
        }

        uint totalFileCount = 0;

        // Now we test if for any geometry object there are more than one scale versions
        for (int i = 0; i < clippingGeometryFiles.size(); ++i) {
            totalFileCount += scalingPerFile[i].size();
            if (scalingPerFile[i].size() > 1) {
                qDebug("Clipping geometry file %s is used in %d different scales:", qPrintable(clippingGeometryFiles[i]), scalingPerFile[i].size());
            }
        }

        // File header
        clippingStream << totalFileCount << (uint)zoneTemplate->clippingGeometry().size();             


        /**
        These transformations come from the original game and are *constant*.
        **/
        // Old: -44
        Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, -0.77539754f);
        Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

        // Old: 90-135
        Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, 2.3561945f);
        Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

        Matrix4 baseView = rotate1matrix * rotate2matrix;
        Matrix4 baseViewInverse = baseView.inverted();

        for (int i = 0; i < clippingGeometryFiles.size(); ++i) {
            Troika::DagReader reader(vfs.data(), clippingGeometryFiles[i]);
            MeshModel *model = reader.get();

            Q_ASSERT(model->faceGroups().size() == 1);
            Q_ASSERT(model->vertices().size() > 0);

            /**
              Calculate the scaling matrix for each instance and transform the vertices ahead-of-time.
              */
            foreach (Vector4 scale, scalingPerFile[i]) {
                QVector<Vector4> scaledVertices(model->vertices().size());
                
                // Switch z/y
                Matrix4 scaleMatrix2d = Matrix4::scaling(scale.x(), scale.z(), scale.y());
                Matrix4 scaleMatrix = baseViewInverse * scaleMatrix2d * baseView;
                
                for (int j = 0; j < scaledVertices.size(); ++j) {
                    const Troika::Vertex &vertex = model->vertices()[j];
                    scaledVertices[j] = scaleMatrix.mapPosition(Vector4(vertex.positionX, vertex.positionY, vertex.positionZ, 1));
                }

                Box3d boundingBox(scaledVertices[0], scaledVertices[0]);

                Vector4 firstVertex = scaledVertices[0];
                firstVertex.setW(0);
                float originDistance = firstVertex.length();
                float originDistanceSquared = firstVertex.lengthSquared();

                for (int j = 1; j < scaledVertices.size(); ++j) {
                    Vector4 vertex = scaledVertices[j];
                    boundingBox.merge(vertex);
                    vertex.setW(0);
                    originDistance = qMax<float>(originDistance, vertex.length());
                    originDistanceSquared = qMax<float>(originDistance, vertex.lengthSquared());
                }

                clippingStream << boundingBox.minimum().x() << boundingBox.minimum().y() << boundingBox.minimum().z()
                        << boundingBox.minimum().w() << boundingBox.maximum().x() << boundingBox.maximum().y()
                        << boundingBox.maximum().z() << boundingBox.maximum().w() << originDistance << originDistanceSquared;

                clippingStream << (uint)scaledVertices.size() << (uint)model->faceGroups()[0]->faces().size() * 3;

                // We should apply the scaling here.

                foreach (const Vector4 &vertex, scaledVertices) {
                    clippingStream << vertex.x() << vertex.y() << vertex.z() << vertex.w();
                }

                foreach (const Face &face, model->faceGroups()[0]->faces()) {
                    clippingStream << face.vertices[0] << face.vertices[1] << face.vertices[2];
                }
            }
        }

        // Instances
        foreach (GeometryObject *object, zoneTemplate->clippingGeometry()) {
            QString normalizedFilename = QDir::toNativeSeparators(object->mesh()).toLower();

            uint fileNameIndex = (uint)clippingGeometryFiles.indexOf(normalizedFilename);
            uint fileIndex = 0;

            // Find the filename/scale pair appropriate for this instance
            for (uint i = 0; i < fileNameIndex; ++i)
                fileIndex += scalingPerFile[i].size();

            bool found = false;
            
            Vector4 scale(object->scale().x(), object->scale().y(), object->scale().z(), 1);
            for (int i = 0; i < scalingPerFile[fileNameIndex].size(); ++i) {
                Vector4 existingScale = scalingPerFile[fileNameIndex][i];
                if (compareScale(existingScale, scale)) {
                    found = true;
                    break;
                }                    
                fileIndex++;
            }

            Q_ASSERT(found);

            clippingStream << object->position().x() << object->position().y() << object->position().z() << (float)1
                    << object->rotation().x() << object->rotation().y() << object->rotation().z()
                    << object->rotation().scalar()
                    << (float)1 << (float)1 << (float)1 << (float)1
                    << fileIndex;

        }

        writer->addFile(zoneTemplate->directory() + "clippingGeometry.dat", clippingData, 9);
    }

    void convertParticleSystemInstances(const ZoneTemplate *zoneTemplate, ZipWriter *writer)
    {
        QByteArray particleSystems;
        QDataStream stream(&particleSystems, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        foreach (const ParticleSystem &particleSystem, zoneTemplate->particleSystems()) {
            if (!particleSystemHashes.contains(particleSystem.hash)) {
                qWarning("Found particle-system with unknown hash id: %d", particleSystem.hash);
                continue;
            }

            const Vector4 &pos = particleSystem.light.position;
            stream << pos.x() << pos.y() << pos.z() << particleSystem.light.range << particleSystemHashes[particleSystem.hash];
        }

        writer->addFile(zoneTemplate->directory() + "particleSystems.txt", particleSystems, 9);
    }
	
    QString getNewModelFilename(const QString &modelFilename) {
        QString newFilename = QDir::toNativeSeparators(modelFilename);
        if (newFilename.startsWith(QString("art") + QDir::separator(), Qt::CaseInsensitive)) {
            newFilename = newFilename.right(newFilename.length() - 4);
        }

        if (newFilename.endsWith(".skm", Qt::CaseInsensitive) || newFilename.endsWith(".ska", Qt::CaseInsensitive))
        {
            newFilename = newFilename.left(newFilename.length() - 4);
        }

        newFilename.append(".model");

        return newFilename;
    }

    bool openInput() {
        vfs.reset(new Troika::VirtualFileSystem());
        loadArchives();

        prototypes.reset(new Troika::Prototypes(vfs.data()));
        zoneTemplates.reset(new Troika::ZoneTemplates(vfs.data(), prototypes.data()));
        materials.reset(new Troika::Materials(vfs.data()));

        // Validate archives here?

        return true;
    }

    bool createOutputDirectory() {
        QDir outputDir(mOutputPath);
        if (!outputDir.exists()) {
            qDebug("Creating output directory %s.", qPrintable(mOutputPath));
            if (QDir::isAbsolutePath(mOutputPath)) {
                return QDir::root().mkpath(mOutputPath);
            } else {
                return QDir::current().mkpath(mOutputPath);
            }
        } else {
            return true;
        }
    }

    /**
      The interface files (*.tga and *.img in art/interface/) are all converted
      to consecutive PNG-24 images.
      */
    bool convertInterface() {

        ZipWriter zip(mOutputPath + "interface.zip");

        InterfaceConverter converter(&zip, vfs.data());

        return converter.convert();
    }

    /**
      Converts the particle systems.
      */
    bool convertParticleSystems() {
        qDebug("Converting particle systems.");

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
            particleSystemHashes[ELFHash(particleSystemId)] = particleSystemId;

            convertParticleSystem(particleSystemsDoc, particleSystems,
                                  particleSystemId, groupedEmitters[particleSystemId]);
        }

        ZipWriter writer(mOutputPath + "particles.zip");
        writer.addFile("particles/templates.xml", particleSystemsDoc.toByteArray(), 9);

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
                writer.addFile("particles/" + particleFile, content, 9);
            } else {
                qWarning("Missing file: %s", qPrintable(particleFile));
            }
        }

        return true;
    }

    void convertParticleSystem(QDomDocument &document,
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

    QDomElement makeVector(QDomDocument &document, const QString &name, const QByteArray &x, const QByteArray &y, const QByteArray &z,
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

    bool isZeroOrEmpty(const QByteArray &value) {
        return value.trimmed().isEmpty() || value.trimmed() == "0";
    }

    void convertParticleSystemEmitter(QDomDocument &document, QDomElement &particleSystem,
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
            particles.setAttribute("model", "particles/" + QString(sections[20]) + ".model");

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

    void addMeshesMesReferences()
    {
        QHash<quint32, QString> meshesIndex = Troika::MessageFile::parse(vfs->openFile("art/meshes/meshes.mes"));

        foreach (quint32 meshId, meshesIndex.keys()) {
            QString meshFilename = QDir::toNativeSeparators("art/meshes/" + meshesIndex[meshId] + ".skm");

            MeshReference &reference = meshReferences[meshFilename.toLower()];
            reference.meshId = meshId;
            reference.source |= MeshReference::MeshesMes;
        }
    }

	void addAddMeshesMesReferences()
    {
        QHash<quint32, QString> meshesIndex = Troika::MessageFile::parse(vfs->openFile("rules/addmesh.mes"));

        foreach (quint32 meshId, meshesIndex.keys()) {
			QString filename = meshesIndex[meshId].trimmed();

			if (filename.isEmpty())
				continue;

            MeshReference &reference = meshReferences[filename];
			reference.source |= MeshReference::AddMesh;
        }
    }

    void convertModels()
    {        
        ZipWriter zip(mOutputPath + "meshes.zip");

        // Convert all valid meshes in meshes.mes
        addMeshesMesReferences();
		addAddMeshesMesReferences();

        foreach (const QString &meshFilename, meshReferences.keys()) {
            if (exclusions.isExcluded(meshFilename)) {
                qWarning("Skipping %s, since it's excluded.", qPrintable(meshFilename));
                continue;
            }

            Troika::SkmReader reader(vfs.data(), materials.data(), meshFilename);
            QScopedPointer<Troika::MeshModel> model(reader.get());

            if (!model) {
                qWarning("Unable to open model %s.", qPrintable(meshFilename));
                continue;
            }

            convertModel(&zip, meshFilename, model.data());
        }
    }       

    /**
      * Converts a single model, given its original filename.
      */
    bool convertModel(ZipWriter *zip, const QString &filename, Troika::MeshModel *model)
    {
        // This is a tedious process, but necessary. Create a total bounding box,
        // then create a bounding box for every animation of the mesh.
        QString newFilename = getNewModelFilename(filename);

        QByteArray modelData;
        QBuffer modelBuffer(&modelData);

        if (!modelBuffer.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
            qFatal("Unable to open a write buffer for the model.");
            return false;
        }

        QDataStream stream(&modelBuffer);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        writeModel(model, stream, zip);

        modelBuffer.close();

        return zip->addFile(newFilename, modelData, 9);
    }

	bool writeModel(Troika::MeshModel *model, QDataStream &stream, ZipWriter *zip)
    {
        ModelWriter writer(stream);

        QHash< QString, QSharedPointer<Troika::Material> > groupedMaterials;

        MaterialConverter converter(vfs.data());
		converter.setExternal(external);

        // Convert materials used by the model
        foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, model->faceGroups()) {
            if (!faceGroup->material())
                continue;
            groupedMaterials[faceGroup->material()->name()] = faceGroup->material();
        }

        QHash<QString,int> materialMapping;
		QHash<uint,QString> materialFileMapping;
        int i = 0;

        foreach (const QString &materialName, groupedMaterials.keys()) {
            qDebug("Converting %s.", qPrintable(materialName));
            converter.convert(groupedMaterials[materialName].data());
			
			materialFileMapping[i] = getNewMaterialFilename(materialName);
			materialMapping[materialName] = i++;
        }

		if (!external) {
			writer.writeTextures(converter.textureList());
			writer.writeMaterials(converter.materialList());
		} else {
			QStringList materials;
			for (int j = 0; j < i; ++j) {
				materials.append(materialFileMapping[j]);
			}
			writer.writeMaterialReferences(materials);

			foreach (const QString &filename, converter.materialScripts().keys()) {
				if (!mWrittenMaterials[filename]) {
					mWrittenMaterials[filename] = true;
					zip->addFile(filename, converter.materialScripts()[filename].data, 9);
				}
			}
			foreach (const QString &filename, converter.textures().keys()) {
				if (!mWrittenTextures[filename]) {
					mWrittenTextures[filename] = true;
					zip->addFile(filename, converter.textures()[filename].data, 9);
				}
			}
		}

        writer.writeVertices(model->vertices());
        writer.writeFaces(model->faceGroups(), materialMapping);

		writer.writeBones(model->skeleton());
		writer.writeBoneAttachments(model->vertices());

		writer.writeBoundingVolumes(model);

		if (model->skeleton()->animations().size() > 0) {
			writer.writeAnimations(model);
		}

        writer.finish();

        return true;
    }

    /**
      Converts scripts and global definition files.
      */
    void convertScripts()
    {
        ZipWriter writer(mOutputPath + "scripts.zip");

        convertPrototypes(&writer);

        writer.close();
    }

    void convertPrototypes(ZipWriter *writer)
    {
        PrototypeConverter converter(vfs.data());

        QByteArray result;
        QXmlStreamWriter xml(&result);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(4);

        converter.convertPrototypes(prototypes.data(), xml);

        writer->addFile("prototypes.xml", result, 9);
    }

    bool convert()
    {
        if (!openInput()) {
            qFatal("Unable to open Temple of Elemental Evil data files in %s.", qPrintable(mInputPath));
        }

        if (!createOutputDirectory()) {
            qFatal("Unable to create output directory %s.", qPrintable(mOutputPath));
        }

        convertScripts();

        convertParticleSystems();

        convertMaps();
		
        convertModels();               

        convertInterface();        

        return true;
    }

};

Converter::Converter(const QString &inputPath, const QString &outputPath) :
        d_ptr(new ConverterData(inputPath, outputPath))
{
}

Converter::~Converter()
{
}

bool Converter::convert()
{
    return d_ptr->convert();
}

void Converter::setExternal(bool ext)
{
	d_ptr->external = ext;
}

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    std::cout << "Conversion utility for Temple of Elemental Evil." << std::endl;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " " << "<install-dir>" << std::endl;
        return -1;
    }

    Converter converter(QString::fromLocal8Bit(argv[1]), QString("data/"));

	for (int i = 2; i < argc; ++i) {
		if (!strcmp(argv[i], "-external")) {
			converter.setExternal(true);
		}
	}

    if (!converter.convert()) {
        std::cout << "ERROR: Conversion failed." << std::endl;
        return -1;
    }

    return 0;
}
