///*
//* Vulkan Example - Omni directional shadows using a dynamic cube map
//*
//* Copyright (C) by Sascha Willems - www.saschawillems.de
//*
//* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
//*/
//
//#include "vulkanexamplebase.h"
//#include "VulkanglTFModel.h"
//
//#define ENABLE_VALIDATION false
//
//// Texture properties
//#define TEX_DIM 1024
//#define TEX_FILTER VK_FILTER_LINEAR
//
//// Offscreen frame buffer properties
//#define FB_DIM TEX_DIM
//#define FB_COLOR_FORMAT VK_FORMAT_R32_SFLOAT
//
//class VulkanExample : public VulkanExampleBase
//{
//public:
//	bool displayCubeMap = false;
//
//	float zNear = 0.1f;
//	float zFar = 1024.0f;
//
//	struct {
//		vkglTF::Model scene;
//		vkglTF::Model debugcube;
//	} models;
//
//	struct {
//		vks::Buffer scene;
//		vks::Buffer offscreen;
//	} uniformBuffers;
//
//	struct {
//		glm::mat4 projection;
//		glm::mat4 model;
//	} uboVSquad;
//
//	glm::vec4 lightPos = glm::vec4(0.0f, -2.5f, 0.0f, 1.0);
//
//	struct UBO {
//		glm::mat4 projection;
//		glm::mat4 view;
//		glm::mat4 model;
//		glm::vec4 lightPos;
//	};
//
//	UBO uboVSscene, uboOffscreenVS;
//
//	struct {
//		VkPipeline scene;
//		VkPipeline offscreen;
//		VkPipeline cubemapDisplay;
//	} pipelines;
//
//	struct {
//		VkPipelineLayout scene;
//		VkPipelineLayout offscreen;
//	} pipelineLayouts;
//
//	struct {
//		VkDescriptorSet scene;
//		VkDescriptorSet offscreen;
//	} descriptorSets;
//
//	VkDescriptorSetLayout descriptorSetLayout;
//
//	vks::Texture shadowCubeMap;
//	std::array<VkImageView, 6> shadowCubeMapFaceImageViews;
//
//	// Framebuffer for offscreen rendering
//	struct FrameBufferAttachment {
//		VkImage image;
//		VkDeviceMemory mem;
//		VkImageView view;
//	};
//	struct OffscreenPass {
//		int32_t width, height;
//		std::array<VkFramebuffer, 6> frameBuffers;
//		FrameBufferAttachment depth;
//		VkRenderPass renderPass;
//		VkSampler sampler;
//		VkDescriptorImageInfo descriptor;
//	} offscreenPass;
//
//	VkFormat fbDepthFormat;
//
//	VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
//	{
//		title = "Point light shadows (cubemap)";
//		camera.type = Camera::CameraType::lookat;
//		camera.setPerspective(45.0f, (float)width / (float)height, zNear, zFar);
//		camera.setRotation(glm::vec3(-20.5f, -673.0f, 0.0f));
//		camera.setPosition(glm::vec3(0.0f, 0.5f, -15.0f));
//		timerSpeed *= 0.5f;
//	}
//
//	~VulkanExample()
//	{
//		// Clean up used Vulkan resources
//		// Note : Inherited destructor cleans up resources stored in base class
//
//		// Cube map
//		for (uint32_t i = 0; i < 6; i++)
//		{
//			vkDestroyImageView(device, shadowCubeMapFaceImageViews[i], nullptr);
//		}
//
//		vkDestroyImageView(device, shadowCubeMap.view, nullptr);
//		vkDestroyImage(device, shadowCubeMap.image, nullptr);
//		vkDestroySampler(device, shadowCubeMap.sampler, nullptr);
//		vkFreeMemory(device, shadowCubeMap.deviceMemory, nullptr);
//
//		// Depth attachment
//		vkDestroyImageView(device, offscreenPass.depth.view, nullptr);
//		vkDestroyImage(device, offscreenPass.depth.image, nullptr);
//		vkFreeMemory(device, offscreenPass.depth.mem, nullptr);
//
//		for (uint32_t i = 0; i < 6; i++)
//		{
//			vkDestroyFramebuffer(device, offscreenPass.frameBuffers[i], nullptr);
//		}
//
//		vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);
//
//		// Pipelines
//		vkDestroyPipeline(device, pipelines.scene, nullptr);
//		vkDestroyPipeline(device, pipelines.offscreen, nullptr);
//		vkDestroyPipeline(device, pipelines.cubemapDisplay, nullptr);
//
//		vkDestroyPipelineLayout(device, pipelineLayouts.scene, nullptr);
//		vkDestroyPipelineLayout(device, pipelineLayouts.offscreen, nullptr);
//
//		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
//
//		// Uniform buffers
//		uniformBuffers.offscreen.destroy();
//		uniformBuffers.scene.destroy();
//	}
//
//	void prepareCubeMap()
//	{
//		shadowCubeMap.width = TEX_DIM;
//		shadowCubeMap.height = TEX_DIM;
//
//		// 32 bit float format for higher precision
//		VkFormat format = VK_FORMAT_R32_SFLOAT;
//
//		// Cube map image description
//		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
//		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//		imageCreateInfo.format = format;
//		imageCreateInfo.extent = { shadowCubeMap.width, shadowCubeMap.height, 1 };
//		imageCreateInfo.mipLevels = 1;
//		imageCreateInfo.arrayLayers = 6;
//		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//		imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
//
//		VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
//		VkMemoryRequirements memReqs;
//
//		VkCommandBuffer layoutCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//
//		// Create cube map image
//		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &shadowCubeMap.image));
//
//		vkGetImageMemoryRequirements(device, shadowCubeMap.image, &memReqs);
//
//		memAllocInfo.allocationSize = memReqs.size;
//		memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &shadowCubeMap.deviceMemory));
//		VK_CHECK_RESULT(vkBindImageMemory(device, shadowCubeMap.image, shadowCubeMap.deviceMemory, 0));
//
//		// Image barrier for optimal image (target)
//		VkImageSubresourceRange subresourceRange = {};
//		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		subresourceRange.baseMipLevel = 0;
//		subresourceRange.levelCount = 1;
//		subresourceRange.layerCount = 6;
//		vks::tools::setImageLayout(
//			layoutCmd,
//			shadowCubeMap.image,
//			VK_IMAGE_LAYOUT_UNDEFINED,
//			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//			subresourceRange);
//
//		vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);
//
//		// Create sampler
//		VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
//		sampler.magFilter = TEX_FILTER;
//		sampler.minFilter = TEX_FILTER;
//		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
//		sampler.addressModeV = sampler.addressModeU;
//		sampler.addressModeW = sampler.addressModeU;
//		sampler.mipLodBias = 0.0f;
//		sampler.maxAnisotropy = 1.0f;
//		sampler.compareOp = VK_COMPARE_OP_NEVER;
//		sampler.minLod = 0.0f;
//		sampler.maxLod = 1.0f;
//		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
//		VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &shadowCubeMap.sampler));
//
//		// Create image view
//		VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
//		view.image = VK_NULL_HANDLE;
//		view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
//		view.format = format;
//		view.components = { VK_COMPONENT_SWIZZLE_R };
//		view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
//		view.subresourceRange.layerCount = 6;
//		view.image = shadowCubeMap.image;
//		VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &shadowCubeMap.view));
//
//		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		view.subresourceRange.layerCount = 1;
//		view.image = shadowCubeMap.image;
//
//		for (uint32_t i = 0; i < 6; i++)
//		{
//			view.subresourceRange.baseArrayLayer = i;
//			VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &shadowCubeMapFaceImageViews[i]));
//		}
//	}
//
//	// Prepare a new framebuffer for offscreen rendering
//	// The contents of this framebuffer are then
//	// copied to the different cube map faces
//	void prepareOffscreenFramebuffer()
//	{
//		offscreenPass.width = FB_DIM;
//		offscreenPass.height = FB_DIM;
//
//		VkFormat fbColorFormat = FB_COLOR_FORMAT;
//
//		// Color attachment
//		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
//		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//		imageCreateInfo.format = fbColorFormat;
//		imageCreateInfo.extent.width = offscreenPass.width;
//		imageCreateInfo.extent.height = offscreenPass.height;
//		imageCreateInfo.extent.depth = 1;
//		imageCreateInfo.mipLevels = 1;
//		imageCreateInfo.arrayLayers = 1;
//		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//		// Image of the framebuffer is blit source
//		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//		imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
//		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//		VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
//		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		colorImageView.format = fbColorFormat;
//		colorImageView.flags = 0;
//		colorImageView.subresourceRange = {};
//		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		colorImageView.subresourceRange.baseMipLevel = 0;
//		colorImageView.subresourceRange.levelCount = 1;
//		colorImageView.subresourceRange.baseArrayLayer = 0;
//		colorImageView.subresourceRange.layerCount = 1;
//
//		VkCommandBuffer layoutCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//
//		// Depth stencil attachment
//		imageCreateInfo.format = fbDepthFormat;
//		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
//
//		VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
//		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		depthStencilView.format = fbDepthFormat;
//		depthStencilView.flags = 0;
//		depthStencilView.subresourceRange = {};
//		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//		if (fbDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
//			depthStencilView.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//		depthStencilView.subresourceRange.baseMipLevel = 0;
//		depthStencilView.subresourceRange.levelCount = 1;
//		depthStencilView.subresourceRange.baseArrayLayer = 0;
//		depthStencilView.subresourceRange.layerCount = 1;
//
//		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &offscreenPass.depth.image));
//
//		VkMemoryRequirements memReqs;
//		vkGetImageMemoryRequirements(device, offscreenPass.depth.image, &memReqs);
//		
//		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
//		memAlloc.allocationSize = memReqs.size;
//		memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//		VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.depth.mem));
//		VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPass.depth.image, offscreenPass.depth.mem, 0));
//
//		vks::tools::setImageLayout(
//			layoutCmd,
//			offscreenPass.depth.image,
//			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
//			VK_IMAGE_LAYOUT_UNDEFINED,
//			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//
//		vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);
//
//		depthStencilView.image = offscreenPass.depth.image;
//		VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPass.depth.view));
//
//		VkImageView attachments[2];
//		attachments[1] = offscreenPass.depth.view;
//
//		VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
//		fbufCreateInfo.renderPass = offscreenPass.renderPass;
//		fbufCreateInfo.attachmentCount = 2;
//		fbufCreateInfo.pAttachments = attachments;
//		fbufCreateInfo.width = offscreenPass.width;
//		fbufCreateInfo.height = offscreenPass.height;
//		fbufCreateInfo.layers = 1;
//
//		for (uint32_t i = 0; i < 6; i++)
//		{
//			attachments[0] = shadowCubeMapFaceImageViews[i];
//			VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offscreenPass.frameBuffers[i]));
//		}
//	}
//
//	// Updates a single cube map face
//	// Renders the scene with face's view directly to the cubemap layer `faceIndex`
//	// Uses push constants for quick update of view matrix for the current cube map face
//	void updateCubeFace(uint32_t faceIndex, VkCommandBuffer commandBuffer)
//	{
//		VkClearValue clearValues[2];
//		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
//		clearValues[1].depthStencil = { 1.0f, 0 };
//
//		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
//		// Reuse render pass from example pass
//		renderPassBeginInfo.renderPass = offscreenPass.renderPass;
//		renderPassBeginInfo.framebuffer = offscreenPass.frameBuffers[faceIndex];
//		renderPassBeginInfo.renderArea.extent.width = offscreenPass.width;
//		renderPassBeginInfo.renderArea.extent.height = offscreenPass.height;
//		renderPassBeginInfo.clearValueCount = 2;
//		renderPassBeginInfo.pClearValues = clearValues;
//
//		// Update view matrix via push constant
//
//		glm::mat4 viewMatrix = glm::mat4(1.0f);
//		switch (faceIndex)
//		{
//		case 0: // POSITIVE_X
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//			break;
//		case 1:	// NEGATIVE_X
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//			break;
//		case 2:	// POSITIVE_Y
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//			break;
//		case 3:	// NEGATIVE_Y
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//			break;
//		case 4:	// POSITIVE_Z
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//			break;
//		case 5:	// NEGATIVE_Z
//			viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//			break;
//		}
//
//		// Render scene from cube face's point of view
//		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//		// Update shader push constant block
//		// Contains current face view matrix
//		vkCmdPushConstants(
//			commandBuffer,
//			pipelineLayouts.offscreen,
//			VK_SHADER_STAGE_VERTEX_BIT,
//			0,
//			sizeof(glm::mat4),
//			&viewMatrix);
//
//		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.offscreen);
//		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.offscreen, 0, 1, &descriptorSets.offscreen, 0, NULL);
//		models.scene.draw(commandBuffer);
//
//		vkCmdEndRenderPass(commandBuffer);
//	}
//
//	void buildCommandBuffers()
//	{
//		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
//
//		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
//		{
//			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));
//
//			/*
//				Generate shadow cube maps using one render pass per face
//			*/
//			{
//				VkViewport viewport = vks::initializers::viewport((float)offscreenPass.width, (float)offscreenPass.height, 0.0f, 1.0f);
//				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
//
//				VkRect2D scissor = vks::initializers::rect2D(offscreenPass.width, offscreenPass.height, 0, 0);
//				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
//
//				for (uint32_t face = 0; face < 6; face++) {
//					updateCubeFace(face, drawCmdBuffers[i]);
//				}
//			}
//
//			/*
//				Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies
//			*/
//
//			/*
//				Scene rendering with applied shadow map
//			*/
//			{
//				VkClearValue clearValues[2];
//				clearValues[0].color = {1, 1, 1};// defaultClearColor;
//				clearValues[1].depthStencil = { 1.0f, 0 };
//
//				VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
//				renderPassBeginInfo.renderPass = renderPass;
//				renderPassBeginInfo.framebuffer = frameBuffers[i];
//				renderPassBeginInfo.renderArea.extent.width = width;
//				renderPassBeginInfo.renderArea.extent.height = height;
//				renderPassBeginInfo.clearValueCount = 2;
//				renderPassBeginInfo.pClearValues = clearValues;
//
//				vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//				VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
//				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
//
//				VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
//				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
//
//				vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.scene, 0, 1, &descriptorSets.scene, 0, NULL);
//
//				if (displayCubeMap)
//				{
//					// Display all six sides of the shadow cube map
//					// Note: Visualization of the different faces is done in the fragment shader, see cubemapdisplay.frag
//					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.cubemapDisplay);
//					models.debugcube.draw(drawCmdBuffers[i]);
//				}
//				else
//				{
//					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.scene);
//					models.scene.draw(drawCmdBuffers[i]);
//				}
//
//				drawUI(drawCmdBuffers[i]);
//
//				vkCmdEndRenderPass(drawCmdBuffers[i]);
//			}
//
//			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
//		}
//	}
//
//	void loadAssets()
//	{
//		const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
//		models.debugcube.loadFromFile(getAssetPath() + "models/cube.gltf", vulkanDevice, queue, glTFLoadingFlags);
//		models.scene.loadFromFile(getAssetPath() + "models/fractal.gltf", vulkanDevice, queue, glTFLoadingFlags);
//	}
//
//	void setupDescriptorPool()
//	{
//		// Example uses three ubos and two image samplers
//		std::vector<VkDescriptorPoolSize> poolSizes = {
//			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3),
//			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
//		};
//		VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 3);
//		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
//	}
//
//	void setupDescriptorSetLayout()
//	{
//		// Shared pipeline layout
//		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
//			// Binding 0 : Vertex shader uniform buffer
//			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
//			// Binding 1 : Fragment shader image sampler (cube map)
//			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
//		};
//
//		VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
//		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));
//
//		// 3D scene pipeline layout
//		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
//		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.scene));
//
//		// Offscreen pipeline layout
//		// Push constants for cube map face view matrices
//		VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
//		// Push constant ranges are part of the pipeline layout
//		pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
//		pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
//		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.offscreen));
//	}
//
//	void setupDescriptorSets()
//	{
//		VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
//
//		// 3D scene
//		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets.scene));
//		// Image descriptor for the cube map
//		VkDescriptorImageInfo texDescriptor =
//			vks::initializers::descriptorImageInfo(
//				shadowCubeMap.sampler,
//				shadowCubeMap.view,
//				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//
//		std::vector<VkWriteDescriptorSet> sceneDescriptorSets = {
//			// Binding 0 : Vertex shader uniform buffer
//			vks::initializers::writeDescriptorSet(descriptorSets.scene, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.scene.descriptor),
//			// Binding 1 : Fragment shader shadow sampler
//			vks::initializers::writeDescriptorSet(descriptorSets.scene, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptor)
//		};
//		vkUpdateDescriptorSets(device, sceneDescriptorSets.size(), sceneDescriptorSets.data(), 0, NULL);
//
//		// Offscreen
//		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets.offscreen));
//		std::vector<VkWriteDescriptorSet> offScreenWriteDescriptorSets = {
//			// Binding 0 : Vertex shader uniform buffer
//			vks::initializers::writeDescriptorSet(descriptorSets.offscreen, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.offscreen.descriptor),
//		};
//		vkUpdateDescriptorSets(device, offScreenWriteDescriptorSets.size(), offScreenWriteDescriptorSets.data(), 0, NULL);
//	}
//
//	// Set up a separate render pass for the offscreen frame buffer
//	// This is necessary as the offscreen frame buffer attachments
//	// use formats different to the ones from the visible frame buffer
//	// and at least the depth one may not be compatible
//	void prepareOffscreenRenderpass()
//	{
//		VkAttachmentDescription osAttachments[2] = {};
//
//		// Find a suitable depth format
//		VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &fbDepthFormat);
//		assert(validDepthFormat);
//
//		osAttachments[0].format = FB_COLOR_FORMAT;
//		osAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
//		osAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//		osAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//		osAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//		osAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//		osAttachments[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//		osAttachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//
//		// Depth attachment
//		osAttachments[1].format = fbDepthFormat;
//		osAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
//		osAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//		osAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//		osAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//		osAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//		osAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//		osAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//		VkAttachmentReference colorReference = {};
//		colorReference.attachment = 0;
//		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//		VkAttachmentReference depthReference = {};
//		depthReference.attachment = 1;
//		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//		VkSubpassDescription subpass = {};
//		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//		subpass.colorAttachmentCount = 1;
//		subpass.pColorAttachments = &colorReference;
//		subpass.pDepthStencilAttachment = &depthReference;
//
//		VkRenderPassCreateInfo renderPassCreateInfo = vks::initializers::renderPassCreateInfo();
//		renderPassCreateInfo.attachmentCount = 2;
//		renderPassCreateInfo.pAttachments = osAttachments;
//		renderPassCreateInfo.subpassCount = 1;
//		renderPassCreateInfo.pSubpasses = &subpass;
//
//		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &offscreenPass.renderPass));
//	}
//
//	void preparePipelines()
//	{
//		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
//		VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
//		VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
//		VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
//		VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
//		VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
//		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
//		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
//		VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), dynamicStateEnables.size(), 0);
//
//		// 3D scene pipeline
//		// Load shaders
//		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
//
//		shaderStages[0] = loadShader(getShadersPath() + "shadowmappingomni/scene.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//		shaderStages[1] = loadShader(getShadersPath() + "shadowmappingomni/scene.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
//
//		VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayouts.scene, renderPass, 0);
//		pipelineCI.pInputAssemblyState = &inputAssemblyState;
//		pipelineCI.pRasterizationState = &rasterizationState;
//		pipelineCI.pColorBlendState = &colorBlendState;
//		pipelineCI.pMultisampleState = &multisampleState;
//		pipelineCI.pViewportState = &viewportState;
//		pipelineCI.pDepthStencilState = &depthStencilState;
//		pipelineCI.pDynamicState = &dynamicState;
//		pipelineCI.stageCount = shaderStages.size();
//		pipelineCI.pStages = shaderStages.data();
//		pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Color, vkglTF::VertexComponent::Normal});
//		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.scene));
//
//		// Offscreen pipeline
//		shaderStages[0] = loadShader(getShadersPath() + "shadowmappingomni/offscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//		shaderStages[1] = loadShader(getShadersPath() + "shadowmappingomni/offscreen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
//		pipelineCI.layout = pipelineLayouts.offscreen;
//		pipelineCI.renderPass = offscreenPass.renderPass;
//		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.offscreen));
//
//		// Cube map display pipeline
//		shaderStages[0] = loadShader(getShadersPath() + "shadowmappingomni/cubemapdisplay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//		shaderStages[1] = loadShader(getShadersPath() + "shadowmappingomni/cubemapdisplay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
//		VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
//		pipelineCI.pVertexInputState = &emptyInputState;
//		pipelineCI.layout = pipelineLayouts.scene;
//		pipelineCI.renderPass = renderPass;
//		rasterizationState.cullMode = VK_CULL_MODE_NONE;
//		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.cubemapDisplay));
//	}
//
//	// Prepare and initialize uniform buffer containing shader uniforms
//	void prepareUniformBuffers()
//	{
//		// Offscreen vertex shader uniform buffer
//		VK_CHECK_RESULT(vulkanDevice->createBuffer(
//			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//			&uniformBuffers.offscreen,
//			sizeof(uboOffscreenVS)));
//
//		// Scene vertex shader uniform buffer
//		VK_CHECK_RESULT(vulkanDevice->createBuffer(
//			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//			&uniformBuffers.scene,
//			sizeof(uboVSscene)));
//
//		// Map persistent
//		VK_CHECK_RESULT(uniformBuffers.offscreen.map());
//		VK_CHECK_RESULT(uniformBuffers.scene.map());
//
//		updateUniformBufferOffscreen();
//		updateUniformBuffers();
//	}
//
//	void updateUniformBuffers()
//	{
//		uboVSscene.projection = camera.matrices.perspective;
//		uboVSscene.view = camera.matrices.view;
//		uboVSscene.model = glm::mat4(1.0f);
//		uboVSscene.lightPos = lightPos;
//		memcpy(uniformBuffers.scene.mapped, &uboVSscene, sizeof(uboVSscene));
//	}
//
//	void updateUniformBufferOffscreen()
//	{
//		//lightPos.y -= 0.0001f;
//
//		uboOffscreenVS.projection = glm::perspective((float)(M_PI / 2.0), 1.0f, zNear, zFar);
//		uboOffscreenVS.view = glm::mat4(1.0f);
//		uboOffscreenVS.model = glm::translate(glm::mat4(1.0f), glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));
//		uboOffscreenVS.lightPos = lightPos;
//		memcpy(uniformBuffers.offscreen.mapped, &uboOffscreenVS, sizeof(uboOffscreenVS));
//	}
//
//	void draw()
//	{
//		VulkanExampleBase::prepareFrame();
//		submitInfo.commandBufferCount = 1;
//		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
//		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
//		VulkanExampleBase::submitFrame();
//	}
//
//	void prepare()
//	{
//		VulkanExampleBase::prepare();
//		loadAssets();
//		prepareUniformBuffers();
//		prepareCubeMap();
//		setupDescriptorSetLayout();
//		prepareOffscreenRenderpass();
//		preparePipelines();
//		setupDescriptorPool();
//		setupDescriptorSets();
//		prepareOffscreenFramebuffer();
//		buildCommandBuffers();
//		prepared = true;
//	}
//
//	virtual void render()
//	{
//		if (!prepared)
//			return;
//		draw();
//		if (!paused || camera.updated)
//		{
//			updateUniformBufferOffscreen();
//			updateUniformBuffers();
//		}
//	}
//
//	virtual void OnUpdateUIOverlay(vks::UIOverlay *overlay)
//	{
//		if (overlay->header("Settings")) {
//			if (overlay->checkBox("Display shadow cube render target", &displayCubeMap)) {
//				buildCommandBuffers();
//			}
//		}
//	}
//};
//
//VULKAN_EXAMPLE_MAIN()


