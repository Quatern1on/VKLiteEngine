#pragma once
#include "pch.h"

class SceneObject;

class Component {
public:
    explicit Component() = default;

    Component(const Component&) = delete;

    Component& operator=(const Component&) = delete;

    virtual ~Component() = default;

    void virtual attachToSceneObject(SceneObject* sceneObject);

    inline const SceneObject* getSceneObject() const {
        return mSceneObject;
    }

    inline SceneObject* getSceneObject() {
        return mSceneObject;
    }

private:
    SceneObject* mSceneObject = nullptr;
};
