
#include <QDataStream>
#include <QtOpenGL>
#include <QPainter>

#include "testglwidget.h"

#include "util.h"

const float DagMagicConstant = 0.70710599f; // 40.5 degrees. This is used in the scene rotation.

/**
  Calculates a face normal.
  */
static Vector4 calculateNormal(const Face &face, const Vector4 *vertices)
{
    // Find all faces that use this vertex
    int index1 = face.index[0];
    int index2 = face.index[1];
    int index3 = face.index[2];

    Vector4 thisPos = vertices[index1];

    return (vertices[index2] - thisPos).cross(vertices[index3] - thisPos).normalized();
}

/**
  Transforms DAG vertices according to the given parameters.
  */
static void transformVertices(const Vector4 *vertexIn, int count,
                              Vector4 *vertexOut,
                              float scaleX, float scaleY, float scaleZ, float rotation)
{
    rotation = deg2rad(rotation);

    float rotSin = sinf(rotation);
    float rotCos = cosf(rotation);

    for (int i = 0; i < count; ++i) {
        float x = vertexIn[i].x();
        float y = vertexIn[i].y();
        float z = vertexIn[i].z();

        float nx, ny, nz;

        // Apply rotation / flip coordinate system.
        nx = - (rotCos * x + rotSin * y);
        ny = z;
        nz = (- rotCos * y) - (- rotSin * x);

        float tx = 0.5f * scaleX * (nz - nx);
        float ty = 0.5f * scaleY * (nx + nz);

        x = ty - tx;
        y = scaleZ * ny;
        z = tx + ty;

        vertexOut[i] = Vector4(x, y, z, 1);
    }
}

static void renderFaces(const Face *faces, int count, const Vector4 *vertices)
{
    for (int i = 0; i < count; ++i) {
        const Face &face = faces[i];

        for (int j = 0; j < 3; ++j) {
            ushort index = face.index[j];
            glNormal3fv(calculateNormal(face, vertices).data());
            glVertex3fv(vertices[index].data());
        }
    }
}

TestGLWidget::TestGLWidget(QWidget *parent) :
    QGLWidget(parent), mVertices(NULL), mFaces(NULL), mVertexCount(0), mFaceCount(0),
                    mTransformedVertices(NULL),
                    mScaleX(1), mScaleY(1), mScaleZ(1), mRotation(0)
{
}

TestGLWidget::~TestGLWidget()
{
    delete [] mTransformedVertices;
    delete [] mVertices;
    delete [] mFaces;
}

void TestGLWidget::initializeGL()
{
    glClearColor(1, 1, 1, 1);

    resizeGL(width(), height());
}

static void setLighting()
{
    Vector4 ambient(0, 0, 0, 0);
    Vector4 diffuse(0.6f, 0.6f, 0.6f, 1);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient.data());
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse.data());

    Vector4 lightDirection(0, 0, 0, 0);
    lightDirection.setX(-0.6324093645670703858428703903848f);
    lightDirection.setY(-0.77463436252716949786709498111783f);
    glLightfv(GL_LIGHT0, GL_POSITION, lightDirection.data());

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void TestGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    setLighting();

    glEnable(GL_DEPTH_TEST);

    // Transform + recalc normals
    transformVertices(mVertices, mVertexCount, mTransformedVertices,
                      mScaleX, mScaleY, mScaleZ, mRotation);

    // Render the DAG file directly
    glBegin(GL_TRIANGLES);
    glColor3f(1, 1, 1);
    renderFaces(mFaces, mFaceCount, mTransformedVertices);
    glEnd();

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);
    qglColor(QColor(0, 0, 0));
    renderText(20, 20, mFilename);
    glEnable(GL_LIGHTING);
}

void TestGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    const float zoom = 1;

    Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth / zoom,
                                              halfWidth / zoom,
                                              -halfHeight / zoom,
                                              halfHeight / zoom,
                                              1, 5000);
    glLoadMatrixf(projectionMatrix.data());

    glMatrixMode(GL_MODELVIEW);

    // Old: -44
    Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, deg2rad(-44.42700648682643f));
    Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

    // Old: 90-135
    Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(135.0000005619373f));
    Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

    Matrix4 flipZMatrix;
    flipZMatrix.setToIdentity();
    flipZMatrix(2, 2) = -1;

    Matrix4 id;
    id.setToIdentity();
    id(2,3) = -3000;

    Matrix4 baseViewMatrix = id * flipZMatrix * rotate1matrix * rotate2matrix;

    glLoadMatrixf(baseViewMatrix.data());
}

void TestGLWidget::open(const QString &filename)
{
    mFilename = filename;

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        QErrorMessage errorMessage(this);
        errorMessage.showMessage("Unable to open " + filename + ": " + file.errorString());
        return;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    float bsRadius, bsX, bsY, bsZ;
    stream >> bsX >> bsY >> bsZ >> bsRadius;

    qDebug("Bounding sphere radius: %f with center @ %f,%f,%f", bsRadius, bsX, bsY, bsZ);

    uint nodeCount;
    stream >> nodeCount;

    qDebug("Depth art nodes: %d", nodeCount);

    Q_ASSERT(nodeCount == 1); // The exporter may have supported multiple nodes. But it seems to be unused.

    stream.skipRawData(sizeof(uint)); // Skip the data offset, since the data is always consecutive anyway

    stream >> mVertexCount >> mFaceCount;

    stream.skipRawData(sizeof(uint) * 2); // Following two fields were data offsets for vertices and faces (Unused).

    delete [] mVertices;
    mVertices = new Vector4[mVertexCount];
    delete [] mTransformedVertices;
    mTransformedVertices = new Vector4[mVertexCount];

    for (uint i = 0; i < mVertexCount; ++i) {
        float x, y, z;
        stream >> x >> y >> z;
        mVertices[i] = Vector4(x, y, z, 1);
    }

    delete [] mFaces;
    mFaces = new Face[mFaceCount];

    for (uint i = 0; i < mFaceCount; ++i) {
        stream >> mFaces[i].index[0] >> mFaces[i].index[1] >> mFaces[i].index[2];
    }

    repaint();
}
