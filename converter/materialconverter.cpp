
#include <QString>

#include "virtualfilesystem.h"
#include "material.h"

#include "materialconverter.h"

using namespace Troika;

class MaterialConverterData
{
private:
    VirtualFileSystem *mVfs;
    QString mMaterialTemplate;

public:
    MaterialConverterData(VirtualFileSystem *vfs) : mVfs(vfs) {
        QFile materialTemplateFile(":/material_template.xml");
        if (!materialTemplateFile.open(QIODevice::ReadOnly)) {
            qFatal("Unable to open material template file.");
        }

        mMaterialTemplate = QString::fromUtf8(materialTemplateFile.readAll().data());
        materialTemplateFile.close();
    }

    bool convert(const Material *material) {
        Q_ASSERT(!mMaterialTemplate.isNull());

        QString materialFile = mMaterialTemplate;

        QString textureDefs = "";
        QString samplers = "";
        QString pixelTerm = "vec4 texel;\n";
        QString samplerUniforms = "";

        // TODO: Only perform if lighting is enabled
        if (material->isLightingDisabled()) {
            pixelTerm.append("vec4 diffuseColor = materialDiffuse;\n");
        } else {
            pixelTerm.append("vec4 diffuseColor = Idiff;\n");
        }
        pixelTerm.append("gl_FragColor = diffuseColor;\n");

        for (int i = 0; i < LegacyTextureStages; ++i) {
            const TextureStageInfo *textureStage = material->getTextureStage(i);

            if (textureStage->filename.isEmpty())
                continue;

            QString samplerName = QString("texSampler%1").arg(i);

            samplers.append(QString("uniform sampler2D %1;\n").arg(samplerName));
            textureDefs.append(QString("<textureSampler texture=\"#%1\"/>\n").arg(textures.size()));

            textures.append(mVfs->openFile(textureStage->filename));

            samplerUniforms.append(QString("<uniform name=\"%1\" semantic=\"Texture%2\" />").arg(samplerName).arg(i));

            pixelTerm.append(QString("texel = texture2D(%1, texCoord);\n").arg(samplerName));

            switch (textureStage->blendType) {
            case TextureStageInfo::Modulate:
                pixelTerm.append("gl_FragColor = gl_FragColor * texel;\n");
                break;
            case TextureStageInfo::Add:
                pixelTerm.append("gl_FragColor = vec4(vec3(gl_FragColor + texel), gl_FragColor.a);\n");
                break;
            case TextureStageInfo::TextureAlpha:
                pixelTerm.append("gl_FragColor = vec4(mix(vec3(gl_FragColor), vec3(texel), texel.a), gl_FragColor.a);\n");
                break;
            case TextureStageInfo::CurrentAlpha:
                pixelTerm.append("gl_FragColor = vec4(mix(vec3(gl_FragColor), vec3(texel), gl_FragColor.a), diffuseColor.a);\n");
                break;
            case TextureStageInfo::CurrentAlphaAdd:
                pixelTerm.append("gl_FragColor = vec4(vec3(gl_FragColor.r, gl_FragColor.g, gl_FragColor.b) + vec3(texel) * vec3(gl_FragColor.a, gl_FragColor.a, gl_FragColor.a), diffuseColor.a);\n");
                break;
            }
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

        materialScripts.append(materialFile.toUtf8());

        return true;
    }

    QList<QByteArray> textures;
    QList<QByteArray> materialScripts;
};

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

const QList<QByteArray> &MaterialConverter::textures()
{
    return d_ptr->textures;
}

const QList<QByteArray> &MaterialConverter::materialScripts()
{
    return d_ptr->materialScripts;
}
