#pragma once
#include "pch.h"

#include "Scene/Component.h"
#include "Scene/Mesh/Mesh.h"

class MeshComponent : public Component {
public:
    explicit MeshComponent(std::shared_ptr<Mesh> mesh);

    MeshComponent(const MeshComponent&) = delete;

    MeshComponent& operator=(const MeshComponent&) = delete;

    inline const Mesh& getMesh() const {
        return *mMesh;
    }

    inline Mesh& getMesh() {
        return *mMesh;
    }

    inline void setMesh(const std::shared_ptr<Mesh>& mesh) {
        mMesh = mesh;
    }

    inline std::vector<std::shared_ptr<Material>>& getMaterialSlots() {
        return mMaterialSlots;
    }

    inline const std::vector<std::shared_ptr<Material>>& getMaterialSlots() const {
        return mMaterialSlots;
    }

private:
    std::shared_ptr<Mesh> mMesh;

    std::vector<std::shared_ptr<Material>> mMaterialSlots;
};