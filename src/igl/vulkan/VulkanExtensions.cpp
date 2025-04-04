/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <igl/vulkan/VulkanExtensions.h>

#include <igl/vulkan/VulkanContext.h>

#include <algorithm>
#include <iterator>

namespace igl::vulkan {

VulkanExtensions::VulkanExtensions() {
  extensions_.resize(kNumberOfExtensionTypes);
  enabledExtensions_.resize(kNumberOfExtensionTypes);
}

void VulkanExtensions::enumerate(const VulkanFunctionTable& vf) {
  uint32_t count = 0;
  VK_ASSERT(vf.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

  std::vector<VkExtensionProperties> allExtensions(count);

  VK_ASSERT(vf.vkEnumerateInstanceExtensionProperties(nullptr, &count, allExtensions.data()));

  constexpr size_t vectorIndex = (size_t)ExtensionType::Instance;
  std::transform(allExtensions.cbegin(),
                 allExtensions.cend(),
                 std::back_inserter(extensions_[vectorIndex]),
                 [](const VkExtensionProperties& extensionProperties) {
                   return extensionProperties.extensionName;
                 });
}

void VulkanExtensions::enumerate(const VulkanFunctionTable& vf, VkPhysicalDevice device) {
  uint32_t count = 0;
  VK_ASSERT(vf.vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));

  std::vector<VkExtensionProperties> allExtensions(count);

  VK_ASSERT(vf.vkEnumerateDeviceExtensionProperties(device, nullptr, &count, allExtensions.data()));

  constexpr size_t vectorIndex = (size_t)ExtensionType::Device;
  std::transform(allExtensions.cbegin(),
                 allExtensions.cend(),
                 std::back_inserter(extensions_[vectorIndex]),
                 [](const VkExtensionProperties& extensionProperties) {
                   return extensionProperties.extensionName;
                 });
}

const std::vector<std::string>& VulkanExtensions::allAvailableExtensions(
    ExtensionType extensionType) const {
  const size_t vectorIndex = (size_t)extensionType;
  return extensions_[vectorIndex];
}

bool VulkanExtensions::available(const char* extensionName, ExtensionType extensionType) const {
  const size_t vectorIndex = (size_t)extensionType;
  const std::string extensionNameStr(extensionName);
  auto result = std::find_if(
      extensions_[vectorIndex].begin(),
      extensions_[vectorIndex].end(),
      [&extensionNameStr](const std::string& extension) { return extension == extensionNameStr; });

  return result != extensions_[vectorIndex].end();
}

bool VulkanExtensions::enable(const char* extensionName, ExtensionType extensionType) {
  const size_t vectorIndex = (size_t)extensionType;
  if (available(extensionName, extensionType)) {
    enabledExtensions_[vectorIndex].insert(extensionName);
    return true;
  }
  return false;
}

void VulkanExtensions::enableCommonInstanceExtensions(const VulkanContextConfig& config) {
  enable(VK_KHR_SURFACE_EXTENSION_NAME, ExtensionType::Instance);
  enable(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, ExtensionType::Instance);
#if defined(VK_EXT_debug_utils)
  enable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ExtensionType::Instance);
#endif
#if IGL_PLATFORM_WINDOWS
  enable(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, ExtensionType::Instance);
#elif IGL_PLATFORM_ANDROID
  enable("VK_KHR_android_surface", ExtensionType::Instance);
#elif IGL_PLATFORM_LINUX
  enable("VK_KHR_xlib_surface", ExtensionType::Instance);
#elif IGL_PLATFORM_MACOSX
  enable(VK_EXT_METAL_SURFACE_EXTENSION_NAME, ExtensionType::Instance);
#endif

#if IGL_PLATFORM_MACOSX
  // https://vulkan.lunarg.com/doc/sdk/1.3.216.0/mac/getting_started.html
  if (!enable(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, ExtensionType::Instance)) {
    IGL_LOG_ERROR("VK_KHR_portability_enumeration extension not supported\n");
  }
#endif

#if !IGL_PLATFORM_ANDROID
  if (config.enableValidation) {
    enable(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, ExtensionType::Instance);
  }
#endif
  if (config.headless) {
#if defined(VK_EXT_headless_surface)
    const bool enabledExtension =
        enable(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, ExtensionType::Instance);
#else
    const bool enabledExtension = false;
#endif // VK_EXT_headless_surface
    if (!enabledExtension) {
      IGL_LOG_ERROR("VK_EXT_headless_surface extension not supported");
    }
  }
  if (config.swapChainColorSpace != igl::ColorSpace::SRGB_NONLINEAR) {
#if defined(VK_EXT_swapchain_colorspace)
    const bool enabledExtension =
        enable(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, ExtensionType::Instance);
#else
    const bool enabledExtension = false;
#endif
    if (!enabledExtension) {
      IGL_LOG_ERROR("VK_EXT_swapchain_colorspace extension not supported\n");
    }
  }
}

