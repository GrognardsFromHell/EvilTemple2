#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <QGLContext>

namespace EvilTemple {

    // Interface for Materials
    class Material
    {    
    public:        
        virtual ~Material();
        virtual void bind(QGLContext *context) = 0;
        virtual void unbind(QGLContext *context) = 0;

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
    protected:
        explicit Material(const QString &name);
    private:
        QString _name;
    };

}

#endif // MATERIAL_H
