#include "pch.h"
#include "MeshComponent.h"

MeshComponent::MeshComponent(std::shared_ptr<Mesh> mesh)
        : mMesh(mesh), mMaterialSlots(mesh->getMaterials()) {}
