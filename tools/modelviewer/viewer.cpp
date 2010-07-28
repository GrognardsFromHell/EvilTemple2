
#include <QDataStream>
#include <QtOpenGL>
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QScopedArrayPointer>

#include "viewer.h"
#include "../../game/modelfilereader.h"
#include "../../game/modelfilechunks.h"
#include "../../game/skeleton.h"
#include "../../game/bindingpose.h"
#include "../../game/animation.h"

#include "util.h"

using namespace EvilTemple;

// Inverts the z coordinate for displaying models made for DirectX
GLfloat zInverter[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, -1, 0,
    0, 0, 0, 1
};

class ViewerData {
public:
    ModelGeometry geometry;
    ModelFaces faces;
    Skeleton skeleton;
    BindingPose bindingPose;
    GLUquadric *sphereQuadric;
    QScopedArrayPointer<Animation> animations;
    QHash<QByteArray, Animation*> animationMap;
    Animation *animation; // Playing animation
    Skeleton *animSkeleton; // Used for animation (copy of skeleton)

    void initializeQuadrics();
    void drawBones(float scale);
    void drawClothBones();
    void doSkinning(uint vertexId, Vector4 *vertex, Vector4 *normal, float *clothWeight);

    int frame;
    int selectedBone;
};

void ViewerData::doSkinning(uint vertexId, Vector4 *vertex, Vector4 *normal, float *clothWeight)
{
    if (bindingPose.attachmentsCount() == 0 || !animation)
        return;

    const BoneAttachment &attachment = bindingPose.attachment(vertexId);

    Vector4 origPos = *vertex;
    Vector4 origNormal = *normal;

    *vertex = Vector4(0, 0, 0, 1);
    *normal = Vector4(0, 0, 0, 0);

    for (int i = 0; i < attachment.count(); ++i) {
        ushort boneId = attachment.bones()[i];
        float weight = attachment.weights()[i];

        if (animSkeleton->bone(boneId)->name() == "#ClothBone") {
            if (*clothWeight != 0) {
                qWarning("Double weighted to cloth bone.");
            }
            *clothWeight = weight;
        }

        const Matrix4 &fullWorldInverse = bindingPose.fullWorldInverse(boneId);
        const Matrix4 &fullWorld = animSkeleton->bone(bindingPose.boneName(boneId))->fullWorld();

        *vertex += weight * (fullWorld * fullWorldInverse.mapPosition(origPos));
        *normal += weight * (fullWorld * fullWorldInverse.mapNormal(origNormal));
    }

    vertex->setW(1);
    normal->setW(0);
    normal->normalize();
}

void ViewerData::initializeQuadrics()
{
    if (!sphereQuadric) {
        sphereQuadric = gluNewQuadric();
        gluQuadricNormals(sphereQuadric, GLU_SMOOTH);
        gluQuadricOrientation(sphereQuadric, GLU_INSIDE);
    }
}

void ViewerData::drawBones(float scale)
{
    foreach (const Bone *bone, animSkeleton->bones()) {
        Vector4 boneOrigin = bone->fullWorld() * Vector4(0, 0, 0, 1);

        if (bone->boneId() == selectedBone) {
            glColor3f(1, 1, 1);
        } else {
            glColor3f(1, .5f, .5f);
        }

        glPushMatrix();
        glTranslatef(boneOrigin.x(), boneOrigin.y(), boneOrigin.z());
        if (bone->boneId() == selectedBone)
            gluSphere(sphereQuadric, 16 / scale, 5, 5);
        else
            gluSphere(sphereQuadric, 8 / scale, 5, 5);
        glPopMatrix();

        glColor3f(1, 1, 1);
        if (bone->parent()) {
            Vector4 parentOrigin = bone->parent()->fullWorld() * Vector4(0, 0, 0, 1);

            // Draw a line from bone to parent
            glBegin(GL_LINES);
            glVertex3fv(boneOrigin.data());
            glVertex3fv(parentOrigin.data());
            glEnd();
        }
    }
}

