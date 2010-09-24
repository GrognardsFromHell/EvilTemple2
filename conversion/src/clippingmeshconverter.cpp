
#include <zonetemplate.h>

#include "conversion/clippingmeshconverter.h"
#include "conversion/util.h"
#include "conversion/conversiontask.h"

using namespace Troika;

class ClippingMeshConverterData
{
public:
    ClippingMeshConverterData(IConversionService *service, const Troika::ZoneTemplate *zoneTemplate);

    IConversionService *mService;
    const Troika::ZoneTemplate *mZoneTemplate;
};

ClippingMeshConverter::ClippingMeshConverter(IConversionService *service, const Troika::ZoneTemplate *zoneTemplate)
    : d(new ClippingMeshConverterData(service, zoneTemplate))
{
}

ClippingMeshConverter::~ClippingMeshConverter()
{
}

ClippingMeshConverterData::ClippingMeshConverterData(IConversionService *service,
                                                     const Troika::ZoneTemplate *zoneTemplate)
    : mService(service), mZoneTemplate(zoneTemplate)
{
}

/**
  These properties become pre-baked.
  */
struct DagInstanceProperty {
    float scaleX;
    float scaleY;
    float scaleZ;
    float rotation;

    bool operator ==(const DagInstanceProperty &other) const
    {
        return qFuzzyCompare(scaleX, other.scaleX)
                && qFuzzyCompare(scaleY, other.scaleY)
                && qFuzzyCompare(scaleZ, other.scaleZ)
                && qFuzzyCompare(rotation, other.rotation);
    }
};

