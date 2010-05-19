#include "modelinstance.h"
#include "util.h"

namespace EvilTemple {

    ModelInstance::ModelInstance()
        : mPositionBuffer(QGLBuffer::VertexBuffer), mNormalBuffer(QGLBuffer::VertexBuffer),
		mCurrentAnimation(NULL), mPartialFrameTime(0), mCurrentFrame(0)
    {
		mPositionBuffer.setUsagePattern(QGLBuffer::StreamDraw);
		mNormalBuffer.setUsagePattern(QGLBuffer::StreamDraw);
    }

    void ModelInstance::setModel(const SharedModel &model)
    {
        mModel = model;

		mCurrentAnimation = model->animation("item_idle");

		if (mCurrentAnimation && mModel) {
			mPositionBuffer.create();
			mPositionBuffer.bind();
			mPositionBuffer.allocate(model->positions, sizeof(Vector4) * model->vertices);

			mNormalBuffer.create();
			mNormalBuffer.bind();
			mNormalBuffer.allocate(model->normals, sizeof(Vector4) * model->vertices);

			mBoneStates.resize(mModel->bones().size());
		}
    }

    void ModelInstance::draw() const
    {
        const Model *model = mModel.data();

        if (!model)
            return;

        for (int faceGroupId = 0; faceGroupId < model->faces; ++faceGroupId) {
            const FaceGroup &faceGroup = model->faceGroups[faceGroupId];
            MaterialState *material = faceGroup.material;

            if (!material)
                continue;

            for (int i = 0; i < material->passCount; ++i) {
                MaterialPassState &pass = material->passes[i];

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

                // Draw the actual model
                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceGroup.buffer));
                SAFE_GL(glDrawElements(GL_TRIANGLES, faceGroup.elementCount, GL_UNSIGNED_SHORT, 0));
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
    }

    void ModelInstance::draw(MaterialState *overrideMaterial) const
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



    void ModelInstance::elapseTime(float elapsedSeconds)
    {
		if (!mModel || !mCurrentAnimation)
			return;

		mPartialFrameTime += elapsedSeconds;

		if (mPartialFrameTime > 1 / mCurrentAnimation->frameRate()) {
			mCurrentFrame++;

			if (mCurrentFrame > mCurrentAnimation->frames()) {
				mCurrentFrame = 0;
			}
			mPartialFrameTime = 0;
		} else {
			return;
		}

		const QVector<Bone> &bones = mModel->bones();

		// Create a bone state map for all bones
		const QMap<uint, AnimationBone> &animationBones = mCurrentAnimation->animationBones();

		for (int i = 0; i < mBoneStates.size(); ++i) {
			BoneState &boneState = mBoneStates[i];
			const Bone &bone = bones[i];
						
			Matrix4 relativeWorld;

			if (animationBones.contains(i)) {
				const AnimationBone &animationBone = animationBones[i];

				int frame = mCurrentFrame;
				while (!animationBone.rotationStream().contains(frame)) {
					frame--;
				}
				Quaternion rotation = animationBone.rotationStream()[frame];

				frame = mCurrentFrame;
				while (!animationBone.scaleStream().contains(frame)) {
					frame--;
				}
				Vector4 scale = animationBone.scaleStream()[frame];

				frame = mCurrentFrame;
				while (!animationBone.translationStream().contains(frame)) {
					frame--;
				}
				Vector4 translation = animationBone.translationStream()[frame];

				relativeWorld = Matrix4::transformation(scale, rotation, translation);
			} else {
				relativeWorld = bone.relativeWorld();
			}

			// Use relative world and fullWorld of parent to build this bone's full world
			const Bone *parent = bone.parent();

			if (parent) {
				Q_ASSERT(parent->boneId() >= 0 && parent->boneId() < bone.boneId());
				boneState.fullWorld = mBoneStates[parent->boneId()].fullWorld * relativeWorld;
			} else {
				boneState.fullWorld = relativeWorld;
			}

			boneState.fullTransform = boneState.fullWorld * bone.fullWorldInverse();
		}

		// Now transform positions + normals
		if (!mPositionBuffer.bind())
			qWarning("Unable to bind position buffer before update.");
		Vector4 *transformedPositions = reinterpret_cast<Vector4*>(mPositionBuffer.map(QGLBuffer::WriteOnly));
		if (!transformedPositions)
			qWarning("Unable to map the position buffer for updating.");

		if (!mNormalBuffer.bind())
			qWarning("Unable to bind normal buffer before update.");
		Vector4 *transformedNormals = reinterpret_cast<Vector4*>(mNormalBuffer.map(QGLBuffer::WriteOnly));
		if (!transformedNormals)
			qWarning("Unable to map the normal buffer for updating");

		for (int i = 0; i < mModel->vertices; ++i) {
			if (mModel->attachments[i].count() == 0)
				continue;

            Vector4 result(0, 0, 0, 0);
            Vector4 resultNormal(0, 0, 0, 0);

            for (int k = 0; k < mModel->attachments[i].count(); ++k) {
                float weight = mModel->attachments[i].weights()[k];
				uint boneId = mModel->attachments[i].bones()[k];
				Q_ASSERT(boneId >= 0 && boneId <= boneStates.size());
				const BoneState &bone = mBoneStates[boneId];

				Vector4 transformedPos = bone.fullTransform * mModel->positions[i];
                transformedPos *= 1 / transformedPos.w();

                result += weight * transformedPos;
                resultNormal += weight * (bone.fullTransform * mModel->normals[i]);
            }

            result.data()[2] *= -1;
            resultNormal.data()[2] *= -1;

            result.data()[3] =  1;

			transformedPositions[i] = result;
			transformedNormals[i] = result;
		}
				
		if (!mNormalBuffer.unmap())
			qWarning("Unmapping normal buffer after update failed.");
		if (!mPositionBuffer.bind())
			qWarning("Binding position buffer after update failed.");
		if (!mPositionBuffer.unmap())
			qWarning("Unmapping position buffer after update failed.");
    }

}