void VulkanExtensions::enableCommonDeviceExtensions(const VulkanContextConfig& config) {
  enable(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME, ExtensionType::Device);
#if IGL_PLATFORM_ANDROID
  enable(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, ExtensionType::Device);
  enable(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, ExtensionType::Device);
  enable(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME, ExtensionType::Device);
  enable(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, ExtensionType::Device);
  if (config.enableDescriptorIndexing) {
#endif
    // On Android, vkEnumerateInstanceExtensionProperties crashes when validation layers are
    // enabled for DEBUG builds. https://issuetracker.google.com/issues/209835779?pli=1 Hence,
    // allow developers to not enable certain extensions on Android which are not present.
    enable(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, ExtensionType::Device);
#if IGL_PLATFORM_ANDROID
  }
#endif
  enable(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, ExtensionType::Device);
#if !IGL_PLATFORM_ANDROID || !IGL_DEBUG
  // On Android, vkEnumerateInstanceExtensionProperties crashes when validation layers are
  // enabled for DEBUG builds. https://issuetracker.google.com/issues/209835779?pli=1 Hence,
  // don't enable some extensions on Android which are not present and no way to check without
  // crashing.
  enable(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, ExtensionType::Device);
#endif // !IGL_PLATFORM_ANDROID || !IGL_DEBUG
  enable(VK_KHR_SWAPCHAIN_EXTENSION_NAME, ExtensionType::Device);

#if IGL_PLATFORM_MACOSX
  std::ignore = IGL_DEBUG_VERIFY(enable("VK_KHR_portability_subset", ExtensionType::Device));
#endif

#if IGL_PLATFORM_WINDOWS
  enable(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, ExtensionType::Device);
#endif // IGL_PLATFORM_WINDOWS

#if IGL_PLATFORM_LINUX
  enable(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, ExtensionType::Device);
  enable(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, ExtensionType::Device);
#endif // IGL_PLATFORM_LINUX

#if defined(IGL_WITH_TRACY_GPU)
  enable(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, ExtensionType::Device);
#endif // IGL_WITH_TRACY_GPU

  has8BitIndices = enable(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME, ExtensionType::Device);

  hasTimelineSemaphore = enable(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, ExtensionType::Device);
  hasSynchronization2 = enable(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, ExtensionType::Device);
}

bool VulkanExtensions::enabled(const char* extensionName) const {
  return (enabledExtensions_[(size_t)ExtensionType::Instance].count(extensionName) > 0) ||
         (enabledExtensions_[(size_t)ExtensionType::Device].count(extensionName) > 0);
}

std::vector<const char*> VulkanExtensions::allEnabled(ExtensionType extensionType) const {
  const size_t vectorIndex = (size_t)extensionType;
  std::vector<const char*> returnList;
  for (const auto& extension : enabledExtensions_[vectorIndex]) {
    returnList.emplace_back(extension.c_str());
  }
  return returnList;
}

} // namespace igl::vulkan
