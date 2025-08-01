/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// This is a very low-level C wrapper of the Vulkan API to make it slightly less painful to use.

#pragma once

#include <glslang/Include/glslang_c_interface.h>

#include <igl/Macros.h>
#include <igl/vulkan/VulkanFunctionTable.h>
#include <igl/vulkan/VulkanVma.h>

#ifdef __cplusplus
extern "C" {
#endif

// @fb-only
// @fb-only
// @fb-only
// @fb-only
  // @fb-only
  // @fb-only
  // @fb-only
// @fb-only
// @fb-only
// @fb-only
  // @fb-only
  // @fb-only
  // @fb-only
// @fb-only

/// @brief Adds a node to the linked list of next nodes
void ivkAddNext(void* node, const void* next);

const char* ivkGetVulkanResultString(VkResult result);

/// @brief Creates a Debug Utils Messenger if the VK_EXT_debug_utils extension is available and the
/// platform is not Android or Mac Catalyst. Otherwise the function is defined as a no-op that
/// always returns VK_SUCCESS
VkResult ivkCreateDebugUtilsMessenger(const struct VulkanFunctionTable* vt,
                                      VkInstance instance,
                                      PFN_vkDebugUtilsMessengerCallbackEXT callback,
                                      void* logUserData,
                                      VkDebugUtilsMessengerEXT* outMessenger);

// This function uses VK_EXT_debug_report extension, which is deprecated by VK_EXT_debug_utils.
// However, it is available on some Android devices where VK_EXT_debug_utils is not available.
VkResult ivkCreateDebugReportMessenger(const struct VulkanFunctionTable* vt,
                                       VkInstance instance,
                                       PFN_vkDebugReportCallbackEXT callback,
                                       void* logUserData,
                                       VkDebugReportCallbackEXT* outMessenger);

/** @brief Creates a platform specific VkSurfaceKHR object. The surface creation functions
 * conditionally-compiled and guarded by their respective platform specific extension macros defined
 * by the Vulkan API. The current supported platforms, and their macros, are:
 * - Windows: VK_USE_PLATFORM_WIN32_KHR
 * - Linux: VK_USE_PLATFORM_XLIB_KHR
 * - MacOS: VK_USE_PLATFORM_METAL_EXT
 * - Android: VK_USE_PLATFORM_ANDROID_KHR
 */
VkResult ivkCreateSurface(const struct VulkanFunctionTable* vt,
                          VkInstance instance,
                          void* window,
                          void* display,
                          void* layer,
                          VkSurfaceKHR* outSurface);

/** @brief Creates a Vulkan device from the given physical device. The physical device features
 * (VkPhysicalDeviceFeatures) conditionally enabled, based on the provided supported physical device
 * features, are `dualSrcBlend`, `multiDrawIndirect`, `drawIndirectFirstInstance`, `depthBiasClamp`,
 * `fillModeNonSolid`, and `shaderInt16`.
 * The validation layers enabled are defined in `kDefaultValidationLayers`
 * If descriptor indexing is enabled, then the following descriptor indexing features
 * (VkPhysicalDeviceDescriptorIndexingFeaturesEXT) are enabled:
 * `shaderSampledImageArrayNonUniformIndexing`, `descriptorBindingUniformBufferUpdateAfterBind`,
 * `descriptorBindingSampledImageUpdateAfterBind`, `descriptorBindingStorageImageUpdateAfterBind`,
 * `descriptorBindingStorageBufferUpdateAfterBind`, `descriptorBindingUpdateUnusedWhilePending`,
 * `descriptorBindingPartiallyBound`, `runtimeDescriptorArray`.
 * If 16-bit shader floats are enabled, then the following shader float 16 features are enabled:
 * VkPhysicalDevice16BitStorageFeatures::storageBuffer16BitAccess
 * VkPhysicalDeviceShaderFloat16Int8Features::shaderFloat16
 * If the `VK_KHR_buffer_device_address` extension is available, then
 * VkPhysicalDeviceBufferDeviceAddressFeaturesKHR::bufferDeviceAddress is enabled If multiview is
 * enabled, then VkPhysicalDeviceMultiviewFeatures::multiview is enabled
 */
VkResult ivkCreateDevice(const struct VulkanFunctionTable* vt,
                         VkPhysicalDevice physicalDevice,
                         size_t numQueueCreateInfos,
                         const VkDeviceQueueCreateInfo* queueCreateInfos,
                         size_t numDeviceExtensions,
                         const char** deviceExtensions,
                         const VkPhysicalDeviceFeatures2* supported,
                         VkDevice* outDevice);

VkResult ivkCreateHeadlessSurface(const struct VulkanFunctionTable* vt,
                                  VkInstance instance,
                                  VkSurfaceKHR* surface);

VkResult ivkCreateSwapchain(const struct VulkanFunctionTable* vt,
                            VkDevice device,
                            VkSurfaceKHR surface,
                            uint32_t minImageCount,
                            VkSurfaceFormatKHR surfaceFormat,
                            VkPresentModeKHR presentMode,
                            const VkSurfaceCapabilitiesKHR* caps,
                            VkImageUsageFlags imageUsage,
                            uint32_t queueFamilyIndex,
                            uint32_t width,
                            uint32_t height,
                            VkSwapchainKHR* outSwapchain);

/// @brief Returns VkImageViewCreateInfo with the R, G, B, and A components mapped to themselves
/// (identity)
VkImageViewCreateInfo ivkGetImageViewCreateInfo(VkImage image,
                                                VkImageViewType type,
                                                VkFormat imageFormat,
                                                VkImageSubresourceRange range);

VkResult ivkCreateFramebuffer(const struct VulkanFunctionTable* vt,
                              VkDevice device,
                              uint32_t width,
                              uint32_t height,
                              VkRenderPass renderPass,
                              size_t numAttachments,
                              const VkImageView* attachments,
                              VkFramebuffer* outFramebuffer);

VkResult ivkCreateCommandPool(const struct VulkanFunctionTable* vt,
                              VkDevice device,
                              VkCommandPoolCreateFlags flags,
                              uint32_t queueFamilyIndex,
                              VkCommandPool* outCommandPool);

VkResult ivkAllocateCommandBuffer(const struct VulkanFunctionTable* vt,
                                  VkDevice device,
                                  VkCommandPool commandPool,
                                  VkCommandBuffer* outCommandBuffer);

VkResult ivkAllocateMemory(const struct VulkanFunctionTable* vt,
                           VkPhysicalDevice physDev,
                           VkDevice device,
                           const VkMemoryRequirements* memRequirements,
                           VkMemoryPropertyFlags props,
                           bool enableBufferDeviceAddress,
                           VkDeviceMemory* outMemory);

VkResult ivkAllocateMemory2(const struct VulkanFunctionTable* vt,
                            VkPhysicalDevice physDev,
                            VkDevice device,
                            const VkMemoryRequirements2* memRequirements,
                            VkMemoryPropertyFlags props,
                            bool enableBufferDeviceAddress,
                            VkDeviceMemory* outMemory);

VkImagePlaneMemoryRequirementsInfo ivkGetImagePlaneMemoryRequirementsInfo(
    VkImageAspectFlagBits plane);

VkImageMemoryRequirementsInfo2 ivkGetImageMemoryRequirementsInfo2(
    const VkImagePlaneMemoryRequirementsInfo* next,
    VkImage image);

VkBindImageMemoryInfo ivkGetBindImageMemoryInfo(const VkBindImagePlaneMemoryInfo* next,
                                                VkImage image,
                                                VkDeviceMemory memory);

bool ivkIsHostVisibleSingleHeapMemory(const struct VulkanFunctionTable* vt,
                                      VkPhysicalDevice physDev);

uint32_t ivkFindMemoryType(const struct VulkanFunctionTable* vt,
                           VkPhysicalDevice physDev,
                           uint32_t memoryTypeBits,
                           VkMemoryPropertyFlags flags);

VkResult ivkCreateGraphicsPipeline(const struct VulkanFunctionTable* vt,
                                   VkDevice device,
                                   VkPipelineCache pipelineCache,
                                   uint32_t numShaderStages,
                                   const VkPipelineShaderStageCreateInfo* shaderStages,
                                   const VkPipelineVertexInputStateCreateInfo* vertexInputState,
                                   const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState,
                                   const VkPipelineTessellationStateCreateInfo* tessellationState,
                                   const VkPipelineViewportStateCreateInfo* viewportState,
                                   const VkPipelineRasterizationStateCreateInfo* rasterizationState,
                                   const VkPipelineMultisampleStateCreateInfo* multisampleState,
                                   const VkPipelineDepthStencilStateCreateInfo* depthStencilState,
                                   const VkPipelineColorBlendStateCreateInfo* colorBlendState,
                                   const VkPipelineDynamicStateCreateInfo* dynamicState,
                                   VkPipelineLayout pipelineLayout,
                                   VkRenderPass renderPass,
                                   VkPipeline* outPipeline);

VkResult ivkCreateComputePipeline(const struct VulkanFunctionTable* vt,
                                  VkDevice device,
                                  VkPipelineCache pipelineCache,
                                  const VkPipelineShaderStageCreateInfo* shaderStage,
                                  VkPipelineLayout pipelineLayout,
                                  VkPipeline* outPipeline);

VkResult ivkCreateDescriptorSetLayout(const struct VulkanFunctionTable* vt,
                                      VkDevice device,
                                      VkDescriptorSetLayoutCreateFlags flags,
                                      uint32_t numBindings,
                                      const VkDescriptorSetLayoutBinding* bindings,
                                      const VkDescriptorBindingFlags* bindingFlags,
                                      VkDescriptorSetLayout* outLayout);

/// @brief Creates a VkDescriptorSetLayoutBinding structure
VkDescriptorSetLayoutBinding ivkGetDescriptorSetLayoutBinding(uint32_t binding,
                                                              VkDescriptorType descriptorType,
                                                              uint32_t descriptorCount,
                                                              VkShaderStageFlags stageFlags);

VkResult ivkAllocateDescriptorSet(const struct VulkanFunctionTable* vt,
                                  VkDevice device,
                                  VkDescriptorPool pool,
                                  VkDescriptorSetLayout layout,
                                  VkDescriptorSet* outDescriptorSet);

VkResult ivkCreateDescriptorPool(const struct VulkanFunctionTable* vt,
                                 VkDevice device,
                                 VkDescriptorPoolCreateFlags flags,
                                 uint32_t maxDescriptorSets,
                                 uint32_t numPoolSizes,
                                 const VkDescriptorPoolSize* poolSizes,
                                 VkDescriptorPool* outDescriptorPool);

/// @brief Creates a VkSubmitInfo structure with an optional semaphore, used to signal when the
/// command buffer for this batch have completed execution
VkSubmitInfo ivkGetSubmitInfo(const VkCommandBuffer* buffer,
                              uint32_t numWaitSemaphores,
                              const VkSemaphore* waitSemaphores,
                              const VkPipelineStageFlags* waitStageMasks,
                              const VkSemaphore* releaseSemaphore);

/// @brief Creates a VkAttachmentDescription2 structure with load and store operations for the
/// stencil attachment as "Don't Care"
VkAttachmentDescription2 ivkGetAttachmentDescriptionColor(VkFormat format,
                                                          VkAttachmentLoadOp loadOp,
                                                          VkAttachmentStoreOp storeOp,
                                                          VkImageLayout initialLayout,
                                                          VkImageLayout finalLayout);

/// @brief Creates a VkAttachmentReference2 structure with its layout set to
/// `Vk_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`
VkAttachmentReference2 ivkGetAttachmentReferenceColor(uint32_t idx);

VkClearValue ivkGetClearDepthStencilValue(float depth, uint32_t stencil);
VkClearValue ivkGetClearColorValue(float r, float g, float b, float a);

VkBufferCreateInfo ivkGetBufferCreateInfo(uint64_t size, VkBufferUsageFlags usage);

/// @brief Creates a VkImageCreateInfo structure with its layout set to `VK_IMAGE_LAYOUT_UNDEFINED`
VkImageCreateInfo ivkGetImageCreateInfo(VkImageType type,
                                        VkFormat imageFormat,
                                        VkImageTiling tiling,
                                        VkImageUsageFlags usage,
                                        VkExtent3D extent,
                                        uint32_t mipLevels,
                                        uint32_t arrayLayers,
                                        VkImageCreateFlags flags,
                                        VkSampleCountFlags samples);

VkPipelineVertexInputStateCreateInfo ivkGetPipelineVertexInputStateCreateInfo_Empty(void);

/// @brief Creates an empty VkPipelineVertexInputStateCreateInfo structure
VkPipelineVertexInputStateCreateInfo ivkGetPipelineVertexInputStateCreateInfo(
    uint32_t vbCount,
    const VkVertexInputBindingDescription* bindings,
    uint32_t vaCount,
    const VkVertexInputAttributeDescription* attributes);

VkPipelineInputAssemblyStateCreateInfo ivkGetPipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology,
    VkBool32 enablePrimitiveRestart);

