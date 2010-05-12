
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <QtCore/QString>

#include "materialstate.h"
#include "renderstates.h"
#include "util.h"

using namespace GameMath;

namespace EvilTemple {

struct FaceGroup {
	MaterialState *material;
	uint elementCount;
	GLuint buffer;
	
	FaceGroup();
	~FaceGroup();
};

class Model {
public:
	Model();
	~Model();

        bool open(const QString &filename, const RenderStates &renderState);
	void close();

	Vector4 *positions;
	Vector4 *normals;
	float *texCoords;
	int vertices;

	GLuint positionBuffer;
	GLuint normalBuffer;
	GLuint texcoordBuffer;
	
	int faces;	
        QScopedArrayPointer<FaceGroup> faceGroups;

	void drawNormals() const;

	const QString &error() const;

private:
        QScopedArrayPointer<MaterialState> materialState;
	
        AlignedScopedPointer<char> vertexData;
        AlignedScopedPointer<char> faceData;
        AlignedScopedPointer<char> textureData;
	
	void loadVertexData();
	void loadFaceData();

	QString mError;
};

inline const QString &Model::error() const
{
	return mError;
}

}

#endif
