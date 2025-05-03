#pragma once

#include <cstdint>
#include <string>
#include <limits>
#include <sstream>
#include <array>
#include <vector>
#include <deque>
#include <set>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <cassert>
#include <functional>
#include <numbers>

//glog includes windows.h which creates min/max macro,
//that collides with std::numeric_limits<T>::min()/max()
#define NOMINMAX

#include <glog/logging.h>

#undef near
#undef far

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vk_enum_string_helper.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Core/Assert.h"
#include "Core/Utils.h"
#include "Assets/AssetDescriptor.h"
#include "Assets/AssetFactory.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanUtils.h"
#include "Core/Engine.h"