VkPipelineDynamicStateCreateInfo ivkGetPipelineDynamicStateCreateInfo(
    uint32_t numDynamicStates,
    const VkDynamicState* dynamicStates);

/// @brief Creates a VkPipelineRasterizationStateCreateInfo structure with default values, except
/// for polygon mode and cull mode flags
VkPipelineRasterizationStateCreateInfo ivkGetPipelineRasterizationStateCreateInfo(
    VkPolygonMode polygonMode,
    VkCullModeFlags cullMode);

/// @brief Creates a VkPipelineMultisampleStateCreateInfo structure with default values
VkPipelineMultisampleStateCreateInfo ivkGetPipelineMultisampleStateCreateInfo_Empty(void);

/// @brief Creates a VkPipelineDepthStencilStateCreateInfo structure with depth test and write
/// disabled
VkPipelineDepthStencilStateCreateInfo ivkGetPipelineDepthStencilStateCreateInfo_NoDepthStencilTests(
    void);

/// @brief Creates a VkPipelineColorBlendAttachmentState structure with blending disabled
VkPipelineColorBlendAttachmentState ivkGetPipelineColorBlendAttachmentState_NoBlending(void);

VkPipelineColorBlendAttachmentState ivkGetPipelineColorBlendAttachmentState(
    bool blendEnable,
    VkBlendFactor srcColorBlendFactor,
    VkBlendFactor dstColorBlendFactor,
    VkBlendOp colorBlendOp,
    VkBlendFactor srcAlphaBlendFactor,
    VkBlendFactor dstAlphaBlendFactor,
    VkBlendOp alphaBlendOp,
    VkColorComponentFlags colorWriteMask);

