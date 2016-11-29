#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H 1

#include "Vulkan/Vulkan.h"

#include <vector>
#include <deque>



#ifdef _WIN32
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
            MessageBox(NULL, err_msg, err_class, MB_OK);                       \
        exit(1);                                                               \
			    } while (0)
#endif




#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                  \
			    {                                                                          \
        fp##entrypoint =                                                 \
            (PFN_vk##entrypoint)vkGetDeviceProcAddr(dev, "vk" #entrypoint);    \
			if(fp##entrypoint == NULL)		\
							{\
					ERR_EXIT("vkGetDeviceProcAddr failed to find vk" #entrypoint,    \
                     "vkGetDeviceProcAddr Failure");                         \
							}\
			    }

class CommandPool;
class VulkanImage;
class VulkanMemMngr;


typedef struct SyncedPresentable{
	uint32_t	mEngineImageIndex;
	VkSemaphore mWaitForPresentSemaphore;
	VkSemaphore mWaitForAcquireSemaphore;
} SyncedPresentable;


class VulkanDevice
{
public:
	VulkanDevice(VkPhysicalDevice);
	~VulkanDevice();

	VkDevice GetVkDevice()
	{
		return TheVulkanDevice;
	}

	VkPhysicalDevice GetVkPhysicalDevice()
	{
		return mPhysicalDevice;
	}

	VkQueue GetVkQueue()
	{
		return mQueue;
	}

	CommandPool * GetCommandPool() { return mCommandPool; }

	void GetDeviceExtensionPointers();
	void CreateCommandPool();
	void CreateMemoryManager();
	void PopulatePresentableImages(VkImage *, uint32_t);
	void AddPresentableIndex(SyncedPresentable);
	VulkanMemMngr * GetMemManager() { return mMemoryManager;  };

	SyncedPresentable GetNextPresentable();



private:
	void init();

private:
	VkDevice TheVulkanDevice;
	VkPhysicalDevice mPhysicalDevice;
	VkQueue mQueue;

	VkDeviceQueueCreateInfo mQueueCreateInfo;
	VkDeviceCreateInfo mCreateInfo;


	bool ImageCreated;
	VulkanImage * mImage;

	CommandPool *	mCommandPool;
	VulkanMemMngr * mMemoryManager;

	std::vector<VulkanImage> mPresentableImageArray;
	std::deque<SyncedPresentable>		mAvailableImageIndicesArray;

	// VK_KHR_swapchain function pointers

	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
};
#endif