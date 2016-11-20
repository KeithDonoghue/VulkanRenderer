#include "CommandPool.h"
#include "CommandBuffer.h"

#define ENGINE_LOGGING_ENABLED 1 
#include "EngineLogging.h"
#include "VulkanDevice.h"

#include <Vulkan/vulkan.h>

#include <cstring>

CommandPool::CommandPool(VulkanDevice* theVulkanDevice):
mDevice(theVulkanDevice)
{
	memset(&mCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
	mCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	mCreateInfo.pNext = nullptr;
	mCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	mCreateInfo.queueFamilyIndex = 0;

	VkResult result = vkCreateCommandPool(mDevice->GetVkDevice(), &mCreateInfo, nullptr, &m_TheVulkanCommandPool);
	if (result != VK_SUCCESS)
	{
		EngineLog("Failed to create command pool")
	}

	CreateCommandBuffer();
}





CommandPool::~CommandPool()
{
	vkDestroyCommandPool(mDevice->GetVkDevice(), m_TheVulkanCommandPool, nullptr);
}





void CommandPool::CreateCommandBuffer()
{
	mCurrentCommandBuffer = new CommandBuffer(this);
}