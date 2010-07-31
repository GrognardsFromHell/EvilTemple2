#ifndef MATERIAL_H
#define MATERIAL_H

#include "troikaformatsglobal.h"

#include <QString>
#include <QGLContext>

namespace Troika {

    class VirtualFileSystem;

    /**
      The number of multi texturing stages used in legacy material files.
      */
    const int LegacyTextureStages = 3;

    /**
      Describes a texturing stage declaration in a legacy material file.
      */
    class TROIKAFORMATS_EXPORT TextureStageInfo {
    public:
        /**
          Transforms used for texture coordinates.
          */
        enum UvType
        {
            Mesh = 0, // No texture matrix is used
            Drift, // Texture coordinate drifts linearily (Translation along one axis)
            Swirl, // Texture coordinates rotate
            Wavey, // Like drift but with a cosine/sine acceleration pattern
            Environment // Uses the surface normal
        };

        enum BlendType
        {
            Modulate,
            Add,
            TextureAlpha,
            CurrentAlpha,
            CurrentAlphaAdd
        };

        /**
          Constructs a new texture stage with the following default values:
          - blendType is set to Modulate
          - filename is null
          - uvType is Mesh
          - speedU is 1
          - speedV is 0
          */
        TextureStageInfo();

        /**
          Returns the blending type for multi-texturing that is applied to this texture stage.
          */
        BlendType blendType() const;

        void setBlendType(BlendType blendType);

        /**
          Returns the filename of the texture that is used in this stage.
          */
        const QString &filename() const;

        void setFilename(const QString &filename);

        /**
          Returns the type of UV coordinates for this texture stage.
          */
        UvType uvType() const;

        void setUvType(UvType uvType);

        /**
          If the UV coordinates are animated, this returns the number of loops per minute
          for this texture's U coordinates.
          */
        float speedU() const;

        void setSpeedU(float speed);

        /**
          If the UV coordinates are animated, this returns the number of loops per minute
          for this texture's V coordinates.
          */
        float speedV() const;

        void setSpeedV(float speed);

    private:
        BlendType mBlendType;
        QString mFilename;
        UvType mUvType;
        float mSpeedU, mSpeedV;
    };

    inline TextureStageInfo::TextureStageInfo()
        : mBlendType(Modulate),
        mUvType(Mesh),
        mSpeedU(1),
        mSpeedV(1)
    {
    }

    inline TextureStageInfo::BlendType TextureStageInfo::blendType() const
    {
        return mBlendType;
    }

    inline void TextureStageInfo::setBlendType(BlendType blendType)
    {
        mBlendType = blendType;
    }

    inline const QString &TextureStageInfo::filename() const
    {
        return mFilename;
    }

    inline void TextureStageInfo::setFilename(const QString &filename)
    {
        mFilename = filename;
    }

    inline TextureStageInfo::UvType TextureStageInfo::uvType() const
    {
        return mUvType;
    }

    inline void TextureStageInfo::setUvType(UvType uvType)
    {
        mUvType = uvType;
    }

    inline float TextureStageInfo::speedU() const
    {
        return mSpeedU;
    }

    inline void TextureStageInfo::setSpeedU(float speed)
    {
        mSpeedU = speed;
    }

    inline float TextureStageInfo::speedV() const
    {
        return mSpeedV;
    }

    inline void TextureStageInfo::setSpeedV(float speed)
    {
        mSpeedV = speed;
    }

    // Interface for Materials
    class TROIKAFORMATS_EXPORT Material
    {
    public:

        enum Type {
            UserDefined, // Through MDF files
            DepthArt, // Used by the depth geometry
            Placeholder
        };

        enum BlendType {
            None,
            Alpha,
            Add,
            AlphaAdd
        };

        explicit Material(Type type, const QString &name);
        virtual ~Material();

        const QString &name() const {
            return _name;
        }

        Type type() const {
            return mType;
        }

        BlendType blendType() const {
            return mBlendType;
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

        bool isDepthWriteDisabled() const {
            return disableDepthWrite;
        }

        const QColor &getColor() const {
            return color;
        }

        const TextureStageInfo *getTextureStage(int stage) const {
            return &textureStages[stage];
        }

        float specularPower() const {
            return mSpecularPower;
        }

        const QString &glossmap() const {
            return mGlossmap;
        }

        bool isRecalculateNormals() const {
            return mRecalculateNormals;
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
        BlendType mBlendType;
        bool mRecalculateNormals;
        float mSpecularPower;
        QColor color;
        QString mGlossmap;

        TextureStageInfo textureStages[LegacyTextureStages];
    };

}

#endif // MATERIAL_H
