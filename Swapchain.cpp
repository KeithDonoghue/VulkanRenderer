#include "Swapchain.h"


#define ENGINE_LOGGING_ENABLED 1 
#include "EngineLogging.h"


#include "Windows.h"

Swapchain::Swapchain(VulkanDevice * theDevice, EngineWindow * theWindow):
	mDevice(theDevice),
	mWindow(theWindow)
{ 

	VkExtent2D WindowSize;
	memset(&WindowSize, 0, sizeof(VkExtent2D));
	WindowSize.width = 400;
	WindowSize.height = 400;


	memset(&mCreateInfo, 0, sizeof(mCreateInfo));
	mCreateInfo.sType				= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	mCreateInfo.pNext				= nullptr;
	//mCreateInfo.flags				= 0; reserved for future use.
	mCreateInfo.surface				= NULL; // assigned in init.
	mCreateInfo.minImageCount		= 2;
	mCreateInfo.imageFormat			= VK_FORMAT_B8G8R8A8_UNORM;
	mCreateInfo.imageColorSpace		= VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	mCreateInfo.imageExtent			= WindowSize;
	mCreateInfo.imageArrayLayers	= 1;
	mCreateInfo.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	mCreateInfo.imageSharingMode	= VK_SHARING_MODE_EXCLUSIVE;
	//mCreateInfo.queueFamilyIndexCount 
	//mCreateInfo.pQueueFamilyIndices
	mCreateInfo.preTransform		= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	mCreateInfo.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	mCreateInfo.presentMode			= VK_PRESENT_MODE_FIFO_KHR;

	mCreateInfo.clipped				= VK_TRUE;
	mCreateInfo.oldSwapchain		= VK_NULL_HANDLE;

	init();
}




Swapchain::Swapchain(VulkanDevice * theDevice, EngineWindow * theWindow, VkSwapchainCreateInfoKHR theCreateInfo):
	mDevice(theDevice),
	mWindow(theWindow),
	mCreateInfo(theCreateInfo)
{
	init();
}




Swapchain::~Swapchain()
{

	vkDestroySemaphore(mDevice->GetVkDevice(), mWaitForAquireSemaphore, nullptr);
	vkDestroySemaphore(mDevice->GetVkDevice(), mWaitForPresentSemaphore, nullptr);

	vkDestroySwapchainKHR(mDevice->GetVkDevice(), theVulkanSwapchain, nullptr);
}



void Swapchain::init()
{

	mCreateInfo.surface = mWindow->GetSurface();

	VkResult result = vkCreateSwapchainKHR(mDevice->GetVkDevice(), &mCreateInfo, nullptr, &theVulkanSwapchain);

	if (result != VK_SUCCESS)
	{
		EngineLog("Hello");
	}
	else
	{
		EngineLog("success!");
	}

	GetImages();



	VkSemaphoreCreateInfo SemaphoreCreateInfo;
	memset(&SemaphoreCreateInfo, 0, sizeof(SemaphoreCreateInfo));

	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreCreateInfo.pNext = nullptr;
	SemaphoreCreateInfo.flags = 0; //reserved for future use.


	result = vkCreateSemaphore(mDevice->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &mWaitForAquireSemaphore);


	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to create semaphore");
	}


	result = vkCreateSemaphore(mDevice->GetVkDevice(), &SemaphoreCreateInfo, nullptr, &mWaitForPresentSemaphore);


	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to create semaphore");
	}

}





void Swapchain::GetImages()
{
	uint32_t SwapchainImageCount;
	VkResult result = vkGetSwapchainImagesKHR(mDevice->GetVkDevice(), theVulkanSwapchain, &SwapchainImageCount, nullptr);

	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to get Images");
	}

	VkImage *  SwpachainImages = (VkImage*) malloc(SwapchainImageCount* sizeof(VkImage));
	result = vkGetSwapchainImagesKHR(mDevice->GetVkDevice(), theVulkanSwapchain, &SwapchainImageCount, SwpachainImages);

	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to get Images");
	}
	else
	{
		mDevice->PopulatePresentableImages(SwpachainImages, SwapchainImageCount);
	}

}





void Swapchain::Update()
{


	uint32_t presentIndex;
	VkResult result = vkAcquireNextImageKHR(mDevice->GetVkDevice(), 
		theVulkanSwapchain, 
		UINT64_MAX, 
		mWaitForAquireSemaphore, 
		VK_NULL_HANDLE, 
		&presentIndex);

	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to acquire image");
	}

	mDevice->AddPresentableIndex(presentIndex);

	mDevice->GetNextPresentable(&mWaitForAquireSemaphore, &mWaitForPresentSemaphore);

	VkPresentInfoKHR thePresentInfo;
	memset(&thePresentInfo, 0, sizeof(VkPresentInfoKHR));
	thePresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	thePresentInfo.pNext = nullptr;
	thePresentInfo.waitSemaphoreCount = 1;
	thePresentInfo.pWaitSemaphores = &mWaitForPresentSemaphore;
	thePresentInfo.swapchainCount = 1;
	thePresentInfo.pSwapchains = &theVulkanSwapchain;
	thePresentInfo.pImageIndices = &presentIndex;

	result = vkQueuePresentKHR(mDevice->GetVkQueue(), &thePresentInfo);


	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to present Image");
	}

	
	result = vkQueueWaitIdle(mDevice->GetVkQueue());

	if (result != VK_SUCCESS)
	{
		EngineLog("Wait Idle failed.");
	}
}