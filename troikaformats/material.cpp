#include "material.h"
#include "virtualfilesystem.h"
#include "glext.h"
#include "constants.h"
#include "util.h"

namespace Troika {

    Material::Material(Type type, const QString &name) : _name(name),
        mType(type),
        disableFaceCulling(false),
        disableLighting(false),
        disableDepthTest(false),
        linearFiltering(false),
        disableAlphaBlending(false),
        sourceBlendFactor(GL_SRC_ALPHA),
        destBlendFactor(GL_ONE_MINUS_SRC_ALPHA),
        disableDepthWrite(false),
        color(255, 255, 255, 255) {

    }

    Material::~Material() {
    }

    bool Material::hitTest(float u, float v)
    {
        Q_UNUSED(u);
        Q_UNUSED(v);

        return true;
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

            QByteArray data = vfs->openFile(texture);
            if (!data.isNull()) {
                textureStages[unit].filename = texture;
                //textureStages[unit].image.loadFromData(data, "tga");
            } else {
                qWarning("Unknown texture %s referenced in %s.", qPrintable(texture), qPrintable(name()));
            }

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
            QString type = args[1];

            if (!type.compare("modulate", Qt::CaseInsensitive))
            {
                // Reset to default parameters
                stage.colorOp = GL_MODULATE;
                stage.alphaOp = GL_MODULATE;
                stage.colorArg0 = GL_TEXTURE;
                stage.colorOperand0 = GL_SRC_COLOR;
                stage.alphaArg0 = GL_TEXTURE;
                stage.alphaOperand0 = GL_SRC_ALPHA;
                stage.colorArg1 = GL_PREVIOUS;
                stage.colorOperand1 = GL_SRC_COLOR;
                stage.alphaArg1 = GL_PREVIOUS;
                stage.alphaOperand1 = GL_SRC_ALPHA;
                stage.blendType = TextureStageInfo::Modulate;
            }
            else if (!type.compare("add", Qt::CaseInsensitive))
            {
                stage.blendType = TextureStageInfo::Add;
                qWarning("Unsupported type: %s", qPrintable(type));
            }
            else if (!type.compare("texturealpha", Qt::CaseInsensitive))
            {
                qWarning("Unsupported type: %s", qPrintable(type));
                stage.blendType = TextureStageInfo::TextureAlpha;
            }
            else if (!type.compare("currentalpha", Qt::CaseInsensitive))
            {
                stage.colorOp = GL_INTERPOLATE;

                stage.alphaOp = GL_REPLACE;
                stage.alphaArg0 = GL_PRIMARY_COLOR;
                stage.blendType = TextureStageInfo::CurrentAlpha;
            }
            else if (!type.compare("currentalphaadd", Qt::CaseInsensitive))
            {
                /*
                TODO: This doesn't match ToEE's behaviour. I suppose this behaviour
                    is only possible with a pixelshader in opengl.

                 The following are the original commands used in ToEE:
                 material.Stages[unit].ColorOp = TextureOperation.ModulateAlphaAddColor;
                 material.Stages[unit].AlphaOp = TextureOperation.SelectArg2;
                 material.Stages[unit].ColorArg1 = TextureArgument.Current;
                 material.Stages[unit].ColorArg2 = TextureArgument.Texture;
                 material.Stages[unit].AlphaArg1 = TextureArgument.Current;
                 material.Stages[unit].AlphaArg2 = TextureArgument.Diffuse;*/
                stage.colorOp = GL_INTERPOLATE;

                stage.alphaOp = GL_REPLACE;
                stage.alphaArg0 = GL_PRIMARY_COLOR;

                stage.blendType = TextureStageInfo::CurrentAlphaAdd;
            }
            else
            {
                qWarning("Unknown blend type for texture stage %d: %s", unit, qPrintable(type));
                return false;
            }

            return true;

        } else if (!command.compare("Speed", Qt::CaseInsensitive)) {
            // Sets both U&V speed for all stages
            if (args.size() == 1)
            {
                bool ok;
                float speed = args[0].toFloat(&ok);

                if (!ok) {
                    qWarning("Invalid UV speed.");
                    return false;
                }

                for (int i = 0; i < LegacyTextureStages; ++i)
                {
                    textureStages[i].speedu = textureStages[i].speedv = speed;
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
                    textureStages[stage].speedu = speed;
                else
                    textureStages[stage].speedv = speed;

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

                QString type = args[1];

                if (!type.compare("Drift", Qt::CaseInsensitive))
                    textureStages[stage].transformType = TextureStageInfo::Drift;
                else if (!type.compare("Swirl", Qt::CaseInsensitive))
                    textureStages[stage].transformType = TextureStageInfo::Swirl;
                else if (!type.compare("Wavey", Qt::CaseInsensitive))
                    textureStages[stage].transformType = TextureStageInfo::Wavey;
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
                QString type = args[0];

                if (!type.compare("none", Qt::CaseInsensitive)) {
                    disableAlphaBlending = true;
                } else if (!type.compare("alpha", Qt::CaseInsensitive)) {
                    disableAlphaBlending = false;
                } else if (!type.compare("add", Qt::CaseInsensitive)) {
                    sourceBlendFactor = GL_ONE;
                    destBlendFactor = GL_ONE;
                } else if (!type.compare("alphaadd", Qt::CaseInsensitive)) {
                    sourceBlendFactor = GL_SRC_ALPHA;
                    destBlendFactor = GL_ONE;
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
        } else if (!command.compare("notlit", Qt::CaseInsensitive)) {
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

    TextureStageInfo::TextureStageInfo()
    {
       transformType = None;
       speedu = 0;
       speedv = 0;

       blendType = Modulate;

       // By default modulation takes place (Texture Color * Fragment Color for first stage)
       colorOp = GL_MODULATE;
       alphaOp = GL_MODULATE;

       // Argument 1 of multi texturing equations are the texture's color/alpha
       colorArg0 = GL_TEXTURE;
       colorOperand0 = GL_SRC_COLOR;

       alphaArg0 = GL_TEXTURE;
       alphaOperand0 = GL_SRC_ALPHA;

       // Argument 2 of multi texturing equations are the previous texture's color/alpha
       colorArg1 = GL_PREVIOUS;
       colorOperand1 = GL_SRC_COLOR;

       alphaArg1 = GL_PREVIOUS;
       alphaOperand1 = GL_SRC_ALPHA;
    }

    void TextureStageInfo::updateTransform(float elapsedSeconds)
    {
        switch (transformType)
        {
        case Drift:
            transformMatrix.translate(- elapsedSeconds * speedu, - elapsedSeconds * speedv);
            break;

        case Swirl:
            {
                elapsedSeconds *= 1000;
                float rotation = - (elapsedSeconds*speedu*60*1.0471976e-4f)*0.1f;
                transformMatrix.translate(-.5, -.5);
                transformMatrix.rotate(rad2deg(rotation), 0, 0, 1);
                transformMatrix.translate(.5, .5);
                break;
            }


        case Wavey:
            {
                float udrift = (elapsedSeconds*speedu);
                float vdrift = (elapsedSeconds*speedv);

                transformMatrix.translate(- udrift - cos(udrift*2*Pi), - vdrift - cos(vdrift*2*Pi));
            }
            break;

        default:
            break;
        }
    }

}
