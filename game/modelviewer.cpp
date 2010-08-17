
#include <QPointer>
#include <QElapsedTimer>

#include "modelviewer.h"
#include "modelinstance.h"
#include "modelfile.h"

namespace EvilTemple {

class ModelViewerData : public AlignedAllocation {
public:
    ModelViewerData();
    ~ModelViewerData();

    void updateBuffers();

    bool mBuffersInvalid;

    RenderStates mRenderStates;
    Materials mMaterials;
    Scene mScene;
    SceneNode *mSceneNode;
    QPointer<ModelInstance> mModelInstance;

    // Off-screen rendering of character.
    GLuint mFbo, mRboDepthStencil, mTexture;
    int bufferWidth, bufferHeight;
    bool mFboUsed;
    QElapsedTimer mAnimationTimer;

    Matrix4 mBaseViewMatrix;
};

ModelViewerData::ModelViewerData()
    : mMaterials(mRenderStates), mScene(&mMaterials), mBuffersInvalid(true), mFboUsed(true)
{
    // Generate a texture to hold the color-result of the rendering process.
    glGenTextures(1, &mTexture);

    // Model materials touch the stencil and depth buffer, so we create an off-screen buffer for both
    glGenRenderbuffersEXT(1, &mRboDepthStencil);

    // Create the FBO that will piece together the two buffers above
    glGenFramebuffersEXT(1, &mFbo);

    // Old: -44
    Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, deg2rad(-44.42700648682643));
    Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

    // Old: 90-135
    Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(135.0000005619373));
    Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

    Matrix4 flipZMatrix;
    flipZMatrix.setToIdentity();
    flipZMatrix(2, 2) = -1;

    Matrix4 id;
    id.setToIdentity();
    id(2,3) = -3000;

    mBaseViewMatrix = id * flipZMatrix * rotate1matrix * rotate2matrix;

    /*
     Set up lighting. Use the default directional lighting and nothing else.
     */
    Light *light = new Light;
    light->setColor(Vector4(1, 1, 1, 1));
    light->setDirection(Vector4(-0.6324094, -0.7746344, 0, 0).normalized());
    light->setRange(10000000000);
    light->setType(Light::Directional);

    /*QSharedPointer<Model> model(new Model);
    if (!model->load("meshes/PCs/PC_Human_Male/PC_Human_Male.model", &mMaterials, mRenderStates)) {
        qWarning("Unable to load mirror model.");
    }*/

    ModelInstance *modelinstance = new ModelInstance;
    /*modelinstance->setModel(model);

    // Load default materials
    SharedMaterialState headMaterial(new MaterialState);
    headMaterial->createFromFile("meshes/PCs/PC_Human_Male/head.xml", mRenderStates);
    modelinstance->overrideMaterial("HEAD", headMaterial);

    SharedMaterialState feetMaterial(new MaterialState);
    feetMaterial->createFromFile("meshes/PCs/PC_Human_Male/feet.xml", mRenderStates);
    modelinstance->overrideMaterial("BOOTS", feetMaterial);

    SharedMaterialState handsMaterial(new MaterialState);
    handsMaterial->createFromFile("meshes/PCs/PC_Human_Male/hands.xml", mRenderStates);
    modelinstance->overrideMaterial("GLOVES", handsMaterial);

    SharedMaterialState chestMaterial(new MaterialState);
    chestMaterial->createFromFile("meshes/PCs/PC_Human_Male/chest.xml", mRenderStates);
    modelinstance->overrideMaterial("CHEST", chestMaterial);*/

    mModelInstance = modelinstance;

    mAnimationTimer.start();

    mSceneNode = mScene.createNode();
    mSceneNode->setPosition(Vector4(0, 0, 0, 1));
    mSceneNode->setRotation(Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(-125)));
    mSceneNode->attachObject(light);
    mSceneNode->attachObject(modelinstance);
}

ModelViewerData::~ModelViewerData()
{
    glDeleteFramebuffersEXT(1, &mFbo);

    glDeleteTextures(1, &mTexture);

    glDeleteRenderbuffersEXT(1, &mRboDepthStencil);
}

void ModelViewerData::updateBuffers()
{
    qDebug("Creating FBO with dimensions: %d,%d", bufferWidth, bufferHeight);

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bufferWidth, bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRboDepthStencil);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT, bufferWidth, bufferHeight);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

    /**
      Since the dimensions of the buffers changed, we have to piece the FBO together from scratch.
      */
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mTexture, 0);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRboDepthStencil);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRboDepthStencil);

    // check FBO status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        qWarning("Unable to use FBO's to render paperdolls: %d", status);
        mFboUsed = false;
    } else {
        mFboUsed = true;
    }

    // switch back to window-system-provided framebuffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    mBuffersInvalid = false;
}

ModelViewer::ModelViewer() : mModelScale(1), mModelRotation(0), d(new ModelViewerData)
{
    setFlag(ItemHasNoContents, false);
}

ModelViewer::~ModelViewer()
{
}

void ModelViewer::updateBuffers()
{
    d->bufferWidth = ceil(width());
    d->bufferHeight = ceil(height());

    d->updateBuffers();
}

void ModelViewer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (d->mBuffersInvalid) {
        updateBuffers();
    }

    painter->drawText(0, 20, QString("Scale: %1").arg(mModelScale));

    // Update the scene node's rotation and scale
    d->mSceneNode->setRotation(Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(mModelRotation)));
    d->mSceneNode->setScale(Vector4(mModelScale, mModelScale, mModelScale, mModelScale));

    // Animate the model instance
    if (d->mModelInstance) {
        if (d->mAnimationTimer.elapsed() > 0) {
            float elapsed = d->mAnimationTimer.restart() / 1000.0f;
            d->mModelInstance->elapseTime(elapsed);
        }
    }

    /**
      This is an attempt to save as much GL state as possible, to not upset the QML engine.
      */
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, d->mFbo);

    SAFE_GL(glEnable(GL_DEPTH_TEST));
    SAFE_GL(glEnable(GL_CULL_FACE));
    SAFE_GL(glEnable(GL_BLEND));
    SAFE_GL(glDisable(GL_STENCIL_TEST));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0, 0, 0, 0);
    glClearStencil(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    float halfWidth = width() * 0.5f;
    float halfHeight = height() * 0.5f;
    glViewport(0, 0, width(), height());

    float zoom = 1.5;

    Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth / zoom, halfWidth / zoom,
                                              -halfHeight / zoom, halfHeight / zoom, 1, 5000);
    d->mRenderStates.setProjectionMatrix(projectionMatrix);

    // Move down the origin of the scene's coordinate system to 1/3 the height
    float moveDown = height() / 3 - halfHeight;

    Matrix4 viewMatrix = Matrix4::translation(0, moveDown) * d->mBaseViewMatrix;

    d->mRenderStates.setViewMatrix(viewMatrix);

    glUseProgram(0);
    d->mScene.render(d->mRenderStates);

    // Read texture (easiest at this point, should probably be changed in the future)
    QImage image(d->bufferWidth, d->bufferHeight, QImage::Format_ARGB32);

    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadPixels(0, 0, d->bufferWidth, d->bufferHeight, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
    image = image.mirrored();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glPopAttrib();
    glPopClientAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    painter->drawImage(0, 0, image);
}

void ModelViewer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);

    d->mBuffersInvalid = true;
}

ModelInstance *ModelViewer::modelInstance() const
{
    return d->mModelInstance;
}

Materials *ModelViewer::materials() const
{
    return &d->mMaterials;
}

}
