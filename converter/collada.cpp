
#include <QtXml>
#include <QString>
#include <QFile>

#include "virtualfilesystem.h"
#include "messagefile.h"
#include "troikaarchive.h"
#include "prototypes.h"
#include "troika_materials.h"
#include "zonetemplate.h"
#include "zonetemplatereader.h"
#include "zonetemplates.h"
#include "zonebackgroundmap.h"
#include "skmreader.h"
#include "model.h"
#include "troika_material.h"

#include "util.h"

QDomElement createTechniqueCommon(QDomDocument &document, const QString &sourceId, int vertices, int stride)
{
    QDomElement techniqueCommon = document.createElement("technique_common");

    QDomElement accessor = document.createElement("accessor");
    techniqueCommon.appendChild(accessor);
    accessor.setAttribute("source", sourceId);
    accessor.setAttribute("count", vertices);
    accessor.setAttribute("stride", stride);

    QDomElement param = document.createElement("param");
    param.setAttribute("name", (stride == 3) ? "X" : "U");
    param.setAttribute("type", "float");
    accessor.appendChild(param);

    param = document.createElement("param");
    param.setAttribute("name", (stride == 3) ? "Y" : "V");
    param.setAttribute("type", "float");
    accessor.appendChild(param);

    if (stride == 3) {
        param = document.createElement("param");
        param.setAttribute("name", "Z");
        param.setAttribute("type", "float");
        accessor.appendChild(param);
    }

    return techniqueCommon;
}

void createSources(QDomDocument &document, QDomElement &mesh, Troika::MeshModel *model)
{
    // Create a vertex source
    QDomElement source = document.createElement("source");
    source.setAttribute("id", "mesh-positions");
    mesh.appendChild(source);

    QDomElement floatArray = document.createElement("float_array");
    source.appendChild(floatArray);
    floatArray.setAttribute("count", model->vertices().size() * 3);
    floatArray.setAttribute("id", "mesh-positions-array");

    QString floatArrayText;
    foreach (const Troika::Vertex &vertex, model->vertices()) {
        floatArrayText.append(QString("%1 %2 %3\n").arg(vertex.positionX).arg(vertex.positionY).arg(vertex.positionZ));
    }

    floatArray.appendChild(document.createTextNode(floatArrayText));

    source.appendChild(createTechniqueCommon(document, "#mesh-positions-array", model->vertices().size(), 3));

    // Create a normal source
    source = document.createElement("source");
    source.setAttribute("id", "mesh-normals");
    mesh.appendChild(source);

    floatArray = document.createElement("float_array");
    source.appendChild(floatArray);
    floatArray.setAttribute("count", model->vertices().size() * 3);
    floatArray.setAttribute("id", "mesh-normals-array");

    floatArrayText.clear();
    foreach (const Troika::Vertex &vertex, model->vertices()) {
        floatArrayText.append(QString("%1 %2 %3\n").arg(vertex.normalX).arg(vertex.normalY).arg(vertex.normalZ));
    }

    floatArray.appendChild(document.createTextNode(floatArrayText));

    source.appendChild(createTechniqueCommon(document, "#mesh-normals-array", model->vertices().size(), 3));

    // Create a UV source
    source = document.createElement("source");
    source.setAttribute("id", "mesh-uv");
    mesh.appendChild(source);

    floatArray = document.createElement("float_array");
    source.appendChild(floatArray);
    floatArray.setAttribute("count", model->vertices().size() * 2);
    floatArray.setAttribute("id", "mesh-uv-array");

    floatArrayText.clear();
    foreach (const Troika::Vertex &vertex, model->vertices()) {
        floatArrayText.append(QString("%1 %2\n").arg(vertex.texCoordX).arg(vertex.texCoordY));
    }

    floatArray.appendChild(document.createTextNode(floatArrayText));

    source.appendChild(createTechniqueCommon(document, "#mesh-uv-array", model->vertices().size(), 2));
}

void createVertices(QDomDocument &document, QDomElement &mesh)
{
    QDomElement vertices = document.createElement("vertices");
    vertices.setAttribute("id", "mesh-vertices");

    QDomElement input = document.createElement("input");
    vertices.appendChild(input);
    input.setAttribute("semantic", "POSITION");
    input.setAttribute("source", "#mesh-positions");

    mesh.appendChild(vertices);
}

