
#include "troika_material.h"
#include "virtualfilesystem.h"
#include "constants.h"
#include "util.h"

namespace Troika {

    Material::Material(Type type, const QString &name) : _name(name),
        mType(type),
        disableFaceCulling(false),
        disableLighting(false),
        disableDepthTest(false),
        linearFiltering(false),
        disableDepthWrite(false),
        mBlendType(Alpha),
        mSpecularPower(50),
        mRecalculateNormals(false),
        color(255, 255, 255, 255) {
    }

    Material::~Material() {
    }

    Material *Material::create(VirtualFileSystem *vfs, const QString &filename) {
        QByteArray rawContent = vfs->openFile(filename);
        QString content = QString::fromLocal8Bit(rawContent.data(), rawContent.size());

        Material *material = new Material(Material::UserDefined, filename);

        QStringList lines = content.split('\n', QString::SkipEmptyParts);

        foreach (QString line, lines) {
            QStringList args;
            bool inQuotes = false; // No escape characters allowed
            QString buffer;
            foreach (QChar ch, line) {
                if (ch.isSpace() && !inQuotes) {
                    if (!buffer.isEmpty())
                        args.append(buffer);
                    buffer.clear();
                } else if (ch == '"') {
                    if (inQuotes) {
                        args.append(buffer);
                        buffer.clear();
                    }
                    inQuotes = !inQuotes;
                } else {
                    buffer.append(ch);
                }
            }
            if (!buffer.isEmpty()) {
                args.append(buffer);
            }

            if (args.isEmpty())
                continue;

            QString command = args.takeFirst();

            if (!material->processCommand(vfs, command, args))
                qWarning("Material %s has incorrect command: %s", qPrintable(filename), qPrintable(line));
        }

        return material;
    }


