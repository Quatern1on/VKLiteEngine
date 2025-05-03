#include "pch.h"
#include "SceneObject.h"

SceneObject::SceneObject() {
    std::unique_ptr<TransformComponent> transformComponent = std::make_unique<TransformComponent>();
    mTransform = &*transformComponent;
    addComponent(std::move(transformComponent));
}

void SceneObject::attachToParent(SceneObject* parent) {
    mParent = parent;
}

void SceneObject::addChild(std::unique_ptr<SceneObject> child) {
    child->attachToParent(this);
    mChildren.emplace_back(std::move(child));
}

void SceneObject::removeChild(std::size_t index) {
    mChildren[index]->attachToParent(nullptr);
    mChildren.erase(mChildren.begin() + index);
}

void SceneObject::addComponent(std::unique_ptr<Component> component) {
    component->attachToSceneObject(this);
    mComponents.emplace_back(std::move(component));
}

void SceneObject::removeComponent(std::size_t index) {
    if (index == 0) {
        throw std::runtime_error("Could not remove transform component");
    }

    mComponents[index]->attachToSceneObject(nullptr);
    mComponents.erase(mComponents.begin() + index);
}

void SceneObject::updateTransforms() {
    mTransform->update();

    for (auto& child : mChildren) {
        child->updateTransforms();
    }
}
