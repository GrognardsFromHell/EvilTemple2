
#include <QString>
#include <QCryptographicHash>

#include "virtualfilesystem.h"
#include "troika_material.h"


#include "util.h"
#include "materialconverter.h"

using namespace Troika;

class MaterialConverterData
{
private:
    VirtualFileSystem *mVfs;
    QString mMaterialTemplate;
    QStringList shadowCasterList;
    QStringList shadowCasterExclusions;

public:
    bool external;

    MaterialConverterData(VirtualFileSystem *vfs) : mVfs(vfs) {
        QFile materialTemplateFile(":/material_template.xml");
        if (!materialTemplateFile.open(QIODevice::ReadOnly)) {
            qFatal("Unable to open material template file.");
        }

        mMaterialTemplate = QString::fromUtf8(materialTemplateFile.readAll().data());
        materialTemplateFile.close();

        QFile shadowCaster(":/shadow_caster.txt");

        if (!shadowCaster.open(QIODevice::ReadOnly|QIODevice::Text)) {
            qFatal("Unable to load shadow caster material list.");
        }

        QTextStream shadowCasterStream(&shadowCaster);

        while (!shadowCasterStream.atEnd()) {
            QString line = shadowCasterStream.readLine().trimmed().toLower();
            if (line.startsWith('#') || line.isEmpty())
                continue;
            if (line.startsWith('-')) {
                shadowCasterExclusions.append(QDir::cleanPath(QDir::toNativeSeparators(line.mid(1))));
            } else {
                shadowCasterList.append(QDir::cleanPath(QDir::toNativeSeparators(line)));
            }
        }
    }

    bool isShadowCaster(const QString &filename)
    {
        QString comparison = QDir::cleanPath(QDir::toNativeSeparators(filename.toLower()));
        foreach (const QString &exclusion, shadowCasterExclusions) {
            if (comparison.startsWith(exclusion)) {
                return false;
            }
        }

        foreach (const QString &shadowCaster, shadowCasterList) {
            if (comparison.startsWith(shadowCaster)) {
                return true;
            }
        }
        return false;
    }

