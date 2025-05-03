#pragma once
#include "pch.h"

#include "Scene/Mesh/Mesh.h"

namespace MeshLoader {
    std::shared_ptr<Mesh> loadAsSingleMesh(std::unique_ptr<AssetDescriptor> modelAsset);
};
