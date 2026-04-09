#pragma once
#include "GLOBALS.h"

#if VULKAN == 1
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "ErrorHandling/Errormessage.h"

class VulkanSwapchain {
public:
	bool Init(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	bool CleanupSwapchain(VkDevice& device, VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers);
	VkExtent2D ChooseSwapchainExtent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	//Getters
	VkSwapchainKHR& GetSwapchain() { return swapchain; }
	const VkSwapchainKHR& GetSwapchain() const { return swapchain; }

	VkExtent2D& GetSwapchainExtent() { return swapchainExtent; }
	const VkExtent2D& GetSwapchainExtent() const { return swapchainExtent; }

	VkFormat& GetSwapchainImageFormat() { return swapchainImageFormat; }
	const VkFormat& GetSwapchainImageFormat() const { return swapchainImageFormat; }

	std::vector<VkImage>& GetSwapchainImages() { return swapchainImages; }
	const std::vector<VkImage>& GetSwapchainImages() const { return swapchainImages; }

	std::vector<VkFramebuffer>& GetSwapchainFramebuffers() { return swapchainFramebuffers; }
	const std::vector<VkFramebuffer>& GetSwapchainFramebuffers() const { return swapchainFramebuffers; }

	std::vector<VkImageView>& GetSwapchainImageViews() { return swapchainImageViews; }
	const std::vector<VkImageView>& GetSwapchainImageViews() const { return swapchainImageViews; }
private:
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D swapchainExtent = {};
	VkFormat swapchainImageFormat = {};
	std::vector<VkImage> swapchainImages = {};
	std::vector<VkFramebuffer> swapchainFramebuffers = {};
	std::vector<VkImageView> swapchainImageViews = {};
};
#endif