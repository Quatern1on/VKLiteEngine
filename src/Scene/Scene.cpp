#include "pch.h"
#include "Scene.h"

Scene::Scene()
        : mCamera(nullptr) {
    mRoot = std::make_unique<SceneObject>();
}
