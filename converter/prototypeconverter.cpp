

#include "prototypeconverter.h"
#include "prototypes.h"
#include "messagefile.h"
#include "util.h"

using namespace Troika;

static const QString typeTagNames[ObjectTypeCount] = {
    "Portal",
    "Container",
    "Scenery",
    "Projectile",
    "Weapon",
    "Ammo",
    "Armor",
    "Money",
    "Food",
    "Scroll",
    "Key",
    "Written",
    "Generic",
    "PlayerCharacter",
    "NonPlayerCharacter",
    "Trap",
    "Bag"
};

QVariantMap PrototypeConverter::convertPrototype(Prototype *prototype)
{
    QVariantMap result;

    result["type"] = typeTagNames[prototype->type];
    result["id"] = prototype->id;

    JsonPropertyWriter writer(result);

    if (prototype->internalDescriptionId.isDefined()) {
        writer.write("internalDescription", mInternalDescriptions[prototype->internalDescriptionId.value()]);
    }

    writer.write("flags", prototype->objectFlags);
    writer.write("scale", prototype->scale);
    // writer.write("internalDescriptionId", prototype->internalDescriptionId);
    writer.write("descriptionId", prototype->descriptionId);
    writer.write("size", prototype->objectSize);
    writer.write("hitPoints", prototype->hitPoints);
    writer.write("material", prototype->objectMaterial);
    writer.write("soundId", prototype->soundId);
    writer.write("categoryId", prototype->categoryId);
    writer.write("rotation", prototype->rotation);
    writer.write("walkSpeedFactor", prototype->walkSpeedFactor);
    writer.write("runSpeedFactor", prototype->runSpeedFactor);
    writer.write("model", mModelFiles[prototype->modelId]);
    writer.write("radius", prototype->radius);
    writer.write("height", prototype->renderHeight);

    ItemProperties *itemProps = qobject_cast<ItemProperties*>(prototype->additionalProperties);

    if (itemProps) {
        writer.write("itemFlags", itemProps->flags);
        writer.write("weight", itemProps->weight);
        writer.write("worth", itemProps->worth);
        writer.write("inventoryId", itemProps->inventoryIcon);
        writer.write("inventoryGroundMesh", itemProps->inventoryGroundMesh);
        writer.write("unidentifiedDescriptionId", itemProps->unidentifiedDescriptionId);
        writer.write("chargesLeft", itemProps->chargesLeft);
        writer.write("wearFlags", itemProps->wearFlags);
        writer.write("wearMeshId", itemProps->wearMeshId);
    }

    return result;
}

QVariantMap PrototypeConverter::convertPrototypes(Troika::Prototypes *prototypes)
{
    QVariantMap result;

    foreach (Prototype *prototype, prototypes->prototypes()) {
        result[QString("%1").arg(prototype->id)] = convertPrototype(prototype);
    }

    return result;
}

PrototypeConverter::PrototypeConverter(VirtualFileSystem *vfs) : mVfs(vfs)
{
    mInternalDescriptions = MessageFile::parse(mVfs->openFile("oemes/oname.mes"));
    mDescriptions = MessageFile::parse(mVfs->openFile("mes/description.mes"));
    mModelFiles = MessageFile::parse(mVfs->openFile("art/meshes/meshes.mes"));

    foreach (uint key, mModelFiles.keys()) {
        mModelFiles[key] = "meshes/" + mModelFiles[key] + ".model";
    }
}
