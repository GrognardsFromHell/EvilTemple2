#ifndef TESTGLWIDGET_H
#define TESTGLWIDGET_H

#include <QGLWidget>
#include <QScopedPointer>

#include "../../game/skeleton.h"
using namespace EvilTemple;

#include <gamemath.h>
using namespace GameMath;

class ViewerData;

struct Face {
    ushort index[3];
};

#include "animinfo.h"

class Viewer : public QGLWidget
{
    Q_OBJECT
public:
    explicit Viewer(QWidget *parent = 0);
    ~Viewer();

    void open(const QString &filename);

signals:

public slots:

    void selectBone(int bone);

    void nextFrame();

    void playAnimation(const QString &name);

    QList<AnimationInfo> animations() const;

    void setScale(float value) {
        mScale = value;
        repaint();
    }

    void setRotation(float value) {
        mRotation = value;
        repaint();
    }

    void setDrawGeometry(bool draw) {
        mDrawGeometry = draw;
    }

    bool isDrawGeometry() const {
        return mDrawGeometry;
    }

    bool showNormals() const {
        return mShowNormals;
    }

    void setShowNormals(bool showNormals) {
        mShowNormals = showNormals;
        repaint();
    }

    bool isDrawBones() const {
        return mDrawBones;
    }

    void setDrawBones(bool draw) {
        mDrawBones = draw;
        repaint();
    }

    bool isDrawClothBones() const
    {
        return mDrawClothBones;
    }

    void setDrawClothBones(bool drawClothBones)
    {
        mDrawClothBones = drawClothBones;
        repaint();
    }

    void setShowBoneNames(bool enabled) {
        mShowBoneNames = enabled;
        repaint();
    }

    bool isShowBoneNames() const {
        return mShowBoneNames;
    }

    bool isRenderGroundPlane() const {
        return mRenderGroundPlane;
    }

    void setRenderGroundPlane(bool enabled) {
        mRenderGroundPlane = enabled;
    }

    const Skeleton *skeleton() const;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    QString mFilename;

    float mScale, mRotation;
    bool mShowNormals;
    bool mDrawGeometry;
    bool mDrawBones;
    bool mDrawClothBones;
    bool mShowBoneNames;
    bool mRenderGroundPlane;

    QScopedPointer<ViewerData> d;

};

#endif // TESTGLWIDGET_H
