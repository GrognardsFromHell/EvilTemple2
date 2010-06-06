

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

    CritterProperties *critterProps = qobject_cast<CritterProperties*>(prototype->additionalProperties);

    if (critterProps) {
        writer.write("critterFlags", critterProps->flags);
        writer.write("strength", critterProps->strength);
        writer.write("dexterity", critterProps->dexterity);
        writer.write("constitution", critterProps->constitution);
        writer.write("intelligence", critterProps->intelligence);
        writer.write("wisdom", critterProps->wisdom);
        writer.write("charisma", critterProps->charisma);
        writer.write("race", critterProps->race);
        writer.write("gender", critterProps->gender);
        writer.write("age", critterProps->age);
        writer.write("alignment", critterProps->alignment);
        writer.write("deity", critterProps->deity);
        writer.write("alignmentChoice", critterProps->alignmentChoice);
        // Should probably be an array
        writer.write("domain1", critterProps->domain1);
        writer.write("domain2", critterProps->domain2);
        writer.write("portraitId", critterProps->portraitId);
        writer.write("unknownDescriptionId", critterProps->unknownDescription);
        writer.write("reach", critterProps->reach);
        writer.write("hairColor", critterProps->hairColor);
        writer.write("hairType", critterProps->hairType);

        // TODO:
        // QList<NaturalAttack> naturalAttacks; // at most 4 different ones
        // QList<ClassLevel> classLevels;
        // QList<SkillLevel> skills;
        // QStringList feats;
        // QString levelUpScheme; // Defines auto-leveling stuff (which feats to take, etc.)
        // String strategy; // Which AI is used in auto fighting situations
    }

    NonPlayerCharacterProperties *npcProps = qobject_cast<NonPlayerCharacterProperties*>
                                             (prototype->additionalProperties);

    if (npcProps) {
        writer.write("npcFlags", npcProps->flags);
        writer.write("aiData", npcProps->aiData);

        QVariantList factions;
        foreach (uint faction, npcProps->factions)
            factions.append(faction);
        writer.write("factions", factions);
        writer.write("challengeRating", npcProps->challengeRating);
        writer.write("reflexSave", npcProps->reflexSave);
        writer.write("fortitudeSave", npcProps->fortitudeSave);
        writer.write("willpowerSave", npcProps->willpowerSave);
        writer.write("acBonus", npcProps->acBonus);
        writer.write("hitDice", npcProps->hitDice);
        writer.write("npcType", npcProps->type);
        writer.write("npcSubtype", npcProps->subType);
        writer.write("lootSharing", npcProps->lootShareAmount);
        writer.write("addMeshId", npcProps->additionalMeshId);
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