void ViewerData::drawClothBones()
{
    QErrorMessage errorMessage;

    foreach (const Bone *bone, animSkeleton->bones()) {
        if (!bone->name().startsWith("#Sphere")
            && !bone->name().startsWith("#Cylinder"))
            continue;

        glPushMatrix();
        glMultMatrixf(bone->fullWorld().data());

        if (bone->boneId() == selectedBone) {
            glColor3f(1, 0, 0);
        } else {
            glColor3f(0, 0, 1);
        }

        QRegExp sphereNamePattern("\\#Sphere\\[(\\d+)\\]=\\{([^\\}]+)\\}");
        QRegExp cylinderNamePattern("\\#Cylinder\\[(\\d+)\\]=\\{([^\\,]+),([^\\,]+)\\}");

        if (sphereNamePattern.exactMatch(bone->name())) {
            bool idOk, radiusOk;
            sphereNamePattern.cap(1).toUInt(&idOk);
            float sphereRadius = sphereNamePattern.cap(2).toFloat(&radiusOk);
            if (!idOk) {
                errorMessage.showMessage(QString("Invalid id in '%1'").arg(QString::fromLatin1(bone->name())),
                                         "invalidName" + bone->name());
            }
            if (!radiusOk) {
                errorMessage.showMessage(QString("Invalid radius '%1'").arg(QString::fromLatin1(bone->name())),
                                         "invalidRadius" + bone->name());
            }

            gluSphere(sphereQuadric, sphereRadius, 10, 10);
        } else if (cylinderNamePattern.exactMatch(bone->name())) {
            bool idOk, radiusOk, heightOk;
            cylinderNamePattern.cap(1).toUInt(&idOk);
            float radius = cylinderNamePattern.cap(2).toFloat(&radiusOk);
            float height = cylinderNamePattern.cap(3).toFloat(&heightOk);
            if (!idOk) {
                errorMessage.showMessage(QString("Invalid id in '%1'").arg(QString::fromLatin1(bone->name())),
                                         "invalidName" + bone->name());
            }
            if (!radiusOk) {
                errorMessage.showMessage(QString("Invalid radius '%1'").arg(QString::fromLatin1(bone->name())),
                                         "invalidRadius" + bone->name());
            }
            if (!heightOk) {
                errorMessage.showMessage(QString("Invalid radius '%1'").arg(QString::fromLatin1(bone->name())),
                                         "invalidRadius" + bone->name());
            }

            glDisable(GL_CULL_FACE);
            gluCylinder(sphereQuadric, radius, radius, height, 15, 15);
            gluDisk(sphereQuadric, 0, radius, 15, 15);
            glTranslatef(0, 0, height);
            gluDisk(sphereQuadric, 0, radius, 15, 15);
            glEnable(GL_CULL_FACE);
        } else if (bone->name() == "#ClothBone") {

            glColor3f(1, 0, 0);
            gluSphere(sphereQuadric, 32, 10, 10);
        }

        glPopMatrix();
    }
}


Viewer::Viewer(QWidget *parent) :
    QGLWidget(parent), d(new ViewerData), mScale(1), mRotation(0), mShowNormals(false), mDrawGeometry(true),
    mDrawBones(false), mDrawClothBones(false), mShowBoneNames(false), mRenderGroundPlane(true)
{
    d->sphereQuadric = NULL;
    d->selectedBone = -1;
    d->animSkeleton = NULL;
    d->animation = NULL;
}

Viewer::~Viewer()
{
    delete d->animSkeleton;
}

void Viewer::initializeGL()
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
    lightDirection.setX(0.6324093645670703858428703903848f);
    lightDirection.setY(0.77463436252716949786709498111783f);
    glLightfv(GL_LIGHT0, GL_POSITION, lightDirection.data());

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHT0);
}

static void drawNormals(Vector4 *positions, Vector4 *normals, int vertices)
{
    glLineWidth(1.5);
    glEnable(GL_LINE_SMOOTH);
    glColor3f(0, 0, 1);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    for (int i = 0; i < vertices; i += 2) {
        const Vector4 &vertex = positions[i];
        const Vector4 &normal = normals[i];
        glVertex3fv(vertex.data());
        glVertex3fv((vertex + 15 * normal).data());
    }

    glEnd();

    glPointSize(2);
    glBegin(GL_POINTS);
    glColor3f(1, 0, 0);
    for (int i = 0; i < vertices; i += 2) {
        const Vector4 &vertex = positions[i];
        glVertex3fv(vertex.data());
    }
    glEnd();
}

void Viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    d->initializeQuadrics();
    setLighting();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);

    /*
     Draw a ground plane for reference
     */
    if (mRenderGroundPlane) {
        glColor3f(0.5, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex3f(-250, 0, -250);
        glVertex3f(250, 0, -250);
        glVertex3f(250, 0, 250);
        glVertex3f(-250, 0, 250);
        glEnd();
    }

    glEnable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glRotatef(mRotation, 0, 1, 0);
    glScalef(mScale, mScale, mScale);

    glEnable(GL_NORMALIZE); // Due to the scaling, this is necessary

    if (mDrawGeometry) {
        glBegin(GL_TRIANGLES);

        foreach (const ModelFaceGroup &group, d->faces.faceGroups()) {
            for (int i = 0; i < group.indices().size(); i += 3) {
                for (int j = 2; j >= 0; --j) {
                    ushort index = group.indices()[i + j];
                    Vector4 normal = d->geometry.normals()[index];
                    Vector4 vertex = d->geometry.positions()[index];

                    float clothWeight = 0;

                    d->doSkinning(index, &vertex, &normal, &clothWeight);

                    glColor4f(1 - clothWeight, 0, clothWeight, 1);
                    glNormal3fv(normal.data());
                    glVertex3fv(vertex.data());
                }
            }
        }

        glEnd();
    }

    if (mShowNormals) {
        drawNormals(d->geometry.positions().data(), d->geometry.normals().data(), d->geometry.positions().size());
    }

    if (mDrawBones) {
        d->drawBones(mScale);
    }

    if (mDrawClothBones) {
        d->drawClothBones();
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    if (mShowBoneNames) {
        glColor3f(1.0f, 0.5f, 0.15f);
        foreach (const Bone *bone, d->animSkeleton->bones()) {
            bool isClothBone = bone->name().startsWith("#Cylinder")
                               || bone->name().startsWith("#Sphere")
                               || bone->name().startsWith("#ClothBone");

            if (mDrawClothBones && !mDrawBones && !isClothBone) {
                continue;
            } else if (!mDrawClothBones && mDrawBones && isClothBone) {
                continue;
            }

            Vector4 pos = bone->fullWorld() * Vector4(5, 5, 0, 1);

            renderText(pos.x(), pos.y(), pos.z(), bone->name());
        }
    }

    glPopMatrix();

    /*
     Draw a reference coordinate system.
     */

    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(100, 0, 0);
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 100, 0);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 100);
    glEnd();

    glDisable(GL_CULL_FACE);

    glColor3f(1, 0, 0);
    renderText(110, 0, 0, "X");
    glColor3f(0, 1, 0);
    renderText(0, 100, 0, "Y");
    glColor3f(0, 0, 1);
    renderText(0, 0, 110, "Z");

    /*
     Draw the filename
     */

    qglColor(QColor(0, 0, 0));
    renderText(20, 20, mFilename);
    glEnable(GL_LIGHTING);
}

void Viewer::resizeGL(int width, int height)
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
    Matrix4 rotate1matrix = Matrix4::rotation(rot1);

    // Old: 90-135
    Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(135.0000005619373f));
    Matrix4 rotate2matrix = Matrix4::rotation(rot2);

    Matrix4 flipZMatrix;
    flipZMatrix.setToIdentity();
    flipZMatrix(2, 2) = -1;

    Matrix4 id;
    id.setToIdentity();
    id(2,3) = -3000;

    Matrix4 baseViewMatrix = id * flipZMatrix * rotate1matrix * rotate2matrix;

    glLoadMatrixf(baseViewMatrix.data());
}

