#pragma once
#include "pch.h"

#include "Scene/Component.h"

class TransformComponent : public Component {
public:
    explicit TransformComponent() = default;

    TransformComponent(const TransformComponent&) = delete;

    TransformComponent& operator=(const TransformComponent&) = delete;

    void update();

    inline glm::mat4 getLocalToWorldMatrix() const {
        return mLocalToWorldMatrix;
    }

    inline glm::mat4 getWorldToLocalMatrix() const {
        return mWorldToLocalMatrix;
    }

    inline glm::vec3 getPosition() const {
        return mPosition;
    }

    inline void setPosition(glm::vec3 position) {
        mPosition = position;
    }

    inline void translate(glm::vec3 translation) {
        mPosition += translation;
    }

    inline glm::vec3 getScale() const {
        return mScale;
    }

    inline void setScale(glm::vec3 scale) {
        mScale = scale;
    }

    inline glm::quat getRotation() const {
        return mRotation;
    }

    inline void setRotation(glm::quat rotation) {
        mRotation = rotation;
    }

    inline void rotate(float angle, glm::vec3 axis) {
        mRotation = glm::rotate(mRotation, angle, axis);
    }

    inline glm::mat4 getTransformMatrix() const {
        return mTransformMatrix;
    }

    inline glm::mat4 getInverseTransformMatrix() const {
        return mInverseTransformMatrix;
    }

    inline glm::vec3 forward() const {
        return glm::rotate(mRotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    inline glm::vec3 up() const {
        return glm::rotate(mRotation, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    inline glm::vec3 right() const {
        return glm::rotate(mRotation, glm::vec3(1.0f, 0.0f, 0.0f));
    }

private:
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mScale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::quat mRotation = glm::quat(1.0, 0.0, 0.0, 0.0);

    glm::mat4 mTransformMatrix = glm::mat4(1.0f);
    glm::mat4 mInverseTransformMatrix = glm::mat4(1.0f);

    glm::mat4 mLocalToWorldMatrix = glm::mat4(1.0f);
    glm::mat4 mWorldToLocalMatrix = glm::mat4(1.0f);
};
