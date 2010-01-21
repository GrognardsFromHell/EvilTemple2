
#include <QTime>
#include <QMatrix4x4>
#include <QColor>

#include "materials.h"
#include "material.h"
#include "io/virtualfilesystem.h"
#include "glext.h"
#include "util.h"

static PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
static PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = NULL;

namespace EvilTemple {

    /**
      The number of multi texturing stages used in legacy material files.
      */
    const int LegacyTextureStages = 3;

    #define GL_ASSERT(x) (x); handleGlError(__FILE__, __LINE__);    

    inline void handleGlError(const char *file, int line)
    {
        GLenum error = glGetError();
        switch (error)
        {
        case GL_NO_ERROR:
            return;
        default:            
            qWarning("GL error: %s @ %s:%d", gluErrorString(error), file, line);
        }
    }

    QTime TransformTimer;

    class TextureStageInfo {
    public:
        /**
          Transforms used for texture coordinates.
          */
        enum TransformType
        {
            None = 0, // No texture matrix is used
            Drift, // Texture coordinate drifts linearily (Translation along one axis)
            Swirl, // Texture coordinates rotate
            Wavey, // Like drift but with a cosine/sine acceleration pattern
        };

        QString filename;
        QImage image;
        GLuint handle;

        GLenum colorOp;
        GLenum alphaOp;

        GLenum colorArg0;
        GLenum colorOperand0;

        GLenum alphaArg0;
        GLenum alphaOperand0;

        GLenum colorArg1;
        GLenum colorOperand1;

        GLenum alphaArg1;
        GLenum alphaOperand1;

        /**
          Texture transform speed.
          */
        float speedu, speedv;        
        TransformType transformType;
        QMatrix4x4 transformMatrix;

        TextureStageInfo() {
            handle = 0;
            transformType = None;
            speedu = 0;
            speedv = 0;

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

        void updateTransform(float elapsedSeconds)
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
    };

    /*      
      Material to handle MDF files
     */
    class MdfMaterial : public Material {
    public:
        void bind(QGLContext *context);
        void unbind(QGLContext *context);
        bool hitTest(float u, float v);

        static Material *create(VirtualFileSystem *vfs, const QString &filename);
    private:
        MdfMaterial(const QString &filename);
        ~MdfMaterial();
        bool processCommand(VirtualFileSystem *vfs, const QString &command, const QStringList &args);
        bool disableFaceCulling;
        bool disableLighting;
        bool disableDepthTest;
        bool linearFiltering;
        bool disableAlphaBlending;
        GLenum sourceBlendFactor;
        GLenum destBlendFactor;
        QColor color;

        TextureStageInfo textureStages[LegacyTextureStages];
    };

    MdfMaterial::MdfMaterial(const QString &filename) :
            Material(filename),
            disableFaceCulling(false),
            disableLighting(false),
            disableDepthTest(false),
            linearFiltering(false),
            disableAlphaBlending(false),
            sourceBlendFactor(0),
            destBlendFactor(0)
    {
    }

    MdfMaterial::~MdfMaterial()
    {
        for (int i = 0; i < LegacyTextureStages; ++i)
        {
            if (textureStages[i].handle)
                glDeleteTextures(1, &textureStages[i].handle);
        }
    }

    bool MdfMaterial::hitTest(float u, float v)
    {
        // Only stage 0 is considered for a fast-negative test
        const TextureStageInfo &stage = textureStages[0];

        const QImage &image = stage.image;

        if (!image.isNull())
        {
            QRgb color = image.pixel(u * (image.width() - 1), (1 - v) * (image.height() - 1));

            if (qAlpha(color) == 0)
                return false;
        }

        return true;
    }

    void MdfMaterial::bind(QGLContext *context) {
        if (glActiveTexture == NULL) {
            glActiveTexture = (PFNGLACTIVETEXTUREPROC)context->getProcAddress("glActiveTexture");
        }

        if (glClientActiveTexture == NULL) {
            glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)context->getProcAddress("glClientActiveTexture");
        }

        if (TransformTimer.isNull())
            TransformTimer.start();

        // Transform intervals are measured in seconds
        double elapsedSecs = TransformTimer.elapsed() / 1000.f;