VkPipelineColorBlendStateCreateInfo ivkGetPipelineColorBlendStateCreateInfo(
    uint32_t numAttachments,
    const VkPipelineColorBlendAttachmentState* colorBlendAttachmentStates);

/// @brief Creates a VkPipelineViewportStateCreateInfo structure. If the viewport state is dyanamoc,
/// the parameters `viewport` and `scissor` may be NULL
VkPipelineViewportStateCreateInfo ivkGetPipelineViewportStateCreateInfo(const VkViewport* viewport,
                                                                        const VkRect2D* scissor);

/// @brief Creates a VkImageSubresourceRange structure for the first layer and mip level
VkImageSubresourceRange ivkGetImageSubresourceRange(VkImageAspectFlags aspectMask);

VkWriteDescriptorSet ivkGetWriteDescriptorSet_ImageInfo(VkDescriptorSet dstSet,
                                                        uint32_t dstBinding,
                                                        VkDescriptorType descriptorType,
                                                        uint32_t numDescriptors,
                                                        const VkDescriptorImageInfo* pImageInfo);

VkWriteDescriptorSet ivkGetWriteDescriptorSet_BufferInfo(VkDescriptorSet dstSet,
                                                         uint32_t dstBinding,
                                                         VkDescriptorType descriptorType,
                                                         uint32_t numDescriptors,
                                                         const VkDescriptorBufferInfo* pBufferInfo);

