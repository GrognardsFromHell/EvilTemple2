

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

void PrototypeConverter::convertPrototype(Prototype *prototype, QXmlStreamWriter &xml)
{
    // Add a comment with the description
    xml.writeComment(mDescriptions[prototype->descriptionId]);

    xml.writeStartElement(typeTagNames[prototype->type]);

    xml.writeAttribute("id", QString("%1").arg(prototype->id));

    PropertyWriter writer(xml);

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
    writer.write("modelId", prototype->modelId);
    writer.write("radius", prototype->radius);
    writer.write("height", prototype->renderHeight);

    xml.writeEndElement();
}

void PrototypeConverter::convertPrototypes(Troika::Prototypes *prototypes, QXmlStreamWriter &xml)
{
    xml.writeStartDocument("1.0");

    xml.writeStartElement("prototypes");

    foreach (Prototype *prototype, prototypes->prototypes()) {
        convertPrototype(prototype, xml);

        // Nicer to read
        xml.writeCharacters("\n\n    ");
    }

    xml.writeEndElement();

    xml.writeEndDocument();
}

PrototypeConverter::PrototypeConverter(VirtualFileSystem *vfs) : mVfs(vfs)
{
    mInternalDescriptions = MessageFile::parse(mVfs->openFile("oemes/oname.mes"));
    mDescriptions = MessageFile::parse(mVfs->openFile("mes/description.mes"));
}
