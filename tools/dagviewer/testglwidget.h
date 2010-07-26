#ifndef TESTGLWIDGET_H
#define TESTGLWIDGET_H

#include <QGLWidget>

#include <gamemath.h>
using namespace GameMath;

struct Face {
    ushort index[3];
};

class TestGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit TestGLWidget(QWidget *parent = 0);
    ~TestGLWidget();

    void open(const QString &filename);

signals:

public slots:

    void setScaleX(float value) {
        mScaleX = value;
        repaint();
    }

    void setScaleY(float value) {
        mScaleY = value;
        repaint();
    }

    void setScaleZ(float value) {
        mScaleZ = value;
        repaint();
    }

    void setRotation(float value) {
        mRotation = value;
        repaint();
    }

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    QString mFilename;

    Vector4 *mVertices;
    Vector4 *mTransformedVertices;
    Face *mFaces;
    uint mVertexCount;
    uint mFaceCount;

    float mScaleX, mScaleY, mScaleZ, mRotation;

};

#endif // TESTGLWIDGET_H
