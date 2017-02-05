#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H 1

#include "Vulkan/Vulkan.h"


#include <vector>
#include <deque>
#include <mutex>
#include <memory>


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
class DescriptorPool;
class VulkanImage;
class VulkanMemMngr;
class RenderPass;
class ShaderModule;
class VulkanPipeline;
class RenderInstance;


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

	VkDevice getVkDevice()
	{
		return TheVulkanDevice;
	}

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

	CommandPool& GetCommandPool() { return *mCommandPool; }
	DescriptorPool& GetDescriptorPool() { return *mDescriptorPool; }

	void GetDeviceExtensionPointers();
	void CreateCommandPool();
	void CreateMemoryManager();
	void PopulatePresentableImages(VkImage *, uint32_t, uint32_t, uint32_t);
	void AddToAvailableQueue(SyncedPresentable);
	VulkanMemMngr& GetMemManager() { return *mMemoryManager;  };

	bool GetFromAvailableQueue(SyncedPresentable&);
	void AddToPresentQueue(SyncedPresentable);
	bool GetFromPresentQueue(SyncedPresentable&);
	void CreateInitialData();
	void CreateRenderTargets(int, int);

	void Update();
	void DoRender(uint32_t);
	void TakeInput(unsigned int);


	void CreateDescriptorPool();


	void LockQueue()	{ mQueueLock.lock() ; }
	void UnlockQueue()	{ mQueueLock.unlock() ; }


private:
	void init();

private:
	VkDevice TheVulkanDevice;
	VkPhysicalDevice mPhysicalDevice;
	VkQueue mQueue;
	std::mutex mQueueLock;

	VkDeviceQueueCreateInfo mQueueCreateInfo;
	VkDeviceCreateInfo mCreateInfo;

	VulkanImage * mImage;

	std::vector<VulkanImage*> mDepthImages;
	std::vector<RenderPass*> mRenderPasses;

	std::unique_ptr<DescriptorPool> mDescriptorPool;
	std::unique_ptr<CommandPool>	mCommandPool;
	std::unique_ptr<VulkanMemMngr> mMemoryManager;


	ShaderModule * mVert;
	ShaderModule * mFrag, *mFrag2;
	VulkanPipeline * mPipeline, *mPipeline2;
	RenderInstance * mRenderInstance, *mRenderInstance2;



	std::vector<VulkanImage>			mPresentableImageArray;
	std::deque<SyncedPresentable>		mAvailableImageIndicesArray;
	std::mutex							mAvailableImageIndicesArrayLock;

	std::deque<SyncedPresentable>		mPresentableImageIndicesArray;
	std::mutex							mPresentableImageIndicesArrayLock;

	// VK_KHR_swapchain function pointers

	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;



	VkSampler theSampler;
	VkImageView theView;
	bool inited = false;

};
#endif