    bool convert(const Material *material)
    {
        Q_ASSERT(!mMaterialTemplate.isNull());

        bool textureAnimation = false;
        bool sphereMap = false;
        bool useNormals = false;
        QString materialFile = mMaterialTemplate;

        QString textureDefs = "";
        QString samplers = "";
        QString pixelTerm = "vec4 texel;\n";
        QString samplerUniforms = "";

        if (isShadowCaster(material->name())) {
            materialFile.replace("{{SHADOW_ON}}", "");
            materialFile.replace("{{/SHADOW_ON}}", "");
        } else {
            QRegExp lightingBlocks("\\{\\{SHADOW_ON\\}\\}.+\\{\\{\\/SHADOW_ON\\}\\}");
            lightingBlocks.setMinimal(true);
            materialFile.replace(lightingBlocks, "");
        }

        int samplersUsed = 0;

        for (int i = 0; i < LegacyTextureStages; ++i) {
            const TextureStageInfo *textureStage = material->getTextureStage(i);

            if (textureStage->filename().isEmpty())
                continue;

            QString samplerName = QString("texSampler%1").arg(samplersUsed);

            int textureId = getTexture(textureStage->filename()); // This forces the texture to be loaded -> ok

            samplers.append(QString("uniform sampler2D %1;\n").arg(samplerName));
            if (external) {
                textureDefs.append(QString("<textureSampler texture=\"%1\"/>\n").arg(getNewTextureFilename(textureStage->filename())));
            } else {
                textureDefs.append(QString("<textureSampler texture=\"#%1\"/>\n").arg(textureId));
            }

            samplerUniforms.append(QString("<uniform name=\"%1\" semantic=\"Texture%2\" />").arg(samplerName).arg(samplersUsed));

            ++samplersUsed;

            /*
             The texel on the texture of this texture stage may be retrieved from texture coordinates that
             are transformed (animated) first.
             */
            switch (textureStage->uvType()) {
            case TextureStageInfo::Mesh:
                pixelTerm.append(QString("texel = texture2D(%1, texCoord);\n").arg(samplerName));
                break;
            case TextureStageInfo::Drift:
                textureAnimation = true;
                if (textureStage->speedU() != 0 && textureStage->speedV() != 0) {
                    pixelTerm.append(QString("texel = texture2D(%1, textureDriftUV(texCoord, t, %2, %3));\n").arg(samplerName).arg(textureStage->speedU()).arg(textureStage->speedV()));
                } else if (textureStage->speedU() > 0) {
                    pixelTerm.append(QString("texel = texture2D(%1, textureDriftU(texCoord, t, %2));\n").arg(samplerName).arg(textureStage->speedU()));
                } else {
                    pixelTerm.append(QString("texel = texture2D(%1, textureDriftV(texCoord, t, %2));\n").arg(samplerName).arg(textureStage->speedV()));
                }
                break;
            case TextureStageInfo::Swirl:
                /*
                 Since swirl rotates, there is not much sense to use two different speed settings.
                 */
                textureAnimation = true;
                Q_ASSERT(textureStage->speedU() == textureStage->speedV());
                pixelTerm.append(QString("texel = texture2D(%1, textureSwirl(texCoord, t, %2));\n").arg(samplerName).arg(textureStage->speedU()));
                break;
            case TextureStageInfo::Wavey:
                textureAnimation = true;
                if (textureStage->speedU() != 0 && textureStage->speedV() != 0) {
                    pixelTerm.append(QString("texel = texture2D(%1, textureWaveyUV(texCoord, t, %2, %3));\n").arg(samplerName).arg(textureStage->speedU()).arg(textureStage->speedV()));
                } else if (textureStage->speedU() != 0) {
                    pixelTerm.append(QString("texel = texture2D(%1, textureWaveyU(texCoord, t, %2));\n").arg(samplerName).arg(textureStage->speedU()));
                } else {
                    pixelTerm.append(QString("texel = texture2D(%1, textureWaveyV(texCoord, t, %2));\n").arg(samplerName).arg(textureStage->speedV()));
                }
                break;
            case TextureStageInfo::Environment:
                sphereMap = true;
                useNormals = true;
                pixelTerm.append(QString("texel = texture2D(%1, sphereMapping());\n").arg(samplerName));
                break;
            default:
                qFatal("Invalid setting for texture stage transform: %d.", textureStage->uvType());
            }

            switch (textureStage->blendType()) {
            case TextureStageInfo::Modulate:
                pixelTerm.append("gl_FragColor = gl_FragColor * texel;\n");
                break;
            case TextureStageInfo::Add:
                pixelTerm.append("gl_FragColor = vec4(gl_FragColor.rgb + texel.rgb, gl_FragColor.a);\n");
                break;
            case TextureStageInfo::TextureAlpha:
                pixelTerm.append("gl_FragColor = vec4(mix(gl_FragColor.rgb, texel.rgb, texel.a), gl_FragColor.a);\n");
                break;
            case TextureStageInfo::CurrentAlpha:
                pixelTerm.append("gl_FragColor = vec4(mix(gl_FragColor.rgb, texel.rgb, gl_FragColor.a), materialColor.a);\n");
                break;
            case TextureStageInfo::CurrentAlphaAdd:
                pixelTerm.append("gl_FragColor.rgb += texel.rgb * gl_FragColor.a;\n");
                pixelTerm.append("gl_FragColor.a = materialColor.a;\n");
                break;
            }
        }

        // Use a constant material color if no lighting is enabled
        pixelTerm.prepend("gl_FragColor = materialColor;\n");

        if (material->isLightingDisabled()) {
            QRegExp lightingBlocks("\\{\\{LIGHTING_ON\\}\\}.+\\{\\{\\/LIGHTING_ON\\}\\}");
            lightingBlocks.setMinimal(true);
            materialFile.replace(lightingBlocks, "");
        } else {

            if (material->glossmap().isNull()) {
                // Use a constant material color if no lighting is enabled
                pixelTerm.append("gl_FragColor.xyz *= lighting(shininess, materialColor).xyz;\n");
            } else {
                int textureId = getTexture(material->glossmap()); // This forces the texture to be loaded -> ok

                samplers.append("uniform sampler2D texSamplerGlossmap;\n");
                if (external) {
                    textureDefs.append(QString("<textureSampler texture=\"%1\"/>\n").arg(getNewTextureFilename(material->glossmap())));
                } else {
                    textureDefs.append(QString("<textureSampler texture=\"#%1\"/>\n").arg(textureId));
                }

                samplerUniforms.append(QString("<uniform name=\"texSamplerGlossmap\" semantic=\"Texture%2\" />").arg(samplersUsed++));

                pixelTerm.append("gl_FragColor.xyz *= lightingGlossmap(shininess, materialColor, texSamplerGlossmap, texCoord).xyz;\n");
            }

            useNormals = true;
            materialFile.replace("{{LIGHTING_ON}}", "");
            materialFile.replace("{{/LIGHTING_ON}}", "");
        }

        if (!textureAnimation) {
            QRegExp blocks("\\{\\{TEXTUREANIM_ON\\}\\}.+\\{\\{\\/TEXTUREANIM_ON\\}\\}");
            blocks.setMinimal(true);
            materialFile.replace(blocks, "");
        } else {
            materialFile.replace("{{TEXTUREANIM_ON}}", "");
            materialFile.replace("{{/TEXTUREANIM_ON}}", "");
        }

        if (!useNormals) {
            QRegExp blocks("\\{\\{NORMALS_ON\\}\\}.+\\{\\{\\/NORMALS_ON\\}\\}");
            blocks.setMinimal(true);
            materialFile.replace(blocks, "");
        } else {
            materialFile.replace("{{NORMALS_ON}}", "");
            materialFile.replace("{{/NORMALS_ON}}", "");
        }

        if (!sphereMap) {
            QRegExp blocks("\\{\\{SPHEREMAP_ON\\}\\}.+\\{\\{\\/SPHEREMAP_ON\\}\\}");
            blocks.setMinimal(true);
            materialFile.replace(blocks, "");
        } else {
            materialFile.replace("{{SPHEREMAP_ON}}", "");
            materialFile.replace("{{/SPHEREMAP_ON}}", "");
        }

        QColor color = material->getColor();
        materialFile.replace("{{MATERIAL_DIFFUSE_R}}", QString("%1").arg(color.redF()));
        materialFile.replace("{{MATERIAL_DIFFUSE_G}}", QString("%1").arg(color.greenF()));
        materialFile.replace("{{MATERIAL_DIFFUSE_B}}", QString("%1").arg(color.blueF()));
        materialFile.replace("{{MATERIAL_DIFFUSE_A}}", QString("%1").arg(color.alphaF()));
        materialFile.replace("{{PIXEL_TERM}}", pixelTerm);
        materialFile.replace("{{SAMPLERS}}", samplers);
        materialFile.replace("{{SAMPLER_UNIFORMS}}", samplerUniforms);
        materialFile.replace("{{TEXTURES}}", textureDefs);
        materialFile.replace("{{CULL_FACE}}", material->isFaceCullingDisabled() ? "false" : "true");
        if (material->blendType() != Material::None)
            materialFile.replace("{{BLEND}}", "true");
        else
            materialFile.replace("{{BLEND}}", "false");
        materialFile.replace("{{DEPTH_WRITE}}", material->isDepthWriteDisabled() ? "false" : "true");
        materialFile.replace("{{DEPTH_TEST}}", material->isDepthTestDisabled() ? "false" : "true");
        materialFile.replace("{{SPECULAR_POWER}}", QString("%1").arg(material->specularPower()));

        QString srcFactor, destFactor;
        bool alphaTest;
        switch (material->blendType()) {
        case Material::None:
            srcFactor = "one";
            destFactor = "zero";
            alphaTest = false;
            break;
        case Material::Alpha:
            srcFactor = "srcAlpha";
            destFactor = "oneMinusSrcAlpha";
            alphaTest = true;
            break;
        case Material::Add:
            srcFactor = "one";
            destFactor = "one";
            alphaTest = true;
            break;
        case Material::AlphaAdd:
            srcFactor = "srcAlpha";
            destFactor = "one";
            alphaTest = true;
            break;
        }

        if (!alphaTest) {
            QRegExp blocks("\\{\\{ALPHATEST_ON\\}\\}.+\\{\\{\\/ALPHATEST_ON\\}\\}");
            blocks.setMinimal(true);
            materialFile.replace(blocks, "");
        } else {
            materialFile.replace("{{ALPHATEST_ON}}", "");
            materialFile.replace("{{/ALPHATEST_ON}}", "");
        }

        materialFile.replace("{{BLEND_SRC}}", srcFactor);
        materialFile.replace("{{BLEND_DEST}}", destFactor);

        HashedData materialScriptData(materialFile.toUtf8());
        materialScripts.insert(getNewMaterialFilename(material->name()), materialScriptData);
        materialList.append(materialScriptData);

        return true;
    }

