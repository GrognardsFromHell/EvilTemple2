#include "modelinstance.h"
#include "util.h"
#include "drawhelper.h"
#include "particlesystem.h"
#include "scenenode.h"

namespace EvilTemple {

    ModelInstance::ModelInstance()
        : mPositionBuffer(QGLBuffer::VertexBuffer), mNormalBuffer(QGLBuffer::VertexBuffer),
		mCurrentAnimation(NULL), mPartialFrameTime(0), mCurrentFrame(0),
		mTransformedPositions(NULL), mTransformedNormals(NULL), mFullTransform(NULL),
		mFullWorld(NULL)
    {
		mPositionBuffer.setUsagePattern(QGLBuffer::StreamDraw);
		mNormalBuffer.setUsagePattern(QGLBuffer::StreamDraw);
    }

	ModelInstance::~ModelInstance()
	{
		delete [] mTransformedPositions;
		delete [] mTransformedNormals;
		delete [] mFullWorld;
		delete [] mFullTransform;
	}

    void ModelInstance::setModel(const SharedModel &model)
    {
		delete [] mTransformedPositions;
		mTransformedPositions = 0;
		delete [] mTransformedNormals;
		mTransformedNormals = 0;
		delete [] mFullWorld;
		mFullWorld = 0;
		delete [] mFullTransform;
		mFullTransform = 0;

        mModel = model;

		mCurrentAnimation = model->animation("item_idle");

		if (!mCurrentAnimation) {
			mCurrentAnimation = model->animation("unarmed_unarmed_idle");
		}

		if (mCurrentAnimation) {
			mTransformedPositions = new Vector4[mModel->vertices];
			mTransformedNormals = new Vector4[mModel->vertices];

			mPositionBuffer.create();
			mPositionBuffer.bind();
			mPositionBuffer.allocate(model->positions, sizeof(Vector4) * model->vertices);

			mNormalBuffer.create();
			mNormalBuffer.bind();
			mNormalBuffer.allocate(model->normals, sizeof(Vector4) * model->vertices);

			mFullWorld = new Matrix4[mModel->bones().size()];
			mFullTransform = new Matrix4[mModel->bones().size()];
		}
    }

    void ModelInstance::addMesh(const SharedModel &model)
    {
        mAddMeshes.append(model);
    }

    Matrix4 ModelInstance::getBoneSpace(const QString &boneName) const
    {
        if (!mModel)
            return Matrix4::identity();

        // Search for the bone id
        // TODO: We need a QHash that maps bone names to their id here
        for (int i = 0; i < mModel->bones().size(); ++i) {
            if (mModel->bones()[i].name() == boneName) {
                return mFullWorld[i];
            }
        }

        qWarning("Unknown bone name: %s.", qPrintable(boneName));
        return Matrix4::identity();
    }

    struct ModelDrawStrategy : public DrawStrategy {
        ModelDrawStrategy(GLint bufferId, int elementCount)
            : mBufferId(bufferId), mElementCount(elementCount)
        {
        }

        inline void draw(const RenderStates &renderStates, MaterialPassState &state) const
        {
            Q_UNUSED(state);
            Q_UNUSED(renderStates);

            // Render once without diffuse/specular, then render again without ambient
            int typePos = state.program.uniformLocation("lightSource.type");
            if (typePos != -1) {
                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

                typePos = state.program.uniformLocation("lightSource.type");
                int colorPos = state.program.uniformLocation("lightSource.color");
                int positionPos = state.program.uniformLocation("lightSource.position");
                int attenuationPos = state.program.uniformLocation("lightSource.attenuation");

                SAFE_GL(glUniform1i(typePos, 1));
                //SAFE_GL(glUniform4f(colorPos, 0.662745f, 0.564706f, 0.905882f, 0));
                SAFE_GL(glUniform4f(colorPos, 0.962745f, 0.964706f, 0.965882f, 0));
                SAFE_GL(glUniform4f(positionPos, -0.632409f, -0.774634f, 0, 0));              
                
                SAFE_GL(glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, 0));

                SAFE_GL(glDepthFunc(GL_LEQUAL));
                SAFE_GL(glEnable(GL_CULL_FACE));

                if (renderStates.activeLights().size() > 0) {
                    SAFE_GL(glEnable(GL_BLEND));
                    SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));

