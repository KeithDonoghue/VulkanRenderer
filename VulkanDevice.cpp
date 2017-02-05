#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanMemoryManager.h"

#include "DescriptorPool.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include "VulkanPipeline.h"
#include "RenderInstance.h"




#include "Windows.h"

#define  ENGINE_LOGGING_ENABLED 1
#include "EngineLogging.h"

#include <vector>

VulkanDevice::VulkanDevice(VkPhysicalDevice thePhysicalDevice):
mPhysicalDevice(thePhysicalDevice)
{
	VkDeviceQueueCreateInfo QueueCreateInfo;
	memset(&QueueCreateInfo, 0, sizeof(QueueCreateInfo));

	float queuePriorities= 1.0f;

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
	CreateDescriptorPool();
}






VulkanDevice::~VulkanDevice()
{
	vkDeviceWaitIdle(TheVulkanDevice);

	delete mRenderInstance;
	delete mRenderInstance2;

	for (size_t i = 0; i < mDepthImages.size(); i++)
	{
		delete mDepthImages[i];
		delete mRenderPasses[i];
	}

	delete mVert;
	delete mFrag;
	delete mFrag2;
	delete mPipeline;
	delete mPipeline2;


	delete mImage;
	
	mCommandPool.reset();
	mMemoryManager.reset();
	mDescriptorPool.reset();
	
	std::lock_guard<std::mutex> lock1(mAvailableImageIndicesArrayLock);
	std::lock_guard<std::mutex> lock2(mPresentableImageIndicesArrayLock);

	for (SyncedPresentable it : mAvailableImageIndicesArray)
	{
		vkDestroySemaphore(TheVulkanDevice, it.mWaitForAcquireSemaphore, nullptr);
		vkDestroySemaphore(TheVulkanDevice, it.mWaitForPresentSemaphore, nullptr);
	}

	for (SyncedPresentable it : mPresentableImageIndicesArray)
	{
		vkDestroySemaphore(TheVulkanDevice, it.mWaitForAcquireSemaphore, nullptr);
		vkDestroySemaphore(TheVulkanDevice, it.mWaitForPresentSemaphore, nullptr);
	}


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





void VulkanDevice::PopulatePresentableImages(VkImage * ImageArray, uint32_t size, uint32_t width, uint32_t height)
{
	for (uint32_t i = 0; i < size; i++)
	{
		mPresentableImageArray.emplace_back(VulkanImage(this, ImageArray[i], width, height, true));
	}
}





void VulkanDevice::CreateCommandPool()
{
	mCommandPool = std::make_unique<CommandPool>(this);
}





void VulkanDevice::CreateMemoryManager()
{
	mMemoryManager = std::make_unique<VulkanMemMngr>(this);
}





void VulkanDevice::CreateDescriptorPool()
{
	mDescriptorPool = std::make_unique<DescriptorPool>(*this);
}





void VulkanDevice::AddToAvailableQueue(SyncedPresentable freeImage)
{
	mPresentableImageArray[freeImage.mEngineImageIndex].SetLayout(VK_IMAGE_LAYOUT_UNDEFINED);

	std::lock_guard<std::mutex> lock(mAvailableImageIndicesArrayLock);
	mAvailableImageIndicesArray.push_back(freeImage);
}





bool VulkanDevice::GetFromAvailableQueue( SyncedPresentable& nextPresentable)
{
	bool imageAvailable = false;

	std::lock_guard<std::mutex> lock(mAvailableImageIndicesArrayLock);

	if (!mAvailableImageIndicesArray.empty())
	{
		nextPresentable = mAvailableImageIndicesArray.front();
		mAvailableImageIndicesArray.pop_front();
		imageAvailable = true;
	}

	return imageAvailable;
}





void VulkanDevice::AddToPresentQueue(SyncedPresentable addPresentable)
{
	std::lock_guard<std::mutex> lock(mPresentableImageIndicesArrayLock);

	mPresentableImageIndicesArray.push_back(addPresentable);
}





bool VulkanDevice::GetFromPresentQueue(SyncedPresentable& nextPresentable)
{
	bool presentableAvailable = false;

	std::lock_guard<std::mutex> lock(mPresentableImageIndicesArrayLock);

	if (!mPresentableImageIndicesArray.empty())
	{
		nextPresentable = mPresentableImageIndicesArray.front();
		mPresentableImageIndicesArray.pop_front();
		presentableAvailable = true;
	}

	return presentableAvailable;
}





void  VulkanDevice::CreateRenderTargets(int width, int height)
{
	for (size_t i = 0; i < mPresentableImageArray.size(); i++)
	{
		mDepthImages.emplace_back(new VulkanImage(this, width, height, ImageType::VULKAN_IMAGE_DEPTH));
		mRenderPasses.emplace_back(new RenderPass(*this, *mDepthImages[i], mPresentableImageArray[i]));
	}
}






void  VulkanDevice::CreateInitialData()
{
	mImage = new VulkanImage(this, "Resources/jpeg_bad.jpg");



	mVert = ShaderModule::CreateVertexShader(*this);
	mFrag = ShaderModule::CreateFragmentShader(*this);
	mFrag2 = ShaderModule::CreateFragmentShader2(*this);
	mPipeline = new VulkanPipeline(*this, *mRenderPasses[0], *mVert, *mFrag);
	mPipeline2 = new VulkanPipeline(*this, *mRenderPasses[0], *mVert, *mFrag2);
	mRenderInstance = new RenderInstance(*this, *mPipeline, *mImage);
	mRenderInstance2 = new RenderInstance(*this, *mPipeline2, *mImage);

	std::shared_ptr<VulkanBuffer> drawbuffer = VulkanBuffer::SetUpVertexBuffer(*this);
	std::shared_ptr<VulkanBuffer> indexDrawbuffer = VulkanBuffer::SetUpVertexIndexBuffer(*this);
	std::shared_ptr<VulkanBuffer> indexbuffer = VulkanBuffer::SetUpIndexBuffer(*this);

	VertexDraw	theDraw(6, 1, 0, 0, drawbuffer);
	IndexDraw	theIndexDraw(36, 1, 0, 0, 0, indexDrawbuffer, indexbuffer);

	mRenderInstance->SetDraw(theDraw);
	mRenderInstance2->SetDraw(theDraw);

	mRenderInstance->SetDraw(theIndexDraw);
	mRenderInstance2->SetDraw(theIndexDraw);
}





void VulkanDevice::Update()
{

	CommandBuffer * currentCommandBuffer = mCommandPool->GetCurrentCommandBuffer();
	SyncedPresentable nextPresentable;
	bool isPresentableReady = GetFromAvailableQueue(nextPresentable);

	if (!isPresentableReady)
		return;

	uint32_t nextImage = nextPresentable.mEngineImageIndex;


	mPresentableImageArray[nextImage].ClearImage(1.0f);
	mPresentableImageArray[nextImage].BlitFullImage(*mImage);
	
	DoRender(nextImage);

	currentCommandBuffer->GetImageReadyForPresenting(mPresentableImageArray[nextImage]);


	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo theSubmitInfo;
	memset(&theSubmitInfo, 0, sizeof(VkSubmitInfo));
	theSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	theSubmitInfo.pNext = nullptr;
	theSubmitInfo.waitSemaphoreCount = 1;
	theSubmitInfo.pWaitSemaphores = &nextPresentable.mWaitForAcquireSemaphore;
	theSubmitInfo.pWaitDstStageMask = &stageFlags;
	theSubmitInfo.commandBufferCount = 1;
	theSubmitInfo.pCommandBuffers = currentCommandBuffer->GetVkCommandBufferAddr();
	theSubmitInfo.signalSemaphoreCount = 1;
	theSubmitInfo.pSignalSemaphores = &nextPresentable.mWaitForPresentSemaphore;

	currentCommandBuffer->EndCommandBuffer();

	LockQueue();
	VkResult  result = vkQueueSubmit(GetVkQueue(), 1, &theSubmitInfo, currentCommandBuffer->GetCompletionFence());
	UnlockQueue();

	mCommandPool->NextCmdBuffer();

	if (result != VK_SUCCESS)
	{
		EngineLog("Queue submission failed.");
	}

	AddToPresentQueue(nextPresentable);
}





void VulkanDevice::TakeInput(unsigned int keyPress)
{
	if (keyPress == 65)
		mRenderInstance->ChangeWorldPosition(1.0f, 0.0f, 0.0f);

	if (keyPress == 66)
		mRenderInstance->ChangeWorldPosition(-1.0f, 0.0f, 0.0f);

	if (keyPress == 67)
		mRenderInstance->ChangeWorldPosition(0.0f, 0.0f, 1.0f);

	if (keyPress == 68)
		mRenderInstance->ChangeWorldPosition(0.0f, 0.0f, -1.0f);

	if (keyPress == 37)
		mRenderInstance->ChangeWorldPosition(0.0f, -1.0f, 0.0f);

	if (keyPress == 39)
		mRenderInstance->ChangeWorldPosition(0.0f, 1.0f, 0.0f);
}





void VulkanDevice::DoRender(uint32_t nextImage)
{

		mRenderInstance->Draw(*mRenderPasses[nextImage]);

		//mRenderInstance2->Draw(*mRenderPasses[nextImage]);
}