    int getTexture(const QString &filename) {
        QString key = getNewTextureFilename(filename);

        if (loadedTextures.contains(key)) {
            return loadedTextures[key];
        } else {
            QByteArray texture = mVfs->openFile(filename);
            int textureId = textures.size();
            textures.insert(key, texture);

            textureList.append(HashedData(texture));

            loadedTextures[key] = textureId;
            return textureId;
        }
    }

    QHash<QString, uint> loadedTextures;

    QMap<QString,HashedData> textures;
    QMap<QString,HashedData> materialScripts;
    QList<HashedData> materialList;
    QList<HashedData> textureList;
};

HashedData::HashedData(const QByteArray &_data) : data(_data), md5Hash(QCryptographicHash::hash(_data, QCryptographicHash::Md5))
{

}

MaterialConverter::MaterialConverter(VirtualFileSystem *vfs)
    : d_ptr(new MaterialConverterData(vfs))
{
}

MaterialConverter::~MaterialConverter()
{
}

bool MaterialConverter::convert(const Material *material)
{
    return d_ptr->convert(material);
}

const QMap<QString,HashedData> &MaterialConverter::textures()
{
    return d_ptr->textures;
}

const QMap<QString,HashedData> &MaterialConverter::materialScripts()
{
    return d_ptr->materialScripts;
}

void MaterialConverter::setExternal(bool external)
{
    d_ptr->external = external;
}

QList<HashedData> MaterialConverter::textureList()
{
    return d_ptr->textureList;
}

QList<HashedData> MaterialConverter::materialList()
{
    return d_ptr->materialList;
}

QDataStream &operator <<(QDataStream &stream, const HashedData &hashedData)
{
    Q_ASSERT(hashedData.md5Hash.size() == 16);

    stream.writeRawData(hashedData.md5Hash.constData(), 16);
    stream << hashedData.data.size();
    stream.writeRawData(hashedData.data.constData(), hashedData.data.size());

    return stream;
}