QByteArray ClippingMeshConverter::convert()
{
    QByteArray clippingData;
    QDataStream clippingStream(&clippingData, QIODevice::WriteOnly);
    clippingStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    clippingStream.setByteOrder(QDataStream::LittleEndian);

    // Create a list of all clipping geometry meshes
    QStringList clippingGeometryFiles;
    QList< QList<DagInstanceProperty> > instancesPerFile;
    foreach (GeometryObject *object, d->mZoneTemplate->clippingGeometry()) {
        QString normalizedFilename = normalizePath(object->mesh());

        if (!clippingGeometryFiles.contains(normalizedFilename)) {
            clippingGeometryFiles.append(normalizedFilename);
            instancesPerFile.append(QList<DagInstanceProperty>());
        }
    }

    /**
    For each geometry file we will collect all the scales in which it is used.
    This is important since ToEE uses non-uniform scaling here (it scales in 2d space),
    and we want to remove that scaling entirely. Instead, we will create a copy of the geometry
    for each scaling level used.
      */
    foreach (GeometryObject *object, d->mZoneTemplate->clippingGeometry()) {
        QString normalizedFilename = normalizePath(object->mesh());
        int fileIndex = (uint)clippingGeometryFiles.indexOf(normalizedFilename);

        DagInstanceProperty instanceProperty;
        instanceProperty.scaleX = object->scale().x();
        instanceProperty.scaleY = object->scale().y();
        instanceProperty.scaleZ = object->scale().z();
        instanceProperty.rotation = object->rotation();

        bool instanceExists = false;
        foreach (DagInstanceProperty existingInstance, instancesPerFile[fileIndex]) {
            if (existingInstance ==  instanceProperty) {
                instanceExists = true;
                break;
            }
        }

        if (!instanceExists)
            instancesPerFile[fileIndex].append(instanceProperty);
    }

    uint totalFileCount = 0;

    // Now we test if for any geometry object there are more than one scale versions
    for (int i = 0; i < clippingGeometryFiles.size(); ++i) {
        totalFileCount += instancesPerFile[i].size();
        if (instancesPerFile[i].size() > 1) {
            qDebug("Clipping geometry file %s is used in %d different versions:",
                   qPrintable(clippingGeometryFiles[i]), instancesPerFile[i].size());
        }
    }

    // File header
    clippingStream << totalFileCount << (uint)d->mZoneTemplate->clippingGeometry().size();

    for (int i = 0; i < clippingGeometryFiles.size(); ++i) {
        Troika::DagReader reader(d->mService->virtualFileSystem(), clippingGeometryFiles[i]);
        MeshModel *model = reader.get();

        Q_ASSERT(model->faceGroups().size() == 1);
        Q_ASSERT(model->vertices().size() > 0);

        /**
          Calculate the scaling matrix for each instance and transform the vertices ahead-of-time.
          */
        foreach (const DagInstanceProperty &instance, instancesPerFile[i]) {
            QVector<Vector4> instanceVertices(model->vertices().size());

            float scaleX = instance.scaleX;
            float scaleY = instance.scaleY;
            float scaleZ = instance.scaleZ;
            float rotation = instance.rotation;
            float rotCos = cos(rotation);
            float rotSin = sin(rotation);

            for (int j = 0; j < instanceVertices.size(); ++j) {
                const Troika::Vertex &vertex = model->vertices()[j];

                float x = vertex.positionX;
                float y = vertex.positionY;
                float z = vertex.positionZ;

                float nx, ny, nz;

                // Apply rotation, coordinate system already flipped.
                nx = rotSin * z + rotCos * x;
                ny = y;
                nz = rotCos * z - rotSin * x;

                float tx = 0.5f * scaleX * (nz - nx);
                float ty = 0.5f * scaleY * (nx + nz);

                nx = ty - tx;
                ny = scaleZ * ny;
                nz = tx + ty;

                instanceVertices[j] = Vector4(nx, ny, nz, 1);
            }

            // Calculate an axis-aligned bounding box.
            Box3d boundingBox(instanceVertices[0], instanceVertices[0]);

            Vector4 firstVertex = instanceVertices[0];
            firstVertex.setW(0);
            float originDistanceSquared = firstVertex.lengthSquared();

            for (int j = 1; j < instanceVertices.size(); ++j) {
                Vector4 vertex = instanceVertices[j];
                boundingBox.merge(vertex);
                vertex.setW(0);
                originDistanceSquared = qMax<double>(originDistanceSquared, vertex.lengthSquared());
            }

            float originDistance = sqrt(originDistanceSquared);

            clippingStream << boundingBox.minimum().x() << boundingBox.minimum().y() << boundingBox.minimum().z()
                    << boundingBox.minimum().w() << boundingBox.maximum().x() << boundingBox.maximum().y()
                    << boundingBox.maximum().z() << boundingBox.maximum().w() << originDistance << originDistanceSquared;

            clippingStream << (uint)instanceVertices.size() << (uint)model->faceGroups()[0]->faces().size() * 3;

            // We should apply the scaling here.

            foreach (const Vector4 &vertex, instanceVertices) {
                clippingStream << vertex.x() << vertex.y() << vertex.z() << vertex.w();
            }

            foreach (const Face &face, model->faceGroups()[0]->faces()) {
                clippingStream << face.vertices[0] << face.vertices[1] << face.vertices[2];
            }
        }
    }

    // Instances
    foreach (GeometryObject *object, d->mZoneTemplate->clippingGeometry()) {
        QString normalizedFilename = normalizePath(object->mesh());

        uint fileNameIndex = (uint)clippingGeometryFiles.indexOf(normalizedFilename);
        uint fileIndex = 0;

        // Find the filename/scale pair appropriate for this instance
        for (uint i = 0; i < fileNameIndex; ++i)
            fileIndex += instancesPerFile[i].size();

        bool found = false;

        DagInstanceProperty instanceProperty;
        instanceProperty.scaleX = object->scale().x();
        instanceProperty.scaleY = object->scale().y();
        instanceProperty.scaleZ = object->scale().z();
        instanceProperty.rotation = object->rotation();

        for (int i = 0; i < instancesPerFile[fileNameIndex].size(); ++i) {
            DagInstanceProperty existing = instancesPerFile[fileNameIndex][i];
            if (existing == instanceProperty) {
                found = true;
                break;
            }
            fileIndex++;
        }

        Q_ASSERT(found);

        clippingStream << object->position().x() << object->position().y() << object->position().z() << (float)1
                << fileIndex;
    }

    return clippingData;
}
