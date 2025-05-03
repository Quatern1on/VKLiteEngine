#include "pch.h"
#include "TransformComponent.h"

#include "Scene/SceneObject.h"

void TransformComponent::update() {
    mTransformMatrix = glm::translate(glm::mat4(1.0f), mPosition);
    mTransformMatrix = mTransformMatrix * glm::toMat4(mRotation);
    mTransformMatrix = glm::scale(mTransformMatrix, mScale);

    mInverseTransformMatrix = glm::inverse(mTransformMatrix);

    mLocalToWorldMatrix = getTransformMatrix();

    SceneObject* parentObject = getSceneObject()->getParent();

    if (parentObject) {
        mLocalToWorldMatrix = parentObject->getTransform().getLocalToWorldMatrix() * mLocalToWorldMatrix;
        mWorldToLocalMatrix = glm::inverse(mLocalToWorldMatrix);
    } else {
        mWorldToLocalMatrix = getInverseTransformMatrix();
    }
}