        for (int i = 0; i < LegacyTextureStages; ++i)
        {
            TextureStageInfo &stage = textureStages[i];

            if (stage.image.isNull())
                continue; // Skip unused texture stages            

            GL_ASSERT( glActiveTexture(GL_TEXTURE0 + i) );
            GL_ASSERT( glEnable(GL_TEXTURE_2D) );

            // If the transform type is non-null, set the transform matrix
            if (stage.transformType != TextureStageInfo::None)
            {
                stage.updateTransform(elapsedSecs);
                glMatrixMode(GL_TEXTURE);
                glPushMatrix();
                glLoadMatrixd(stage.transformMatrix.data());
                glMatrixMode(GL_MODELVIEW);
            }

            if (stage.handle)
            {
                GL_ASSERT( glBindTexture(GL_TEXTURE_2D, stage.handle) );
            }
            else
            {
                stage.handle = context->bindTexture(stage.image, GL_TEXTURE_2D);
            }

            GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) );
            GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) );

            if (linearFiltering)
            {
                GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
                GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
            }
            else
            {
                GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
                GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
            }

            // Change texture environment if applicable
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE) );

            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, stage.colorOp) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, stage.colorArg0) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, stage.colorOperand0) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, stage.colorArg1) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, stage.colorOperand1) );

            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, stage.alphaOp) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, stage.alphaArg0) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, stage.alphaOperand0) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, stage.alphaArg1) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, stage.alphaOperand1) );

            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_PREVIOUS ) );
            GL_ASSERT( glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA ) );
        }

        glActiveTexture(GL_TEXTURE0);

        if (disableFaceCulling)
            glDisable(GL_CULL_FACE);

        if (disableLighting)
            glDisable(GL_LIGHTING);

        if (disableDepthTest)
            glDisable(GL_DEPTH_TEST);

        if (disableAlphaBlending)
            glDisable(GL_BLEND);

        if (sourceBlendFactor || destBlendFactor)
            glBlendFunc(sourceBlendFactor, destBlendFactor);

        if (color.isValid())
            glColor4f(color.redF(), color.greenF(), color.blueF(), 0);
    }

    void MdfMaterial::unbind(QGLContext *context) {
        Q_UNUSED(context);

        for (int i = 0; i < LegacyTextureStages; ++i)
        {
            if (textureStages[i].image.isNull())
                continue; // Don't bother resetting unused stages

            glActiveTexture(GL_TEXTURE0 + i);
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            if (textureStages[i].transformType != TextureStageInfo::None)
            {
                glMatrixMode(GL_TEXTURE);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

        }

        glActiveTexture(GL_TEXTURE0);

        if (disableFaceCulling)
            glEnable(GL_CULL_FACE);

        if (disableLighting)
            glEnable(GL_LIGHTING);

        if (disableDepthTest)
            glEnable(GL_DEPTH_TEST);

        if (disableAlphaBlending)
            glEnable(GL_BLEND);

        if (sourceBlendFactor || destBlendFactor)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (color.isValid())
            glColor4f(1, 1, 1, 1);
    }

    bool MdfMaterial::processCommand(VirtualFileSystem *vfs, const QString &command, const QStringList &args) {

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
                textureStages[unit].image.loadFromData(data, "tga");
            } else {
                qWarning("Unknown texture %s referenced in %s.", qPrintable(texture), qPrintable(name()));
            }

            return true;

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
            }
            else if (!type.compare("add", Qt::CaseInsensitive))
            {
                qWarning("Unsupported type: %s", qPrintable(type));
            }
            else if (!type.compare("texturealpha", Qt::CaseInsensitive))
            {
                qWarning("Unsupported type: %s", qPrintable(type));
            }
            else if (!type.compare("currentalpha", Qt::CaseInsensitive))
            {
                stage.colorOp = GL_INTERPOLATE;

                stage.alphaOp = GL_REPLACE;
                stage.alphaArg0 = GL_PRIMARY_COLOR;
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
            if (!args.count() == 4) {
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

    Material *MdfMaterial::create(VirtualFileSystem *vfs, const QString &filename) {
        QByteArray rawContent = vfs->openFile(filename);
        QString content = QString::fromLocal8Bit(rawContent.data(), rawContent.size());

        MdfMaterial *material = new MdfMaterial(filename);

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

    /*
      Private implementation details class for the materials class.
     */
    typedef QHash< QString, QWeakPointer<Material> > MaterialCache;

    class MaterialsData {
    public:
        VirtualFileSystem *vfs;
        MaterialCache materials;
    };

    Materials::Materials(VirtualFileSystem *vfs, QObject *owner) :
            QObject(owner),
            d_ptr(new MaterialsData)
    {
        d_ptr->vfs = vfs;
    }

    Materials::~Materials() {

    }

    QSharedPointer<Material> Materials::loadFromFile(const QString &filename)
    {
        // See if the same material has already been loaded
        MaterialCache::iterator it = d_ptr->materials.find(filename);
        if (it != d_ptr->materials.end())
        {
            QSharedPointer<Material> material(*it);

            if (material)
            {
                return material;
            }
        }

        QSharedPointer<Material> material(MdfMaterial::create(d_ptr->vfs, filename));

        // Store the material under it's name in the map
        d_ptr->materials.insert(material->name(), QWeakPointer<Material>(material));

        return material;
    }

}
