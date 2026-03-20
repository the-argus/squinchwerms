module;

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <array>
#include <string_view>

#include "macros.h"

export module renderer;

import opt;
import aliases;
import allocator;
import res;
import logging;

struct QueueFamilyIndices
{
    Opt<u32> graphicsFamily;
    Opt<u32> presentFamily;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatsCount;
    u32 presentModesCount;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
};

constexpr bool enableValidationLayers =
#ifdef ENABLE_VALIDATION_LAYERS
    true
#else
    false
#endif
    ;

constexpr size_t maxFramesInFlight = 2;

constexpr std::array validationLayers = {"VK_LAYER_KHRONOS_validation"};

constexpr std::array deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Res<bool, alloc::Error> checkValidationLayerSupport(Allocator &allocator)
{
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    auto availableLayers =
        allocator
            .allocate({.numBytes = sizeof(VkLayerProperties) * layerCount,
                       .alignment = alignof(VkLayerProperties)})
            .map([](auto slice) {
                return reinterpretBytesAs<VkLayerProperties>(slice);
            });
    if (isError(availableLayers))
        return availableLayers.error();

    vkEnumerateInstanceLayerProperties(&layerCount,
                                       availableLayers->addressOfFirstItem());

    for (const char *layerCString : validationLayers) {
        const std::string_view layer{layerCString};
        bool layerFound = false;

        for (const VkLayerProperties &availableLayer :
             availableLayers.unwrap()) {
            const std::string_view layerName{availableLayer.layerName};
            if (layer == layerName) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

enum class ExtensionError {
    Success,
    OOM,
    SDLFailure,
};

Res<Slice<const char*>, ExtensionError> getRequiredExtensions(Allocator& allocator) {
	/*
	 * If validation layers are enabled, we need to request the VK_EXT_DEBUG_UTILS_EXTENSION_NAME extension
	 * as well as the extensions required by SDL.
	 *
	 * Note: calling this repeatedly could cause a memory leak as we create a new array each time
	 * if validation layers are enabled.
	 */
	
	// Get the required extensions from SDL and set count to the number of extensions
	char const * const * sdlExtensions = SDL_Vulkan_GetInstanceExtensions(count);

	if (sdlExtensions == nullptr) {
        lg::error(lg::Category:Renderer, "Failed to get required extensions from SDL");
        return ExtensionError::SDLFailure;
	}

	if (!enableValidationLayers) {
		return sdlExtensions;
	}

	// If validation layers are enabled, add the debug utils extension
	const char** extensions = malloc(sizeof(const char*) * (*count + 1));
	memcpy(extensions, sdl_extensions, sizeof(const char*) * *count);
	extensions[*count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	*count += 1;
	
	return (char const * const *) extensions;
}

export class Renderer
{
  private:
    VkInstance instance = {};
    VkDebugUtilsMessengerEXT debugMessenger = {};
    VkSurfaceKHR surface = {};
    // physical device is the gpu
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // Logical device is the interface to the physical device
    VkDevice device = {};

    VkQueue graphicsQueue = {};
    VkQueue presentQueue = {};

    VkSwapchainKHR swapchain = {};
    u32 swapchainImagesCount = 0;
    VkImage *swapchainImages = nullptr;
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent = {};
    u32 swapChainImageViewsCount = 0;
    VkImageView *swapchainImageViews = NULL;
    u32 swapchainFramebuffersCount = 0;
    VkFramebuffer *swapchainFramebuffers = NULL;

    VkRenderPass renderPass = {};
    VkDescriptorSetLayout descriptorSetLayout = {};
    VkPipelineLayout pipelineLayout = {};
    VkPipeline graphicsPipeline = {};

    VkDescriptorPool descriptorPool = {};
    VkDescriptorSet descriptorSets[maxFramesInFlight] = {};

    VkCommandPool commandPool = {};
    const u32 commandBuffersCount = maxFramesInFlight;
    VkCommandBuffer commandBuffers[maxFramesInFlight] = {};

    // Semaphores: need enough to avoid reuse before presentation completes
    // Use max of MAX_FRAMES_IN_FLIGHT and swapchain image count
    uint32_t semaphoresCount = 0;
    VkSemaphore *imageAvailableSemaphores = NULL;
    VkSemaphore *renderFinishedSemaphores = NULL;

    // Per-frame fences
    const u32 inFlightFencesCount = maxFramesInFlight;
    VkFence inFlightFences[maxFramesInFlight] = {};

    u32 currentFrame = 0;
    bool framebufferResized = false;

    VkBuffer vertexBuffer = {};
    VkDeviceMemory vertexBufferMemory = {};

    VkBuffer indexBuffer = {};
    VkDeviceMemory indexBufferMemory = {};

    VkBuffer uniformBuffers[maxFramesInFlight] = {};
    VkDeviceMemory uniformBuffersMemory[maxFramesInFlight] = {};
    void *uniformBuffersMapped[maxFramesInFlight] = {};
};