VkPipelineLayoutCreateInfo ivkGetPipelineLayoutCreateInfo(uint32_t numLayouts,
                                                          const VkDescriptorSetLayout* layouts,
                                                          const VkPushConstantRange* range);

VkPushConstantRange ivkGetPushConstantRange(VkShaderStageFlags stageFlags,
                                            size_t offset,
                                            size_t size);

VkRect2D ivkGetRect2D(int32_t x, int32_t y, uint32_t width, uint32_t height);

VkPipelineShaderStageCreateInfo ivkGetPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                                                    VkShaderModule shaderModule,
                                                                    const char* entryPoint);

VkImageCopy ivkGetImageCopy2D(VkOffset2D srcDstOffset,
                              VkImageSubresourceLayers srcImageSubresource,
                              VkImageSubresourceLayers dstImageSubresource,
                              VkExtent2D imageRegion);

VkBufferImageCopy ivkGetBufferImageCopy2D(uint32_t bufferOffset,
                                          uint32_t bufferRowLength,
                                          VkRect2D imageRegion,
                                          VkImageSubresourceLayers imageSubresource);
VkBufferImageCopy ivkGetBufferImageCopy3D(uint32_t bufferOffset,
                                          uint32_t bufferRowLength,
                                          VkOffset3D offset,
                                          VkExtent3D extent,
                                          VkImageSubresourceLayers imageSubresource);