void Viewer::open(const QString &filename)
{
    QErrorMessage errorDialog(this);

    QElapsedTimer timer;
    timer.start();

    ModelFileReader reader;

    if (!reader.open(filename)) {
        errorDialog.showMessage("Unable to open " + filename + ":\n" + reader.error());
        return;
    }

    d->animationMap.clear();
    d->animations.reset();

    QDataStream &stream = reader.stream();

    while (reader.hasNextChunk()) {
        reader.nextChunk();

        switch (reader.chunkType()) {
        case Chunk_Geometry:
            stream >> d->geometry;
            qDebug() << "Read" << d->geometry.positions().size() << "vertices";
            break;
        case Chunk_Faces:
            stream >> d->faces;
            qDebug() << "Read" << d->faces.faceGroups().size() << "face groups";
            break;
        case Chunk_Skeleton:
            stream >> d->skeleton;
            qDebug() << "Read skeleton with" << d->skeleton.bones().size() << "bones";
            break;
        case Chunk_BindingPose:
            stream >> d->bindingPose;
            qDebug() << "Read binding pose.";
            break;
        case Chunk_Animations:
            uint count;
            stream >> count;
            stream.skipRawData(3 * sizeof(uint)); // Padding

            d->animations.reset(new Animation[count]);

            for (uint j = 0; j < count; ++j) {
                Animation &animation = d->animations[j];
                stream >> animation;
                d->animationMap[animation.name()] = &animation;
            }

            qDebug() << "Read" << d->animationMap.size() << "animations";
            break;
        default:
            qDebug() << "Skipped chunk " << reader.chunkTypeName();
            break;
        }
    }

    // Invert position/normal's z coordinate. This is a relic!
    /*for (int i = 0; i < d->geometry.positions().size(); ++i) {
        d->geometry.positions()[i].setZ(- d->geometry.positions()[i].z());
        d->geometry.normals()[i].setZ(- d->geometry.normals()[i].z());
    }*/

    mFilename = filename;
    qDebug() << "Loading" << filename << "took" << timer.elapsed() << "miliseconds";

    d->selectedBone = -1;

    delete d->animSkeleton;
    d->animSkeleton = new Skeleton(d->skeleton);
    playAnimation("unarmed_unarmed_idle");
    if (!d->animation)
        playAnimation("item_idle");

    repaint();
}

const Skeleton *Viewer::skeleton() const
{
    return &d->skeleton;
}

void Viewer::selectBone(int bone)
{
    d->selectedBone = bone;
    repaint();
}

void Viewer::nextFrame()
{
    if (!d->animation)
        return;

    if (d->frame > d->animation->frames() - 1) {
        d->frame = 0;
    }

    /*
     Update skeleton.
     */
    foreach (Bone *bone, d->animSkeleton->bones()) {
        const AnimationBone *animBone = d->animation->animationBones().value(bone->boneId(), NULL);

        if (animBone) {
            bone->setRelativeWorld(animBone->getTransform(d->frame, d->animation->frames()));
        }

        if (bone->parent()) {
            bone->setFullWorld(bone->parent()->fullWorld() * bone->relativeWorld());
        } else {
            bone->setFullWorld(bone->relativeWorld());
        }
    }


    d->frame++;

    repaint();
}

void Viewer::playAnimation(const QString &name)
{
    qDebug("Trying to play animation %s.", qPrintable(name));

    d->animation = d->animationMap.value(name.toLatin1(), NULL);
    d->frame = 0;
}

QList<AnimationInfo> Viewer::animations() const
{
    QList<AnimationInfo> result;

    for (int i = 0; i < d->animationMap.size(); ++i) {
        const Animation &anim = d->animations[i];

        AnimationInfo info;
        info.name = anim.name();
        info.dps = anim.dps();
        switch (anim.driveType()) {
        case Animation::Time:
            info.driveType = "time";
            break;
        case Animation::Rotation:
            info.driveType = "rotation";
            break;
        case Animation::Distance:
            info.driveType = "distance";
            break;
        default:
            info.driveType = "unknown";
            break;
        }

        info.fps = anim.frameRate();
        info.frames = anim.frames();
        info.affectedBones = anim.animationBones().size();

        foreach (const AnimationEvent &event, anim.events()) {
            AnimEventInfo eventInfo;

            if (event.type() == AnimationEvent::Script)
                eventInfo.type = "script";
            else if (event.type() == AnimationEvent::Action)
                eventInfo.type = "action";
            else
                eventInfo.type = "unknown";

            eventInfo.content = event.content();
            info.events.append(eventInfo);
        }

        result.append(info);
    }

    return result;
}