void createMaterials(Troika::VirtualFileSystem *vfs, QDomDocument &document, QDomElement &root, Troika::MeshModel *model)
{

    QDomElement imageLibrary = document.createElement("library_images");
    root.appendChild(imageLibrary);

    QDomElement materialLibrary = document.createElement("library_materials");
    root.appendChild(materialLibrary);

    QDomElement effectsLibrary = document.createElement("library_effects");
    root.appendChild(effectsLibrary);

    QHash<QString, QSharedPointer<Troika::Material> > materials; // Make *sure* that every material is only added once

    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, model->faceGroups()) {
        QString materialName(faceGroup->material()->name());
        mangleMaterialName(materialName);
        if (materials.contains(materialName))
            continue;

        materials[materialName] = faceGroup->material();
    }

    QHash<QString, QImage> images;

    foreach (const QString &materialName, materials.keys()) {
        QSharedPointer<Troika::Material> material = materials[materialName];

        // A material consist of a material and an effect instantiation. for Toee they are the same
        QDomElement materialNode = document.createElement("material");
        materialLibrary.appendChild(materialNode);
        materialNode.setAttribute("id", materialName + "-material");
        QDomElement instanceEffect = document.createElement("instance_effect");
        materialNode.appendChild(instanceEffect);
        instanceEffect.setAttribute("url", "#" + materialName + "-effect");

        QDomElement effectNode = document.createElement("effect");
        effectsLibrary.appendChild(effectNode);
        effectNode.setAttribute("id", materialName + "-effect");

        QDomElement profileCOMMON = document.createElement("profile_COMMON");
        effectNode.appendChild(profileCOMMON);

        for (int i = 0; i < Troika::LegacyTextureStages; ++i) {
            // Ensure that the image for this stage is properly dumped later on
            const Troika::TextureStageInfo *stage = material->getTextureStage(i);
            if (stage->image.isNull())
                continue;

            if (!images.contains(stage->filename))
                images[stage->filename] = stage->image;

            QString mangledName = stage->filename;
            mangleImageName(mangledName);

            // Create the surface itself
            QDomElement newparam = document.createElement("newparam");
            newparam.setAttribute("sid", QString("stage%1-image").arg(i));
            profileCOMMON.appendChild(newparam);
            QDomElement surface = document.createElement("surface");
            surface.setAttribute("type", "2D");
            newparam.appendChild(surface);
            QDomElement initFrom = document.createElement("init_from");
            surface.appendChild(initFrom);
            initFrom.appendChild(document.createTextNode(mangledName));
            QDomElement format = document.createElement("format");
            surface.appendChild(format);
            format.appendChild(document.createTextNode("A8R8G8B8"));

            // And a sampler for the image
            newparam = document.createElement("newparam");
            newparam.setAttribute("sid", QString("stage%1-sampler").arg(i));
            profileCOMMON.appendChild(newparam);
            QDomElement sampler2D = document.createElement("sampler2D");
            newparam.appendChild(sampler2D);
            QDomElement source = document.createElement("source");
            sampler2D.appendChild(source);
            source.appendChild(document.createTextNode(QString("stage%1-image").arg(i)));
            // TODO: Minfilter / Magfilter
        }

        // Common technique
        QDomElement technique = document.createElement("technique");
        profileCOMMON.appendChild(technique);
        technique.setAttribute("sid", "common");

        // Constant shading
        QDomElement constant = document.createElement("constant");
        technique.appendChild(constant);

        // Diffuse color comes from texture
        QDomElement diffuse = document.createElement("diffuse");
        constant.appendChild(diffuse);

        QDomElement texture = document.createElement("texture");
        diffuse.appendChild(texture);
        texture.setAttribute("texture", "stage0-sampler"); // How to do multi-texturing?
        texture.setAttribute("texcoord", "TEX0");
    }

    // Dump images
    foreach (QString imageFilename, images.keys()) {
        QString mangledName = imageFilename;
        mangleImageName(mangledName);

        QDomElement imageNode = document.createElement("image");
        imageLibrary.appendChild(imageNode);
        imageNode.setAttribute("id", mangledName);
        imageNode.setAttribute("format", "TGA");

        QDomElement dataElement = document.createElement("init_from");
        imageNode.appendChild(dataElement);
        dataElement.appendChild(document.createTextNode("./" + mangledName + ".tga"));

        qDebug("Trying to dump image %s.", qPrintable(imageFilename));
        QByteArray imageData = vfs->openFile(imageFilename);

        QFile file(mangledName + ".tga");
        if (file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
            file.write(imageData);
            file.close();
        }
    }
}

void createTriangles(QDomDocument &document, QDomElement &mesh, Troika::MeshModel *model)
{
    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, model->faceGroups()) {
        QDomElement triangles = document.createElement("triangles");
        triangles.setAttribute("count", faceGroup->faces().size());
        QString materialName(faceGroup->material()->name());
        triangles.setAttribute("material", mangleMaterialName(materialName));

        QDomElement input = document.createElement("input");
        triangles.appendChild(input);
        input.setAttribute("semantic", "VERTEX");
        input.setAttribute("source", "#mesh-vertices");
        input.setAttribute("offset", 0);

        input = document.createElement("input");
        triangles.appendChild(input);
        input.setAttribute("semantic", "NORMAL");
        input.setAttribute("source", "#mesh-normals");
        input.setAttribute("offset", 1);

        input = document.createElement("input");
        triangles.appendChild(input);
        input.setAttribute("semantic", "TEXCOORD");
        input.setAttribute("source", "#mesh-uv");
        input.setAttribute("offset", 2);
        input.setAttribute("set", 0);

        QString indices;

        // Append indices
        for (int i = 0; i < faceGroup->faces().size(); ++i) {
            const Troika::Face &face = faceGroup->faces()[i];

            if (i != 0)
                indices.append('\n');
            indices.append(QString("%1 %1 %1 ").arg(face.vertices[0]));
            indices.append(QString("%1 %1 %1 ").arg(face.vertices[1]));
            indices.append(QString("%1 %1 %1").arg(face.vertices[2]));
        }

        QDomElement p = document.createElement("p");
        p.appendChild(document.createTextNode(indices));
        triangles.appendChild(p);

        mesh.appendChild(triangles);
    }
}

