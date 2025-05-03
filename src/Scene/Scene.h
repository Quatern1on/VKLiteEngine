#pragma once
#include "pch.h"

#include "Scene/SceneObject.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Camera/CameraComponent.h"

class Scene {
public:
    explicit Scene();

    Scene(const Scene&) = delete;

    Scene& operator=(const Scene&) = delete;

    inline const SceneObject& getRoot() const {
        return *mRoot;
    }

    inline SceneObject& getRoot() {
        return *mRoot;
    }

    inline const CameraComponent* getCamera() const {
        return mCamera;
    }

    inline CameraComponent* getCamera() {
        return mCamera;
    }

    inline void setCamera(CameraComponent* camera) {
        mCamera = camera;
    }

private:
    std::unique_ptr<SceneObject> mRoot;

    CameraComponent* mCamera;
};
