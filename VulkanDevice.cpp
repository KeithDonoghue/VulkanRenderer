#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanMemoryManager.h"

#include "CommandPool.h"
#include "CommandBuffer.h"



#include "Windows.h"

#define  ENGINE_LOGGING_ENABLED 1
#include "EngineLogging.h"

#include <vector>

VulkanDevice::VulkanDevice(VkPhysicalDevice thePhysicalDevice):
mPhysicalDevice(thePhysicalDevice)
{
	VkDeviceQueueCreateInfo QueueCreateInfo;
	memset(&QueueCreateInfo, 0, sizeof(QueueCreateInfo));

	float queuePriorities = 1.0f;

	QueueCreateInfo.pNext = NULL;
	QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//QueueCreateInfo.flags; Reserved For future use
	QueueCreateInfo.queueFamilyIndex = 0;
	QueueCreateInfo.queueCount = 1;
	QueueCreateInfo.pQueuePriorities = &queuePriorities;




	std::vector<char *> DeviceExtensionsToEnable;
	DeviceExtensionsToEnable.push_back("VK_KHR_swapchain");


	VkDeviceCreateInfo DeviceCreateInfo;
	memset(&DeviceCreateInfo, 0, sizeof(DeviceCreateInfo));

	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.pNext = NULL;
	//DeviceCreateInfo.flags; Reserved For future use
	DeviceCreateInfo.enabledExtensionCount = DeviceExtensionsToEnable.size();
	DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensionsToEnable.data();
	//DeviceCreateInfo.enabledLayerCount; Deprecated
	//DeviceCreateInfo.ppEnabledLayerNames; Depreceated
	DeviceCreateInfo.queueCreateInfoCount = 1;
	DeviceCreateInfo.pQueueCreateInfos = &QueueCreateInfo;


	VkResult theResult = vkCreateDevice(mPhysicalDevice, &DeviceCreateInfo, nullptr, &TheVulkanDevice);

	if (theResult != VK_SUCCESS)
	{
		OutputDebugString("Failed to create Device;");
	}
	else
	{
		OutputDebugString("Hurrah! Created a device.");
	}


	vkGetDeviceQueue(TheVulkanDevice, 0, 0, &mQueue);
	CreateCommandPool();
	CreateMemoryManager();
}






VulkanDevice::~VulkanDevice()
{
	delete mImage;
	delete mCommandPool;
	delete mMemoryManager;
	vkDestroyDevice(TheVulkanDevice, nullptr);
}





void VulkanDevice::GetDeviceExtensionPointers()
{
	// VK_KHR_swapchain function pointers

	GET_DEVICE_PROC_ADDR(TheVulkanDevice, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(TheVulkanDevice, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(TheVulkanDevice, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(TheVulkanDevice, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(TheVulkanDevice, QueuePresentKHR);

}





void VulkanDevice::CreateCommandPool()
{
	mCommandPool = new CommandPool(this);
}





void VulkanDevice::PopulatePresentableImages(VkImage * ImageArray, uint32_t size)
{
	for (uint32_t i = 0; i < size; i++)
	{
		mPresentableImageArray.emplace_back(VulkanImage(ImageArray[i], true));
	}
}





uint32_t VulkanDevice::GetNextPresentable(VkSemaphore * waitSemaphore, VkSemaphore * signalSemaphore)
{
	CommandBuffer * currentCommandBuffer = mCommandPool->GetCurrentCommandBuffer();
	uint32_t nextImage = mAvailableImageIndicesArray.front();

	currentCommandBuffer->GetImageReadyForPresenting(mPresentableImageArray[nextImage]);

	if (!ImageCreated)
	{
		ImageCreated = true;
		mImage = new VulkanImage(this, 1080, 500);
	}
	mAvailableImageIndicesArray.pop_front();


		
	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo theSubmitInfo;
	memset(&theSubmitInfo, 0, sizeof(VkSubmitInfo));
	theSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	theSubmitInfo.pNext = nullptr;
	theSubmitInfo.waitSemaphoreCount = 1;
	theSubmitInfo.pWaitSemaphores = waitSemaphore;
	theSubmitInfo.pWaitDstStageMask = &stageFlags;
	theSubmitInfo.commandBufferCount = 1;
	theSubmitInfo.pCommandBuffers = currentCommandBuffer->GetVkCommandBufferAddr();
	theSubmitInfo.signalSemaphoreCount = 1;
	theSubmitInfo.pSignalSemaphores = signalSemaphore;

		
	VkResult  result = vkQueueSubmit(mQueue, 1, &theSubmitInfo, VK_NULL_HANDLE);

	if (result != VK_SUCCESS)
	{
		EngineLog("Queue submission failed.");
	}

	return nextImage;
}





void VulkanDevice::AddPresentableIndex(uint32_t freeImage)
{
	mPresentableImageArray[freeImage].SetLayout(VK_IMAGE_LAYOUT_UNDEFINED);
	mAvailableImageIndicesArray.push_back(freeImage);
}




void VulkanDevice::CreateMemoryManager()
{
	mMemoryManager = new VulkanMemMngr(this);
}


