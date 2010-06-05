#ifndef MATERIAL_H
#define MATERIAL_H

#include "troikaformatsglobal.h"

#include <QString>
#include <QGLContext>
#include <QtOpenGL>

namespace Troika {

    class VirtualFileSystem;

    /**
      The number of multi texturing stages used in legacy material files.
      */
    const int LegacyTextureStages = 3;

    class TROIKAFORMATS_EXPORT TextureStageInfo {
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

        enum BlendType
        {
            Modulate,
            Add,
            TextureAlpha,
            CurrentAlpha,
            CurrentAlphaAdd
        };

        BlendType blendType;

        QString filename;
        QImage image;

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

        TextureStageInfo();

        void updateTransform(float elapsedSeconds);
    };

    // Interface for Materials
    class TROIKAFORMATS_EXPORT Material
    {    
    public:

        enum Type {
            UserDefined, // Through MDF files
            DepthArt, // Used by the depth geometry
            Placeholder
        };

        explicit Material(Type type, const QString &name);
        virtual ~Material();

        /**
            Checks whether the given texture coordinates are transparent or not.
            The default implementation always returns true. Re-implement this method
            if your material has transparent parts that should be click-through.

            @returns True if the texture coordinates on this material are a hit.
        */
        virtual bool hitTest(float u, float v);

        const QString &name() const {
            return _name;
        }

        Type type() const {
            return mType;
        }

        static Material *create(VirtualFileSystem *vfs, const QString &filename);

        bool isFaceCullingDisabled() const {
            return disableFaceCulling;
        }

        bool isLightingDisabled() const {
            return disableLighting;
        }

        bool isDepthTestDisabled() const {
            return disableDepthTest;
        }

        bool isLinearFiltering() const {
            return linearFiltering;
        }

        bool isAlphaBlendingDisabled() const {
            return disableAlphaBlending;
        }

        bool isDepthWriteDisabled() const {
            return disableDepthWrite;
        }

        GLenum getSourceBlendFactor() const {
            return sourceBlendFactor;
        }

        GLenum getDestBlendFactor() const {
            return destBlendFactor;
        }

        const QColor &getColor() const {
            return color;
        }

        const TextureStageInfo *getTextureStage(int stage) const {
            return &textureStages[stage];
        }

    private:
        QString _name;
        Type mType;

        bool processCommand(VirtualFileSystem *vfs, const QString &command, const QStringList &args);
        bool disableFaceCulling;
        bool disableLighting;
        bool disableDepthTest;
        bool disableDepthWrite;
        bool linearFiltering;
        bool disableAlphaBlending;
        GLenum sourceBlendFactor;
        GLenum destBlendFactor;
        QColor color;

        TextureStageInfo textureStages[LegacyTextureStages];
    };

}

#endif // MATERIAL_H