void createScene(QDomDocument &document, QDomElement &root, Troika::MeshModel *model)
{
    QDomElement visualScenes = document.createElement("library_visual_scenes");
    root.appendChild(visualScenes);

    QDomElement visualScene = document.createElement("visual_scene");
    visualScenes.appendChild(visualScene);
    visualScene.setAttribute("name", "untitled");
    visualScene.setAttribute("id", "scene");

    QDomElement node = document.createElement("node");
    node.setAttribute("id", "mesh-node");
    node.setAttribute("name", "mesh-node");
    visualScene.appendChild(node);

    QDomElement instanceGeometry = document.createElement("instance_geometry");
    node.appendChild(instanceGeometry);
    instanceGeometry.setAttribute("url", "#mesh");

    QDomElement bindMaterial = document.createElement("bind_material");
    instanceGeometry.appendChild(bindMaterial);
    QDomElement techniqueCommon = document.createElement("technique_common");
    bindMaterial.appendChild(techniqueCommon);

    QHash<QString, QString> materials; // Map material symbols to material ids

    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, model->faceGroups()) {
        QString materialName(faceGroup->material()->name());
        mangleMaterialName(materialName);
        materials[materialName] = "#" + materialName + "-material";
    }

    foreach (QString materialSymbol, materials.keys()) {
        QDomElement instanceMaterial = document.createElement("instance_material");
        instanceMaterial.setAttribute("symbol", materialSymbol);
        instanceMaterial.setAttribute("target", materials[materialSymbol]);
        techniqueCommon.appendChild(instanceMaterial);

        // <bind_vertex_input semantic="TEX0" input_semantic="TEXCOORD" input_set="0"/>
        QDomElement bindVertexInput = document.createElement("bind_vertex_input");
        instanceMaterial.appendChild(bindVertexInput);
        bindVertexInput.setAttribute("semantic", "TEX0"); // TODO: Multi texturing
        bindVertexInput.setAttribute("input_semantic", "TEXCOORD");
        bindVertexInput.setAttribute("input_set", 0);
    }

    QDomElement scene = document.createElement("scene");
    root.appendChild(scene);

    QDomElement instanceVisualScene = document.createElement("instance_visual_scene");
    scene.appendChild(instanceVisualScene);
    instanceVisualScene.setAttribute("url", "#scene");
}

bool convertModels(Troika::VirtualFileSystem *vfs, Troika::Materials *materials)
{
    Troika::SkmReader reader(vfs, materials, "art/meshes/Monsters/Carrion_Crawler/CarrionCrawler.skm");

    QScopedPointer<Troika::MeshModel> model(reader.get());

    QDomDocument document;

    QDomElement root = document.createElementNS("http://www.collada.org/2005/11/COLLADASchema", "COLLADA");
    document.appendChild(root);
    root.setAttribute("version", "1.4.1");

    // Create default asset nodes
    QDomElement asset = document.createElement("asset");
    QDomElement created = document.createElement("created");
    created.appendChild(document.createTextNode(QDateTime::currentDateTime().toString(Qt::ISODate)));
    asset.appendChild(created);
    QDomElement modified = document.createElement("modified");
    asset.appendChild(modified);
    modified.appendChild(document.createTextNode(QDateTime::currentDateTime().toString(Qt::ISODate)));

    root.appendChild(asset);

    createMaterials(vfs, document, root, model.data());

    // Create geometry library
    QDomElement geometryLibrary = document.createElement("library_geometries");
    root.appendChild(geometryLibrary);

    // Create geometry object
    QDomElement geometryObject = document.createElement("geometry");
    geometryObject.setAttribute("id", "mesh"); // Set object id to mesh
    geometryLibrary.appendChild(geometryObject);

    // Create a single mesh for the geometry object
    QDomElement mesh = document.createElement("mesh");
    geometryObject.appendChild(mesh);

    createSources(document, mesh, model.data());

    createVertices(document, mesh);

    createTriangles(document, mesh, model.data());

    createScene(document, root, model.data());

    QFile modelFile("model.dae");

    if (modelFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)) {
        modelFile.write(document.toString().toUtf8());
        modelFile.close();
    }

    return true;
}

