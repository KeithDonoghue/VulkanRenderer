#ifndef VULKAN_IMAGE_HDR
#define VULKAN_IMAGE_HDR 1

#include "vulkan/vulkan.h"

#include "VulkanMemoryManager.h"

#include <vector>

class VulkanDevice;

class VulkanImage
{
public:
	VulkanImage(VkImage, bool =  false);
	VulkanImage(VulkanDevice*, int width, int heigh);
	~VulkanImage();
	VkImage GetVkImage() { return m_TheVulkanImage; }
	VkImageLayout GetLayout() { return mCurrentLayout; }
	void SetLayout(VkImageLayout layout) { mCurrentLayout = layout; }
	bool IsDirty(){ return mDirty; }

	void LoadDataToImage();

private:
	VkImage m_TheVulkanImage;
	VkImageCreateInfo mCreateInfo;
	VkImageLayout mCurrentLayout;

	VulkanDevice * mDevice;
	bool mSystemManaged;
	allocation mAllocStruct;
	bool mDirty;
};

#endif // VULKAN_IMAGE_HDR