    bool Material::processCommand(VirtualFileSystem *vfs, const QString &command, const QStringList &args)
    {
        if (!command.compare("Texture", Qt::CaseInsensitive)) {

            int unit;
            QString texture;

            if (args.size() == 1) {
                unit = 0;
                texture = args[0];

            } else if (args.size() == 2) {
                bool ok;
                unit = args[0].toInt(&ok);

                if (!ok || unit < 0 || unit >= LegacyTextureStages)
                    return false; // Wrong texture unit format

                texture = args[1];
            } else {
                return false; // More arguments than needed
            }

            if (vfs->exists(texture)) {
                textureStages[unit].setFilename(texture);
            } else {
                qWarning("Unknown texture %s referenced in %s.", qPrintable(texture), qPrintable(name()));
            }

            return true;

        } else if (!command.compare("Glossmap", Qt::CaseInsensitive)) {

            if (args.size() != 1) {
                qWarning("Glossmap has invalid args: %s", qPrintable(args.join(" ")));
                return false;
            }

            mGlossmap = args[0];

            return true;

        } else if (!command.compare("ColorFillOnly", Qt::CaseInsensitive)) {
            disableDepthWrite = true;

        } else if (!command.compare("BlendType", Qt::CaseInsensitive)) {

            if (args.size() != 2) {
                qWarning("BlendType has invalid args: %s", qPrintable(args.join(" ")));
                return false;
            }

            bool ok;
            int unit = args[0].toInt(&ok);

            // Check the texture unit id for sanity
            if (!ok || unit < 0 || unit >= LegacyTextureStages)
                return false;

            TextureStageInfo &stage = textureStages[unit];
            QString type = args[1].toLower();

            if (type == "modulate") {
                stage.setBlendType(TextureStageInfo::Modulate);
            } else if (type == "add") {
                stage.setBlendType(TextureStageInfo::Add);
            } else if (type == "texturealpha") {
                stage.setBlendType(TextureStageInfo::TextureAlpha);
            } else if (type == "currentalpha") {
                stage.setBlendType(TextureStageInfo::CurrentAlpha);
            } else if (type == "currentalphaadd") {
                stage.setBlendType(TextureStageInfo::CurrentAlphaAdd);
            } else {
                qWarning("Unknown blend type for texture stage %d: %s", unit, qPrintable(type));
                return false;
            }

            return true;

        } else if (!command.compare("Speed", Qt::CaseInsensitive)) {
            // Sets both U&V speed for all stages
            if (args.size() == 1) {
                bool ok;
                float speed = args[0].toFloat(&ok);

                if (!ok) {
                    qWarning("Invalid UV speed.");
                    return false;
                }

                for (int i = 0; i < LegacyTextureStages; ++i) {
                    textureStages[i].setSpeedU(speed);
                    textureStages[i].setSpeedV(speed);
                }

                return true;
            } else {
                qWarning("Invalid arguments for texture command %s", qPrintable(command));
                return false;
            }

        } else if (!command.compare("Specularpower", Qt::CaseInsensitive)) {
            if (args.size() == 1)
            {
                bool ok;
                mSpecularPower = args[0].toFloat(&ok);

                if (!ok) {
                    qWarning("Invalid specular power.");
                    return false;
                }

                return true;
            }
            else
            {
                qWarning("Invalid arguments for texture command %s", qPrintable(command));
                return false;
            }

        } else if (!command.compare("SpeedU", Qt::CaseInsensitive)
            || !command.compare("SpeedV", Qt::CaseInsensitive)) {

            // Sets a transform speed for one stage
            if (args.size() == 2)
            {
                bool ok;
                float speed = args[1].toFloat(&ok);

                if (!ok) {
                    qWarning("Invalid UV speed.");
                    return false;
                }

                int stage = args[0].toInt(&ok);

                if (!ok || stage < 0 || stage >= LegacyTextureStages)
                {
                    qWarning("Invalid texture stage %s.", qPrintable(args[0]));
                    return false;
                }

                if (!command.compare("SpeedU", Qt::CaseInsensitive))
                    textureStages[stage].setSpeedU(speed);
                else
                    textureStages[stage].setSpeedV(speed);

                return true;
            }
            else
            {
                qWarning("Invalid arguments for texture command %s", qPrintable(command));
                return false;
            }

        } else if (!command.compare("UVType", Qt::CaseInsensitive)) {

            // Sets the transform type for one stage
            if (args.size() == 2)
            {
                bool ok;
                int stage = args[0].toInt(&ok);

                if (!ok || stage < 0 || stage >= LegacyTextureStages)
                {
                    qWarning("Invalid texture stage %s.", qPrintable(args[0]));
                    return false;
                }

                QString type = args[1].toLower();

                if (type == "mesh")
                    textureStages[stage].setUvType(TextureStageInfo::Mesh);
                else if (type == "drift")
                    textureStages[stage].setUvType(TextureStageInfo::Drift);
                else if (type == "swirl")
                    textureStages[stage].setUvType(TextureStageInfo::Swirl);
                else if (type == "wavey")
                    textureStages[stage].setUvType(TextureStageInfo::Wavey);
                else if (type == "environment")
                    textureStages[stage].setUvType(TextureStageInfo::Environment);
                else
                    return false;

                return true;
            }
            else
            {
                qWarning("Invalid arguments for texture command %s", qPrintable(command));
                return false;
            }

        } else if (!command.compare("MaterialBlendType", Qt::CaseInsensitive)) {
            if (args.size() == 1) {
                QString type = args[0].toLower();

                if (type == "none") {
                    mBlendType = None;
                } else if (type == "alpha") {
                    mBlendType = Alpha;
                } else if (type == "add") {
                    mBlendType = Add;
                } else if (type == "alphaadd") {
                    mBlendType = AlphaAdd;
                } else {
                    qWarning("Unknown MaterialBlendType type: %s", qPrintable(type));
                    return false;
                }

                return true;
            } else {
                qWarning("Material blend type has invalid arguments: %s", qPrintable(args.join(" ")));
                return false;
            }
        } else if (!command.compare("Double", Qt::CaseInsensitive)) {
            disableFaceCulling = true;
            return true;
        } else if (!command.compare("notlit", Qt::CaseInsensitive)
            || !command.compare("notlite", Qt::CaseInsensitive)) {
            // We also compare against notlite, since it's a very common mistake in the vanilla MDFs
            disableLighting = true;
            return true;
        } else if (!command.compare("DisableZ", Qt::CaseInsensitive)) {
            disableDepthTest = true;
            return true;
        } else if (!command.compare("General", Qt::CaseInsensitive)
            || !command.compare("HighQuality", Qt::CaseInsensitive)) {
            // This was previously used by the material system to define materials
            // for different quality settings. The current hardware performance makes this
            // unneccessary. A better way to deal with this could be to ignore the "general"
            // definition completely.
            return true;
        } else if (!command.compare("LinearFiltering", Qt::CaseInsensitive)) {
            linearFiltering = true;
            return true;
        } else if (!command.compare("Textured", Qt::CaseInsensitive)) {
            // Unused
            return true;
        } else if (!command.compare("RecalculateNormals", Qt::CaseInsensitive)) {
            mRecalculateNormals = true;
            return true;
        } else if (!command.compare("Color", Qt::CaseInsensitive)) {
            if (args.count() != 4) {
                qWarning("Color needs 4 arguments: %s", qPrintable(args.join(" ")));
                return false;
            }

            int rgba[4];
            for (int i = 0; i < 4; ++i) {
                bool ok;
                rgba[i] = args[i].toUInt(&ok);
                if (!ok) {
                    qWarning("Color argument %d is invalid: %s", i, qPrintable(args[i]));
                    return false;
                }
                if (rgba[i] > 255) {
                    qWarning("Color argument %d is out of range (0-255): %d", i, rgba[i]);
                    return false;
                }
            }

            color.setRed(rgba[0]);
            color.setGreen(rgba[1]);
            color.setBlue(rgba[2]);
            color.setAlpha(rgba[3]);

            return true;
        } else {
            return false; // Unknown command
        }
    }
}