                    // Draw again for every light affecting this mesh
                    foreach (const Light &light, renderStates.activeLights()) {
                        SAFE_GL(glUniform1i(typePos, light.type()));
                        SAFE_GL(glUniform4f(colorPos, light.color().x(), light.color().y(), light.color().z(), light.color().w()));
                        SAFE_GL(glUniform4f(positionPos, light.position().x(), light.position().y(), light.position().z(), 1));
                        SAFE_GL(glUniform1f(attenuationPos, light.attenuation()));

                        SAFE_GL(glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, 0));
                    }

                    SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                }
                
                SAFE_GL(glDepthFunc(GL_LESS));

                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            } else {
                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));
                SAFE_GL(glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, 0));
                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            }
        }

        GLint mBufferId;
        int mElementCount;
    };

    void ModelInstance::drawNormals() const
    {
        if (!mModel)
            return;

        if (!mCurrentAnimation) {
            mModel->drawNormals();
            return;
        }

        SAFE_GL(glLineWidth(1.5));
        SAFE_GL(glEnable(GL_LINE_SMOOTH));
        SAFE_GL(glColor3f(0, 0, 1));
        SAFE_GL(glDisable(GL_LIGHTING));
        glBegin(GL_LINES);

        for (int i = 0; i < mModel->vertices; i += 2) {
            const Vector4 &vertex = mTransformedPositions[i];
            const Vector4 &normal = mTransformedNormals[i];
            glVertex3fv(vertex.data());
            glVertex3fv((vertex + 15 * normal).data());
        }
        glEnd();

        glPointSize(2);
        glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        for (int i = 0; i < mModel->vertices; i += 2) {
            const Vector4 &vertex = mTransformedPositions[i];
            glVertex3fv(vertex.data());
        }
        SAFE_GL(glEnd());
    }

    struct ModelBufferSource : public BufferSource {
        inline ModelBufferSource(GLint positionBuffer, GLint normalBuffer, GLint texCoordBuffer)
            : mPositionBuffer(positionBuffer), mNormalBuffer(normalBuffer), mTexCoordBuffer(texCoordBuffer)
        {
        }

        inline GLint buffer(const MaterialPassAttributeState &attribute) const
        {
            switch (attribute.bufferType)
            {
            case 0:
                return mPositionBuffer;
            case 1:
                return mNormalBuffer;
            case 2:
                return mTexCoordBuffer;
            default:
                qWarning("Unknown buffer id requested: %d.", attribute.bufferType);
            }
        }

        GLint mPositionBuffer;
        GLint mNormalBuffer;
        GLint mTexCoordBuffer;
    };

    void ModelInstance::render(RenderStates &renderStates)
    {
        const Model *model = mModel.data();

        if (!model)
            return;

        DrawHelper<ModelDrawStrategy, ModelBufferSource> drawHelper;
        ModelBufferSource bufferSource(mCurrentAnimation ? mPositionBuffer.bufferId() : model->positionBuffer,
            mCurrentAnimation ? mNormalBuffer.bufferId() : model->normalBuffer,
            model->texcoordBuffer);

        for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
            const FaceGroup &faceGroup = model->faceGroups[faceGroupId];
            MaterialState *material = faceGroup.material;

            ModelDrawStrategy drawStrategy(faceGroup.buffer, faceGroup.elementCount);
            
            drawHelper.draw(renderStates, material, drawStrategy, bufferSource);
        }
    }

    void ModelInstance::draw(const RenderStates &renderStates, MaterialState *overrideMaterial) const
    {
        Q_ASSERT(overrideMaterial);

        const Model *model = mModel.data();

        if (!model)
            return;

        for (int i = 0; i < overrideMaterial->passCount; ++i) {
            MaterialPassState &pass = overrideMaterial->passes[i];

            pass.program.bind();

            // Bind texture samplers
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                pass.textureSamplers[j].bind();
            }

            // Bind uniforms
            for (int j = 0; j < pass.uniforms.size(); ++j) {
                pass.uniforms[j].bind();
            }

            // Bind attributes
            for (int j = 0; j < pass.attributes.size(); ++j) {
                MaterialPassAttributeState &attribute = pass.attributes[j];

                // Bind the correct buffer
                switch (attribute.bufferType) {
                case 0:
                    if (!mCurrentAnimation) {
                        SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model->positionBuffer));
                    } else {
                        SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer.bufferId()));
                    }
                    break;
                case 1:
                    if (!mCurrentAnimation) {
                        SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model->normalBuffer));
                    } else {
                        SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer.bufferId()));
                    }
                    break;
                case 2:
                    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, model->texcoordBuffer));
                    break;
                }

                // Assign the attribute
                SAFE_GL(glEnableVertexAttribArray(attribute.location));
                SAFE_GL(glVertexAttribPointer(attribute.location,
                                              attribute.binding.components(),
                                              attribute.binding.type(),
                                              attribute.binding.normalized(),
                                              attribute.binding.stride(),
                                              (GLvoid*)attribute.binding.offset()));

            }
            SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0)); // Unbind any previously bound buffers

            // Set render states
            foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                state->enable();
            }

            for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
                const FaceGroup &faceGroup = model->faceGroups[faceGroupId];

                // Draw the actual model
                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceGroup.buffer));
                SAFE_GL(glDrawElements(GL_TRIANGLES, faceGroup.elementCount, GL_UNSIGNED_SHORT, 0));
            }

            SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

            // Reset render states to default
            foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                state->disable();
            }

            // Unbind textures
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                pass.textureSamplers[j].unbind();
            }

            // Unbind attributes
            for (int j = 0; j < pass.attributes.size(); ++j) {
                MaterialPassAttributeState &attribute = pass.attributes[j];
                SAFE_GL(glDisableVertexAttribArray(attribute.location));
            }

            pass.program.unbind();
        }
    }

	double updateBones = 0;
	double updateVertices = 0;

    void ModelInstance::elapseTime(float elapsedSeconds)
    {
		if (!mModel || !mCurrentAnimation)
			return;

		mPartialFrameTime += elapsedSeconds;
		
		float timePerFrame = 1 / mCurrentAnimation->frameRate();

		if (mPartialFrameTime < timePerFrame)
			return;

		while (mPartialFrameTime >= timePerFrame) {
			mCurrentFrame++;

			if (mCurrentFrame >= mCurrentAnimation->frames()) {
				mCurrentFrame = 0;
			}
			mPartialFrameTime -= timePerFrame;
		}

		LARGE_INTEGER freq, start, end;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);

		const QVector<Bone> &bones = mModel->bones();

		// Create a bone state map for all bones
		const Animation::BoneMap &animationBones = mCurrentAnimation->animationBones();

		for (int i = 0; i < bones.size(); ++i) {
			const Bone &bone = bones[i];
						
			Matrix4 relativeWorld;

			if (animationBones.contains(i)) {
				const AnimationBone *animationBone = animationBones[i];
				
				relativeWorld = animationBone->getTransform(mCurrentFrame, mCurrentAnimation->frames());
			} else {
				relativeWorld = bone.relativeWorld();
			}

			// Use relative world and fullWorld of parent to build this bone's full world
			const Bone *parent = bone.parent();

			if (parent) {
				Q_ASSERT(parent->boneId() >= 0 && parent->boneId() < bone.boneId());
				mFullWorld[i] = mFullWorld[parent->boneId()] * relativeWorld;
			} else {
				mFullWorld[i] = relativeWorld;
			}

			mFullTransform[i] = mFullWorld[i] * bone.fullWorldInverse();
		}

		QueryPerformanceCounter(&end);

		updateBones += (end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

		QueryPerformanceCounter(&start);

		for (int i = 0; i < mModel->vertices; ++i) {
			if (mModel->attachments[i].count() == 0)
				continue;

            Vector4 result(0, 0, 0, 0);
            Vector4 resultNormal(0, 0, 0, 0);

			const BoneAttachment &attachment = mModel->attachments[i];

			for (int k = 0; k < attachment.count(); ++k) {
                float weight = attachment.weights()[k];
				uint boneId = attachment.bones()[k];
				Q_ASSERT(boneId >= 0 && boneId <= mModel->bones().size());

				const Matrix4 &fullTransform = mFullTransform[boneId];

                result += weight * fullTransform.mapPosition(mModel->positions[i]);
                resultNormal += weight * fullTransform.mapNormal(mModel->normals[i]);
            }

            resultNormal.data()[2] *= -1;
            result.data()[2] *= -1;
            result.data()[3] =  1;

			mTransformedPositions[i] = result;
			mTransformedNormals[i] = resultNormal;
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer.bufferId());
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * mModel->vertices, mTransformedPositions, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer.bufferId());
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * mModel->vertices, mTransformedNormals, GL_STREAM_DRAW);
		
		QueryPerformanceCounter(&end);

		updateVertices += (end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
    }
    
    const Box3d &ModelInstance::boundingBox()
    {
        return mModel->boundingBox();
    }
    
    const Matrix4 &ModelInstance::worldTransform() const
    {
        Q_ASSERT(mParentNode);

        return mParentNode->fullTransform();
    }

}
