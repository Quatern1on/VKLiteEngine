#pragma once
#include "pch.h"

#include "Scene/Component.h"
#include "Scene/TransformComponent.h"

class SceneObject {
public:
    explicit SceneObject();

    SceneObject(const SceneObject&) = delete;

    SceneObject& operator=(const SceneObject&) = delete;

    void attachToParent(SceneObject* parent);

    void addChild(std::unique_ptr<SceneObject> child);

    void removeChild(std::size_t index);

    inline const std::vector<std::unique_ptr<SceneObject>>& getChildren() const {
        return mChildren;
    }

    inline SceneObject* getParent() const {
        return mParent;
    }

    void addComponent(std::unique_ptr<Component> component);

    void removeComponent(std::size_t index);

    inline const std::vector<std::unique_ptr<Component>>& getComponents() const {
        return mComponents;
    }

    template<typename T>
    T* getComponent() const {
        for (auto& component : mComponents) {
            T* typedComponent = dynamic_cast<T*>(&*component);
            if (typedComponent != nullptr) {
                return typedComponent;
            }
        }
        return nullptr;
    }

    template<typename T>
    std::vector<std::reference_wrapper<T>> getComponentsRecursive() const {
        std::vector<std::reference_wrapper<T>> typedComponents;

        for (auto& component : mComponents) {
            T* typedComponent = dynamic_cast<T*>(&*component);
            if (typedComponent != nullptr) {
                typedComponents.push_back(std::ref(*typedComponent));
            }
        }

        for (auto& child : mChildren) {
            std::vector<std::reference_wrapper<T>> childrenComponents = child->getComponentsRecursive<T>();
            typedComponents.insert(typedComponents.end(), childrenComponents.begin(), childrenComponents.end());
        }

        return typedComponents;
    }

    inline const TransformComponent& getTransform() const {
        return *mTransform;
    }

    inline TransformComponent& getTransform() {
        return *mTransform;
    }

    void updateTransforms();

private:
    SceneObject* mParent = nullptr;

    std::vector<std::unique_ptr<SceneObject>> mChildren;

    std::vector<std::unique_ptr<Component>> mComponents;

    TransformComponent* mTransform;
};
