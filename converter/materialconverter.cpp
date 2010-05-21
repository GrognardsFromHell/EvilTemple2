
#include <QString>
#include <QCryptographicHash>

#include "virtualfilesystem.h"
#include "material.h"


#include "util.h"
#include "materialconverter.h"

using namespace Troika;

class MaterialConverterData
{
private:
    VirtualFileSystem *mVfs;
    QString mMaterialTemplate;
    QStringList shadowCasterList;

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
            QString line = shadowCasterStream.readLine().trimmed();
            if (line.isEmpty())
                continue;
            shadowCasterList.append(QDir::toNativeSeparators(line.toLower()));
        }
    }

    bool isShadowCaster(const QString &filename)
    {
        QString comparison = QDir::toNativeSeparators(filename.toLower());
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

        pixelTerm.append("gl_FragColor = materialColor;\n");

        for (int i = 0; i < LegacyTextureStages; ++i) {
            const TextureStageInfo *textureStage = material->getTextureStage(i);

            if (textureStage->filename.isEmpty())
                continue;

            QString samplerName = QString("texSampler%1").arg(i);

			int textureId = getTexture(textureStage->filename); // This forces the texture to be loaded -> ok
			
            samplers.append(QString("uniform sampler2D %1;\n").arg(samplerName));
			if (external) {
				textureDefs.append(QString("<textureSampler texture=\"%1\"/>\n").arg(getNewTextureFilename(textureStage->filename)));
			} else {				
				textureDefs.append(QString("<textureSampler texture=\"#%1\"/>\n").arg(textureId));
			}

            samplerUniforms.append(QString("<uniform name=\"%1\" semantic=\"Texture%2\" />").arg(samplerName).arg(i));

            pixelTerm.append(QString("texel = texture2D(%1, texCoord);\n").arg(samplerName));

            switch (textureStage->blendType) {
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

        if (material->isLightingDisabled()) {
            QRegExp lightingBlocks("\\{\\{LIGHTING_ON\\}\\}.+\\{\\{\\/LIGHTING_ON\\}\\}");
            lightingBlocks.setMinimal(true);
            materialFile.replace(lightingBlocks, "");
        } else {
            materialFile.replace("{{LIGHTING_ON}}", "");
            materialFile.replace("{{/LIGHTING_ON}}", "");
            pixelTerm.append("gl_FragColor = gl_FragColor * Idiff;\n");
        }

        QColor color = material->getColor();
        materialFile.replace("{{MATERIAL_DIFFUSE}}", QString("%1, %2, %3, %4").arg(color.redF()).arg(color.greenF()).arg(color.blueF()).arg(color.alphaF()));
        materialFile.replace("{{PIXEL_TERM}}", pixelTerm);
        materialFile.replace("{{SAMPLERS}}", samplers);
        materialFile.replace("{{SAMPLER_UNIFORMS}}", samplerUniforms);
        materialFile.replace("{{TEXTURES}}", textureDefs);
        materialFile.replace("{{CULL_FACE}}", material->isFaceCullingDisabled() ? "false" : "true");
        materialFile.replace("{{BLEND}}", material->isAlphaBlendingDisabled() ? "false" : "true");
        materialFile.replace("{{DEPTH_WRITE}}", material->isDepthWriteDisabled() ? "false" : "true");
        materialFile.replace("{{DEPTH_TEST}}", material->isDepthTestDisabled() ? "false" : "true");

        QString blendFactor;
        switch (material->getSourceBlendFactor()) {
        case GL_ZERO:
            blendFactor = "zero";
            break;
        case GL_ONE:
            blendFactor = "one";
            break;
        case GL_SRC_ALPHA:
            blendFactor = "srcAlpha";
            break;
        }
        materialFile.replace("{{BLEND_SRC}}", blendFactor);

        switch (material->getDestBlendFactor()) {
        case GL_ZERO:
            blendFactor = "zero";
            break;
        case GL_ONE:
            blendFactor = "one";
            break;
        case GL_SRC_ALPHA:
            blendFactor = "srcAlpha";
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            blendFactor = "oneMinusSrcAlpha";
            break;
        }
        materialFile.replace("{{BLEND_DEST}}", blendFactor);

        materialScripts.insert(getNewMaterialFilename(material->name()), HashedData(materialFile.toUtf8()));

        return true;
    }

    int getTexture(const QString &filename) {
        if (loadedTextures.contains(filename.toLower())) {
            return loadedTextures[filename.toLower()];
        } else {
            QByteArray texture = mVfs->openFile(filename);
            int textureId = textures.size();
			textures.insert(getNewTextureFilename(filename), texture);

            loadedTextures[filename.toLower()] = textureId;
            return textureId;
        }
    }

    QHash<QString, uint> loadedTextures;

    QMap<QString,HashedData> textures;
    QMap<QString,HashedData> materialScripts;
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

QDataStream &operator <<(QDataStream &stream, const HashedData &hashedData)
{
    Q_ASSERT(hashedData.md5Hash.size() == 16);

    stream.writeRawData(hashedData.md5Hash.constData(), 16);
    stream << hashedData.data.size();
    stream.writeRawData(hashedData.data.constData(), hashedData.data.size());

    return stream;
}
