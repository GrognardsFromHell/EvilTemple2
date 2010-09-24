
#include <messagefile.h>
#include <prototypes.h>

#include "conversion/prototypeconverter.h"
#include "conversion/util.h"

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
    writer.write("dontDraw", prototype->dontDraw);
    writer.write("disabled", prototype->disabled);
    writer.write("interactive", prototype->interactive);
    writer.write("unlit", prototype->unlit);
    writer.write("scale", prototype->scale);
    writer.write("internalId", prototype->internalDescriptionId);
    writer.write("descriptionId", prototype->descriptionId);
    writer.write("size", prototype->objectSize.toLower());
    writer.write("hitPoints", prototype->hitPoints);
    writer.write("material", prototype->objectMaterial.toLower());
    writer.write("soundId", prototype->soundId);
    writer.write("categoryId", prototype->categoryId);
    writer.write("rotation", prototype->rotation);
    writer.write("walkSpeedFactor", prototype->walkSpeedFactor);
    writer.write("runSpeedFactor", prototype->runSpeedFactor);
    writer.write("model", mModelFiles[prototype->modelId]);
    writer.write("radius", prototype->radius);
    writer.write("height", prototype->renderHeight);

    ScriptProperties *scriptProps = qobject_cast<ScriptProperties*>(prototype->additionalProperties);

    if (scriptProps) {
        foreach (const ScriptAttachment &script, scriptProps->scripts) {
            QVariantMap scriptMap;
            scriptMap["script"] = script.scriptId;
            if (script.parameter.isDefined())
                scriptMap["param"] = script.parameter.value();

            writer.write(script.event, scriptMap);
        }
    }

    EntityProperties *entityProps = qobject_cast<EntityProperties*>(prototype->additionalProperties);

    if (entityProps) {
        QVariantList additionalProps;
        foreach (const AdditionalProperty &property, entityProps->properties) {
            QVariantMap additionalMap;
            additionalMap["type"] = property.type;
            if (!property.param1.isEmpty())
                additionalMap["param1"] = property.param1;
            if (!property.param2.isEmpty())
                additionalMap["param2"] = property.param2;
            additionalProps << additionalMap;
        }
        writer.write("properties", additionalProps);

        QVariantList spells;
        foreach (const KnownSpell &spell, entityProps->spells) {
            QVariantMap spellMap;
            spellMap["name"] = spell.name;
            spellMap["level"] = spell.level;
            spellMap["source"] = spell.source;
            spells << spellMap;
        }
        writer.write("spells", spells);
    }

    PortalProperties *portalProps = qobject_cast<PortalProperties*>(prototype->additionalProperties);

    if (portalProps) {
        writer.write("portalFlags", portalProps->flags);
        writer.write("lockDc", portalProps->lockDc);
        writer.write("keyId", portalProps->keyId);
        writer.write("notifyNpc", portalProps->notifyNpc);
    }

    ContainerProperties *containerProps = qobject_cast<ContainerProperties*>(prototype->additionalProperties);

    if (containerProps) {
        if (containerProps->locked)
            writer.write("locked", containerProps->locked);
        writer.write("lockDc", containerProps->lockDc);
        writer.write("keyId", containerProps->keyId);
        writer.write("inventorySource", containerProps->inventorySource);
    }

    SceneryProperties *sceneryProps = qobject_cast<SceneryProperties*>(prototype->additionalProperties);

    if (sceneryProps) {
        writer.write("sceneryFlags", sceneryProps->flags);
    }

    ItemProperties *itemProps = qobject_cast<ItemProperties*>(prototype->additionalProperties);

    if (itemProps) {
        writer.write("itemFlags", itemProps->flags);
        writer.write("weight", itemProps->weight);
        writer.write("worth", itemProps->worth);
        writer.write("inventoryId", itemProps->inventoryIcon);
        writer.write("inventoryGroundMesh", itemProps->inventoryGroundMesh);
        writer.write("unidentifiedDescriptionId", itemProps->unidentifiedDescriptionId);
        writer.write("chargesLeft", itemProps->chargesLeft);
        writer.write("equipmentFlags", itemProps->wearFlags);
        writer.write("equipmentId", itemProps->wearMeshId);
    }

    WeaponProperties *weaponProps = qobject_cast<WeaponProperties*>(prototype->additionalProperties);

    if (weaponProps) {
        writer.write("weaponFlags", weaponProps->flags);
        writer.write("range", weaponProps->range);
        writer.write("ammoType", weaponProps->ammoType);
        writer.write("missileAnimationId", weaponProps->missileAnimationId);
        writer.write("criticalMultiplier", weaponProps->criticalHitMultiplier);
        writer.write("damageType", weaponProps->damageType);
        writer.write("damageDice", weaponProps->damageDice);
        writer.write("weaponClass", weaponProps->weaponClass);
        writer.write("threatRange", weaponProps->threatRange);
    }

    AmmoProperties *ammoProps = qobject_cast<AmmoProperties*>(prototype->additionalProperties);

    if (ammoProps) {
        writer.write("quantity", ammoProps->quantity);
        writer.write("ammoType", ammoProps->type);
    }

    ArmorProperties *armorProps = qobject_cast<ArmorProperties*>(prototype->additionalProperties);

    if (armorProps) {
        writer.write("maxDexterityBonus", armorProps->maxDexterityBonus);
        writer.write("arcaneSpellFailure", armorProps->arcaneSpellFailure);
        writer.write("skillCheckPenalty", armorProps->skillCheckPenalty);
        writer.write("armorType", armorProps->armorType);
        writer.write("helmType", armorProps->helmetType);
    }

    MoneyProperties *moneyProps = qobject_cast<MoneyProperties*>(prototype->additionalProperties);

    if (moneyProps) {
        writer.write("quantity", moneyProps->quantity);
        writer.write("moneyType", moneyProps->type);
    }

    KeyProperties *keyProps = qobject_cast<KeyProperties*>(prototype->additionalProperties);

    if (keyProps) {
        writer.write("keyId", keyProps->keyId);
    }

    WrittenProperties *writtenProps = qobject_cast<WrittenProperties*>(prototype->additionalProperties);

    if (writtenProps) {
        writer.write("subtype", writtenProps->subtype);
        writer.write("startLine", writtenProps->startLine);
    }

    BagProperties *bagProperties = qobject_cast<BagProperties*>(prototype->additionalProperties);

    if (bagProperties) {
        writer.write("bagFlags", bagProperties->flags);
        writer.write("bagSize", bagProperties->size);
    }



    CritterProperties *critterProps = qobject_cast<CritterProperties*>(prototype->additionalProperties);

    if (critterProps) {
        if (critterProps->flags.removeAll("IsConcealed") > 0) {
            writer.write("concealed", true);
        }
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
        writer.write("domains", critterProps->domains);
        writer.write("portraitId", critterProps->portraitId);
        writer.write("unknownDescriptionId", critterProps->unknownDescription);
        writer.write("reach", critterProps->reach);
        writer.write("hairColor", critterProps->hairColor);
        writer.write("hairType", critterProps->hairType);

        QVariantList naturalAttacks;

        foreach (const NaturalAttack &attack, critterProps->naturalAttacks) {
            QVariantMap attackMap;
            attackMap["count"] = attack.numberOfAttacks;
            attackMap["type"] = attack.type;
            attackMap["attackBonus"] = attack.attackBonus;
            attackMap["damageDice"] = attack.damageDice;
            naturalAttacks << attackMap;
        }

        writer.write("naturalAttacks", naturalAttacks);

        QVariantList classLevels;

        foreach (const ClassLevel &classLevel, critterProps->classLevels) {
            QVariantMap classLevelMap;
            classLevelMap["classId"] = classLevel.name.toLower();
            classLevelMap["count"] = classLevel.count;
            classLevels << classLevelMap;
        }

        writer.write("classLevels", classLevels);

        QVariantMap skillRanks;

        foreach (const SkillLevel &skillLevel, critterProps->skills) {
            if (skillRanks.contains(skillLevel.name))
                skillRanks[skillLevel.name] = skillRanks[skillLevel.name].toInt() + skillLevel.count;
            else
                skillRanks[skillLevel.name] = skillLevel.count;
        }

        if (!skillRanks.isEmpty())
            writer.write("skills", skillRanks);

        writer.write("feats", critterProps->feats);
        writer.write("levelUpScheme", critterProps->levelUpScheme);
        writer.write("strategy", critterProps->strategy);
    }

    NonPlayerCharacterProperties *npcProps = qobject_cast<NonPlayerCharacterProperties*>
                                             (prototype->additionalProperties);

    if (npcProps) {
        if (npcProps->flags.removeAll("KillOnSight"))
            writer.write("killsOnSight", true);
        if (npcProps->flags.removeAll("Wanders"))
            writer.write("wanders", true);
        if (npcProps->flags.removeAll("WandersInDark"))
            writer.write("wandersInDark", true);
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
        writer.write("npcSubtypes", npcProps->subTypes);
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
        QString filename = normalizePath(mModelFiles[key]);

        mModelFiles[key] = "meshes/" + filename + ".model";
    }
}