/*
* Vulkan Example - Hardware accelerated ray tracing shadow example
*
* Renders a complex scene using multiple hit and miss shaders for implementing shadows
*
* Copyright (C) by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "VulkanRaytracingSample.h"
#include "VulkanglTFModel.h"

class VulkanExample : public VulkanRaytracingSample
{
public:
	AccelerationStructure bottomLevelAS;
	AccelerationStructure topLevelAS;

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
	struct ShaderBindingTables {
		ShaderBindingTable raygen;
		ShaderBindingTable miss;
		ShaderBindingTable hit;
	} shaderBindingTables;

	struct UniformData {
		glm::mat4 viewInverse;
		glm::mat4 projInverse;
		glm::vec4 lightPos;
		int32_t vertexSize;
	} uniformData;
	vks::Buffer ubo;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	vkglTF::Model scene;

	// This sample is derived from an extended base class that saves most of the ray tracing setup boiler plate
	VulkanExample() : VulkanRaytracingSample()
	{
		title = "Ray traced shadows";
		timerSpeed *= 0.25f;
		camera.type = Camera::CameraType::lookat;
		camera.setPerspective(45.0f, (float)width / (float)height, 0.1f, 512.0f);
		camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		camera.setTranslation(glm::vec3(0.0f, 0.0f, -10.0f));
		enableExtensions();
	}

	~VulkanExample()
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		deleteStorageImage();
		deleteAccelerationStructure(bottomLevelAS);
		deleteAccelerationStructure(topLevelAS);
		shaderBindingTables.raygen.destroy();
		shaderBindingTables.miss.destroy();
		shaderBindingTables.hit.destroy();
		ubo.destroy();
	}

	/*
		Create the bottom level acceleration structure contains the scene's actual geometry (vertices, triangles)
	*/
	void createBottomLevelAccelerationStructure()
	{
		// Instead of a simple triangle, we'll be loading a more complex scene for this example
		// The shaders are accessing the vertex and index buffers of the scene, so the proper usage flag has to be set on the vertex and index buffers for the scene
		vkglTF::memoryPropertyFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
		scene.loadFromFile(getAssetPath() + "models/fractal.gltf", vulkanDevice, queue, glTFLoadingFlags);

		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

		vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(scene.vertices.buffer);
		indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(scene.indices.buffer);

		uint32_t numTriangles = static_cast<uint32_t>(scene.indices.count) / 3;
		uint32_t maxVertex = scene.vertices.count;

		// Build
		VkAccelerationStructureGeometryKHR accelerationStructureGeometry = vks::initializers::accelerationStructureGeometryKHR();
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.maxVertex = maxVertex;
		accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(vkglTF::Vertex);
		accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
		accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = vks::initializers::accelerationStructureBuildGeometryInfoKHR();
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = vks::initializers::accelerationStructureBuildSizesInfoKHR();
		vkGetAccelerationStructureBuildSizesKHR(
			device,
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&numTriangles,
			&accelerationStructureBuildSizesInfo);

		createAccelerationStructure(bottomLevelAS, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, accelerationStructureBuildSizesInfo);

		// Create a small scratch buffer used during build of the bottom level acceleration structure
		ScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo = vks::initializers::accelerationStructureBuildGeometryInfoKHR();
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		// Build the acceleration structure on the device via a one-time command buffer submission
		// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
		VkCommandBuffer commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
		vulkanDevice->flushCommandBuffer(commandBuffer, queue);

		deleteScratchBuffer(scratchBuffer);
	}

	/*
		The top level acceleration structure contains the scene's object instances
	*/
	void createTopLevelAccelerationStructure()
	{
		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transformMatrix;
		instance.instanceCustomIndex = 0;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = bottomLevelAS.deviceAddress;

		// Buffer for instance data
		vks::Buffer instancesBuffer;
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&instancesBuffer,
			sizeof(VkAccelerationStructureInstanceKHR),
			&instance));

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(instancesBuffer.buffer);

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry = vks::initializers::accelerationStructureGeometryKHR();
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
		accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = vks::initializers::accelerationStructureBuildGeometryInfoKHR();
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		uint32_t primitive_count = 1;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = vks::initializers::accelerationStructureBuildSizesInfoKHR();
		vkGetAccelerationStructureBuildSizesKHR(
			device,
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&primitive_count,
			&accelerationStructureBuildSizesInfo);

		// @todo: as return value?
		createAccelerationStructure(topLevelAS, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfo);

		// Create a small scratch buffer used during build of the top level acceleration structure
		ScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo = vks::initializers::accelerationStructureBuildGeometryInfoKHR();
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = 1;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		// Build the acceleration structure on the device via a one-time command buffer submission
		// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
		VkCommandBuffer commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
		vulkanDevice->flushCommandBuffer(commandBuffer, queue);

		deleteScratchBuffer(scratchBuffer);
		instancesBuffer.destroy();
	}


	/*
		Create the Shader Binding Tables that binds the programs and top-level acceleration structure

		SBT Layout used in this sample:

			/-----------\
			| raygen    |
			|-----------|
			| miss      |
			|-----------|
			| hit       |
			\-----------/

	*/
	void createShaderBindingTables() {
		const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
		const uint32_t handleSizeAligned = vks::tools::alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
		const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(device, pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));

		createShaderBindingTable(shaderBindingTables.raygen, 1);
		// We are using two miss shaders
		createShaderBindingTable(shaderBindingTables.miss, 2);
		createShaderBindingTable(shaderBindingTables.hit, 1);

		// Copy handles
		memcpy(shaderBindingTables.raygen.mapped, shaderHandleStorage.data(), handleSize);
		// We are using two miss shaders, so we need to get two handles for the miss shader binding table
		memcpy(shaderBindingTables.miss.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize * 2);
		memcpy(shaderBindingTables.hit.mapped, shaderHandleStorage.data() + handleSizeAligned * 3, handleSize);
	}

	/*
		Create the descriptor sets used for the ray tracing dispatch
	*/
	void createDescriptorSets()
	{
		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 }
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 1);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo = vks::initializers::writeDescriptorSetAccelerationStructureKHR();
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.handle;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = descriptorSet;
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, storageImage.view, VK_IMAGE_LAYOUT_GENERAL };
		VkDescriptorBufferInfo vertexBufferDescriptor{ scene.vertices.buffer, 0, VK_WHOLE_SIZE };
		VkDescriptorBufferInfo indexBufferDescriptor{ scene.indices.buffer, 0, VK_WHOLE_SIZE };

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			// Binding 0: Top level acceleration structure
			accelerationStructureWrite,
			// Binding 1: Ray tracing result image
			vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor),
			// Binding 2: Uniform data
			vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo.descriptor),
			// Binding 3: Scene vertex buffer
			vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &vertexBufferDescriptor),
			// Binding 4: Scene index buffer
			vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &indexBufferDescriptor),
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	}

	/*
		Create our ray tracing pipeline
	*/
	void createRayTracingPipeline()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// Binding 0: Acceleration structure
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0),
			// Binding 1: Storage image
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1),
			// Binding 2: Uniform buffer
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, 2),
			// Binding 3: Vertex buffer 
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 3),
			// Binding 4: Index buffer
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 4),
		};

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCI, nullptr, &pipelineLayout));

		/*
			Setup ray tracing shader groups
		*/
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		// Ray generation group
		{
			shaderStages.push_back(loadShader(getShadersPath() + "raytracingshadows/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR));
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			shaderGroups.push_back(shaderGroup);
		}

		// Miss group
		{
			shaderStages.push_back(loadShader(getShadersPath() + "raytracingshadows/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			shaderGroups.push_back(shaderGroup);
			// Second shader for shadows
			shaderStages.push_back(loadShader(getShadersPath() + "raytracingshadows/shadow.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroups.push_back(shaderGroup);
		}

		// Closest hit group
		{
			shaderStages.push_back(loadShader(getShadersPath() + "raytracingshadows/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			shaderGroups.push_back(shaderGroup);
		}

		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI = vks::initializers::rayTracingPipelineCreateInfoKHR();
		rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		rayTracingPipelineCI.pStages = shaderStages.data();
		rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
		rayTracingPipelineCI.pGroups = shaderGroups.data();
		rayTracingPipelineCI.maxPipelineRayRecursionDepth = 2;
		rayTracingPipelineCI.layout = pipelineLayout;
		VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline));
	}

	/*
		Create the uniform buffer used to pass matrices to the ray tracing ray generation shader
	*/
	void createUniformBuffer()
	{
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&ubo,
			sizeof(uniformData),
			&uniformData));
		VK_CHECK_RESULT(ubo.map());

		updateUniformBuffers();
	}

	/*
		If the window has been resized, we need to recreate the storage image and it's descriptor
	*/
	void handleResize()
	{
		// Recreate image
		createStorageImage(swapChain.colorFormat, { width, height, 1 });
		// Update descriptor
		VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, storageImage.view, VK_IMAGE_LAYOUT_GENERAL };
		VkWriteDescriptorSet resultImageWrite = vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
		vkUpdateDescriptorSets(device, 1, &resultImageWrite, 0, VK_NULL_HANDLE);
		resized = false;
	}

	/*
		Command buffer generation
	*/
	void buildCommandBuffers()
	{
		if (resized)
		{
			handleResize();
		}

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
		{
			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			/*
				Dispatch the ray tracing commands
			*/
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

			VkStridedDeviceAddressRegionKHR emptySbtEntry = {};
			vkCmdTraceRaysKHR(
				drawCmdBuffers[i],
				&shaderBindingTables.raygen.stridedDeviceAddressRegion,
				&shaderBindingTables.miss.stridedDeviceAddressRegion,
				&shaderBindingTables.hit.stridedDeviceAddressRegion,
				&emptySbtEntry,
				width,
				height,
				1);

			/*
				Copy ray tracing output to swap chain image
			*/

			// Prepare current swap chain image as transfer destination
			vks::tools::setImageLayout(
				drawCmdBuffers[i],
				swapChain.images[i],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange);

			// Prepare ray tracing output image as transfer source
			vks::tools::setImageLayout(
				drawCmdBuffers[i],
				storageImage.image,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				subresourceRange);

			VkImageCopy copyRegion{};
			copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.srcOffset = { 0, 0, 0 };
			copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.dstOffset = { 0, 0, 0 };
			copyRegion.extent = { width, height, 1 };
			vkCmdCopyImage(drawCmdBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChain.images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			// Transition swap chain image back for presentation
			vks::tools::setImageLayout(
				drawCmdBuffers[i],
				swapChain.images[i],
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				subresourceRange);

			// Transition ray tracing output image back to general layout
			vks::tools::setImageLayout(
				drawCmdBuffers[i],
				storageImage.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_GENERAL,
				subresourceRange);

			drawUI(drawCmdBuffers[i], frameBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}

	void updateUniformBuffers()
	{
		uniformData.projInverse = glm::inverse(camera.matrices.perspective);
		uniformData.viewInverse = glm::inverse(camera.matrices.view);
		uniformData.lightPos = glm::vec4(5.0f, -5.0f, 5.0f, 1.0f);// glm::vec4(cos(glm::radians(timer * 360.0f)) * 40.0f, -50.0f + sin(glm::radians(timer * 360.0f)) * 20.0f, 25.0f + sin(glm::radians(timer * 360.0f)) * 5.0f, 0.0f);
		// Pass the vertex size to the shader for unpacking vertices
		uniformData.vertexSize = sizeof(vkglTF::Vertex);
		memcpy(ubo.mapped, &uniformData, sizeof(uniformData));
	}

	void getEnabledFeatures()
	{
		// Enable features required for ray tracing using feature chaining via pNext		
		enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;

		enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
		enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;

		enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
		enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;

		deviceCreatepNextChain = &enabledAccelerationStructureFeatures;
	}

	void prepare()
	{
		VulkanRaytracingSample::prepare();

		// Create the acceleration structures used to render the ray traced scene
		createBottomLevelAccelerationStructure();
		createTopLevelAccelerationStructure();

		createStorageImage(swapChain.colorFormat, { width, height, 1 });
		createUniformBuffer();
		createRayTracingPipeline();
		createShaderBindingTables();
		createDescriptorSets();
		buildCommandBuffers();
		prepared = true;
	}

	void draw()
	{
		VulkanExampleBase::prepareFrame();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanExampleBase::submitFrame();
	}

	virtual void render()
	{
		if (!prepared)
			return;
		draw();
		if (!paused || camera.updated)
			updateUniformBuffers();
	}
};

VULKAN_EXAMPLE_MAIN()