void ivkImageMemoryBarrier(const struct VulkanFunctionTable* vt,
                           VkCommandBuffer buffer,
                           VkImage image,
                           VkAccessFlags srcAccessMask,
                           VkAccessFlags dstAccessMask,
                           VkImageLayout oldImageLayout,
                           VkImageLayout newImageLayout,
                           VkPipelineStageFlags srcStageMask,
                           VkPipelineStageFlags dstStageMask,
                           VkImageSubresourceRange subresourceRange);

void ivkBufferMemoryBarrier(const struct VulkanFunctionTable* vt,
                            VkCommandBuffer cmdBuffer,
                            VkBuffer buffer,
                            VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask,
                            VkDeviceSize offset,
                            VkDeviceSize size,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask);

void ivkBufferBarrier(const struct VulkanFunctionTable* vt,
                      VkCommandBuffer cmdBuffer,
                      VkBuffer buffer,
                      VkBufferUsageFlags usageFlags,
                      VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask);

void ivkCmdBlitImage(const struct VulkanFunctionTable* vt,
                     VkCommandBuffer buffer,
                     VkImage srcImage,
                     VkImage dstImage,
                     VkImageLayout srcImageLayout,
                     VkImageLayout dstImageLayout,
                     const VkOffset3D* srcOffsets,
                     const VkOffset3D* dstOffsets,
                     VkImageSubresourceLayers srcSubresourceRange,
                     VkImageSubresourceLayers dstSubresourceRange,
                     VkFilter filter);

/// @brief Adds a name for the Vulkan object with handle equals to `handle` and type equals to
/// `type`. This function is a no-op if `VK_EXT_DEBUG_UTILS_SUPPORTED` is not defined
VkResult ivkSetDebugObjectName(const struct VulkanFunctionTable* vt,
                               VkDevice device,
                               VkObjectType type,
                               uint64_t handle,
                               const char* name);

/// @brief Opens a command buffer debug region. This function is a no-op if
/// `VK_EXT_DEBUG_UTILS_SUPPORTED` is not defined
void ivkCmdBeginDebugUtilsLabel(const struct VulkanFunctionTable* vt,
                                VkCommandBuffer buffer,
                                const char* name,
                                const float colorRGBA[4]);

/// @brief Inserts a debug label into a command buffer. This function is a no-op if
/// `VK_EXT_DEBUG_UTILS_SUPPORTED` is not defined
void ivkCmdInsertDebugUtilsLabel(const struct VulkanFunctionTable* vt,
                                 VkCommandBuffer buffer,
                                 const char* name,
                                 const float colorRGBA[4]);

/// @brief Closes a command buffer debug region. This function is a no-op if
/// `VK_EXT_DEBUG_UTILS_SUPPORTED` is not defined
void ivkCmdEndDebugUtilsLabel(const struct VulkanFunctionTable* vt, VkCommandBuffer buffer);

VkVertexInputBindingDescription ivkGetVertexInputBindingDescription(uint32_t binding,
                                                                    uint32_t stride,
                                                                    VkVertexInputRate inputRate);
VkVertexInputAttributeDescription ivkGetVertexInputAttributeDescription(uint32_t location,
                                                                        uint32_t binding,
                                                                        VkFormat format,
                                                                        uint32_t offset);

VkResult ivkVmaCreateAllocator(const struct VulkanFunctionTable* vt,
                               VkPhysicalDevice physDev,
                               VkDevice device,
                               VkInstance instance,
                               uint32_t apiVersion,
                               bool enableBufferDeviceAddress,
                               VkDeviceSize preferredLargeHeapBlockSize,
                               VmaAllocator* outVma);

void ivkUpdateGlslangResource(glslang_resource_t* res, const VkPhysicalDeviceProperties* props);

#ifdef __cplusplus
}
#endif
