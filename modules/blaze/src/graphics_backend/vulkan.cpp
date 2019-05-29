#include "graphics_backend/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <set>
#include <fstream>
#include <optional>
#include <array>

#include <chrono>

#include "logger.h"
#include "engine.h"

#include "archive_manager.h"
#include "asset_manager.h"

#include "iohelper/memstream.h"
#include "iohelper/read.h"

#ifdef __ANDROID__
	#include "platform_android.h"
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* callback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, create_info, allocator, callback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* allocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, allocator);
	}
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, create_info, allocator, callback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* allocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, allocator);
	}
}

// We are not making this a function of Vertex as vertex will be used by other graphic backends later on as well
VkVertexInputBindingDescription get_vertex_binding_description() {
	VkVertexInputBindingDescription binding_description  {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(blaze::Vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_description;
}

std::array<VkVertexInputAttributeDescription, 3> get_vertex_attribute_description() {
	std::array<VkVertexInputAttributeDescription, 3> attribute_description = {};

	attribute_description[0].binding = 0;
	attribute_description[0].location = 0;
	attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_description[0].offset = offsetof(blaze::Vertex, pos);

	attribute_description[1].binding = 0;
	attribute_description[1].location = 1;
	attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_description[1].offset = offsetof(blaze::Vertex, color);

	attribute_description[2].binding = 0;
	attribute_description[2].location = 2;
	attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_description[2].offset = offsetof(blaze::Vertex, tex_coord);

	return attribute_description;
}

namespace BLAZE_NAMESPACE {
	VulkanTexture::VulkanTexture(std::string asset_name, VulkanBackend* backend) : GameAsset(asset_name, {std::bind(&VulkanTexture::load, this, std::placeholders::_1)}), _backend(backend) {
		LOG_D("Creating texture\n");
	}

	VulkanTexture::~VulkanTexture() {
		vkDestroySampler(_backend->_device, _texture_sampler, nullptr);

		vkDestroyImageView(_backend->_device, _texture_image_view, nullptr);

		vkDestroyImage(_backend->_device, _texture_image, nullptr);
		vkFreeMemory(_backend->_device, _texture_image_memory, nullptr);
	}

	std::vector<uint8_t> VulkanTexture::load(std::vector<uint8_t> data) {
		create_texture_image(data);
		create_texture_image_view();
		create_texture_sampler();

		return data;
	}

	void VulkanTexture::create_texture_image(std::vector<uint8_t>& data) {
		iohelper::imemstream stream(data);

		int width = iohelper::read_length(stream);
		int height = iohelper::read_length(stream);
		int channels = iohelper::read<uint8_t>(stream);

		if (channels != 4) {
			throw std::runtime_error("We can only load images with 4 channels");
		}

		int size = width * height * channels;
		// @todo Kind of jenky
		int offset = data.size() - size;

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		_backend->create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

		void* temp;
		vkMapMemory(_backend->_device, staging_buffer_memory, 0, size, 0, &temp);
		memcpy(temp, data.data() + offset, size);
		vkUnmapMemory(_backend->_device, staging_buffer_memory);

		_backend->create_image(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _texture_image, _texture_image_memory);

		_backend->transition_image_layout(_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		_backend->copy_buffer_to_image(staging_buffer, _texture_image, width, height);

		_backend->transition_image_layout(_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(_backend->_device, staging_buffer, nullptr);
		vkFreeMemory(_backend->_device, staging_buffer_memory, nullptr);
	}

	void VulkanTexture::create_texture_image_view() {
		_texture_image_view = _backend->create_image_view(_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanTexture::create_texture_sampler() {
		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		// sampler_info.magFilter = VK_FILTER_LINEAR;
		// sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.magFilter = VK_FILTER_NEAREST;
		sampler_info.minFilter = VK_FILTER_NEAREST;

		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		// @todo Make this optonal instead of required
		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy = 16;

		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.unnormalizedCoordinates = VK_FALSE;

		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;

		if (vkCreateSampler(_backend->_device, &sampler_info, nullptr, &_texture_sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	VulkanModel::VulkanModel(std::string asset_name, std::shared_ptr<VulkanMaterial> material, VulkanBackend* backend) : GameAsset(asset_name, {std::bind(&VulkanModel::load, this, std::placeholders::_1)}), _material(material), _backend(backend) {
		LOG_D("Creating model\n");
	}

	VulkanModel::~VulkanModel() {
		vkDestroyBuffer(_backend->_device, _vertex_buffer, nullptr);
		vkFreeMemory(_backend->_device, _vertex_buffer_memory, nullptr);

		vkDestroyBuffer(_backend->_device, _index_buffer, nullptr);
		vkFreeMemory(_backend->_device, _index_buffer_memory, nullptr);
	}

	void VulkanModel::commands(VkCommandBuffer command_buffer, int index) {
		_material->commands(command_buffer, index);

		VkBuffer vertex_buffers[] = {_vertex_buffer};
		VkDeviceSize offsets[] = {0};

		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

		vkCmdBindIndexBuffer(command_buffer, _index_buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
	}

	void VulkanModel::recreate() {
		_material->recreate();
	}

	std::vector<uint8_t> VulkanModel::load(std::vector<uint8_t> data) {
		load_model(data);
		create_vertex_buffer();
		create_index_buffer();

		return data;
	}

	void VulkanModel::load_model(std::vector<uint8_t>& data) {
		iohelper::imemstream stream(data);

		_vertices.resize(iohelper::read_length(stream));
		for (auto& vertex : _vertices) {
			vertex.pos.x = iohelper::read<float>(stream);
			vertex.pos.y = iohelper::read<float>(stream);
			vertex.pos.z = iohelper::read<float>(stream);

			vertex.color.r = iohelper::read<float>(stream);
			vertex.color.g = iohelper::read<float>(stream);
			vertex.color.b = iohelper::read<float>(stream);

			vertex.tex_coord.x = iohelper::read<float>(stream);
			vertex.tex_coord.y = iohelper::read<float>(stream);
		}

		_indices.resize(iohelper::read_length(stream));
		for (auto& index : _indices) {
			index = iohelper::read<uint32_t>(stream);
		}
	}

	void VulkanModel::create_vertex_buffer() {
		VkDeviceSize buffer_size = sizeof(_vertices[0]) * _vertices.size();

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;
		_backend->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

		void* data;
		vkMapMemory(_backend->_device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, _vertices.data(), static_cast<uint32_t>(buffer_size));
		vkUnmapMemory(_backend->_device, staging_buffer_memory);

		_backend->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertex_buffer, _vertex_buffer_memory);

		_backend->copy_buffer(staging_buffer, _vertex_buffer, buffer_size);

		vkDestroyBuffer(_backend->_device, staging_buffer, nullptr);
		vkFreeMemory(_backend->_device, staging_buffer_memory, nullptr);
	}

	void VulkanModel::create_index_buffer() {
		VkDeviceSize buffer_size = sizeof(_indices[0]) * _indices.size();

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;
		_backend->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

		void* data;
		vkMapMemory(_backend->_device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, _indices.data(), static_cast<uint32_t>(buffer_size));
		vkUnmapMemory(_backend->_device, staging_buffer_memory);

		_backend->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _index_buffer, _index_buffer_memory);

		_backend->copy_buffer(staging_buffer, _index_buffer, buffer_size);

		vkDestroyBuffer(_backend->_device, staging_buffer, nullptr);
		vkFreeMemory(_backend->_device, staging_buffer_memory, nullptr);
	}

	// @todo We need to bae able to pass in the seperate shader parts, these parts store what type they are and we link it together here
	// @todo We need a description of sorts that can be used to generate the descriptor set layout and later fill them in
	VulkanShader::VulkanShader(std::string vertex_name, std::string fragment_name, VulkanBackend* backend) : GameAsset(vertex_name), _fragment_handle(archive_manager::load_data(fragment_name)), _backend(backend) {
		_name += ";" + fragment_name;
		LOG_D("Creating shader\n");
	}

	VulkanShader::~VulkanShader() {
		vkDestroyShaderModule(_backend->_device, _vert_shader_module, nullptr);
		vkDestroyShaderModule(_backend->_device, _frag_shader_module, nullptr);
		vkDestroyPipeline(_backend->_device, _pipeline, nullptr);
		vkDestroyPipelineLayout(_backend->_device, _pipeline_layout, nullptr);
		vkDestroyDescriptorSetLayout(_backend->_device, _descriptor_set_layout, nullptr);
	}

	void VulkanShader::recreate() {
		vkDestroyPipeline(_backend->_device, _pipeline, nullptr);
		vkDestroyPipelineLayout(_backend->_device, _pipeline_layout, nullptr);
		create_graphics_pipeline();
	}

	void VulkanShader::commands(VkCommandBuffer command_buffer) {
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	}

	bool VulkanShader::is_loaded() {
		bool vertex_loaded = GameAsset::is_loaded();
		bool fragment_loaded = _fragment_handle.is_loaded();

		static bool loaded = false;
		if (vertex_loaded && fragment_loaded && !loaded) {
			LOG_D("Both the fragment and vertex shader have been loaded\n");
			// @todo Make sure this happends in a seperate thread
			_vert_shader_module = _backend->create_shader_module(_data_handle.get());
			_frag_shader_module = _backend->create_shader_module(_fragment_handle.get());

			create_descriptor_set_layout();
			create_graphics_pipeline();

			loaded = true;
		}

		return loaded;
	}

	void VulkanShader::create_graphics_pipeline() {
		// --- This section can be made an asset aswel

		VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = _vert_shader_module;
		vert_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = _frag_shader_module;
		frag_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
		// ----

		auto binding_description = get_vertex_binding_description();
		auto attribute_description = get_vertex_attribute_description();

		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.pVertexBindingDescriptions = &binding_description;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
		vertex_input_info.pVertexAttributeDescriptions = attribute_description.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = _backend->_swap_chain_extent.width;
		viewport.height = _backend->_swap_chain_extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = _backend->_swap_chain_extent;

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		// Due to the way glm works everything is drawn the other way around
		// rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;

		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;

		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.minDepthBounds = 0.0f;
		depth_stencil.maxDepthBounds = 1.0f;

		depth_stencil.stencilTestEnable = VK_FALSE;
		depth_stencil.front = {};
		depth_stencil.back = {};

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending = {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f;
		color_blending.blendConstants[1] = 0.0f;
		color_blending.blendConstants[2] = 0.0f;
		color_blending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &_descriptor_set_layout;
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(_backend->_device, &pipeline_layout_info, nullptr, &_pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;

		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = &depth_stencil;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = nullptr;

		pipeline_info.layout = _pipeline_layout;
		pipeline_info.renderPass = _backend->_render_pass;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(_backend->_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics pipeline!");
		}
	}

	// This needs some kind of configuration
	void VulkanShader::create_descriptor_set_layout() {
		VkDescriptorSetLayoutBinding ubo_layout_binding = {};
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ubo_layout_binding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding sampler_layout_binding = {};
		sampler_layout_binding.binding = 1;
		sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sampler_layout_binding.descriptorCount = 1;
		sampler_layout_binding.pImmutableSamplers = nullptr;
		sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// @todo Replace all the times we used vector with array
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};
		VkDescriptorSetLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = bindings.size();
		layout_info.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(_backend->_device, &layout_info, nullptr, &_descriptor_set_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}

	VulkanMaterial::VulkanMaterial(std::shared_ptr<VulkanShader> shader, std::shared_ptr<VulkanTexture> texture, VulkanBackend* backend) : _shader(shader), _texture(texture), _backend(backend) {
		create_descriptor_sets();
	}

	void VulkanMaterial::commands(VkCommandBuffer command_buffer, int index) {
		_shader->commands(command_buffer);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _shader->_pipeline_layout, 0, 1, &_descriptor_sets[index], 0, nullptr);
		// Here we have to bind the descriptor set
	}

	void VulkanMaterial::create_descriptor_sets() {
		_descriptor_sets.resize(_backend->_swap_chain_images.size());

		std::vector<VkDescriptorSetLayout> layouts(_backend->_swap_chain_images.size(), _shader->_descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _backend->_descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(_backend->_swap_chain_images.size());
		alloc_info.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(_backend->_device, &alloc_info, _descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < _backend->_swap_chain_images.size(); ++i) {
			VkDescriptorBufferInfo buffer_info = {};
			buffer_info.buffer = _backend->_uniform_buffers[i];
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo image_info = {};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = _texture->_texture_image_view;
			image_info.sampler = _texture->_texture_sampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};
			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = _descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = _descriptor_sets[i];
			descriptor_writes[1].dstBinding = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(_backend->_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
		}
	}

	VulkanBackend::VulkanBackend() : _enable_validation_layers(CVar::get<int>("debug")) {}

	void VulkanBackend::init() {
		init_window();
		init_vulkan();
	}

	void VulkanBackend::update() {
		dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_update();

#ifdef __ANDROID__
		if (_framebuffer_resized) {
			// @todo This is super janky
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
#endif

		draw_frame();
	}

	void VulkanBackend::cleanup() {
		vkDeviceWaitIdle(_device);

		for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			vkDestroySemaphore(_device, _image_available_semaphores[i], nullptr);
			vkDestroySemaphore(_device, _render_finished_semaphores[i], nullptr);
			vkDestroyFence(_device, _in_flight_fences[i], nullptr);
		}

		_vulkan_model = nullptr;

		cleanup_swap_chain();

		vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);

		for (size_t i = 0; i < _swap_chain_images.size(); ++i) {
			vkDestroyBuffer(_device, _uniform_buffers[i], nullptr);
			vkFreeMemory(_device, _uniform_buffers_memory[i], nullptr);
		}

		vkDestroyCommandPool(_device, _graphics_command_pool, nullptr);
		vkDestroyCommandPool(_device, _transfer_command_pool, nullptr);

		vkDestroyDevice(_device, nullptr);

		if (_enable_validation_layers) {
			#if !defined(__ANDROID__)
				DestroyDebugUtilsMessengerEXT(_instance, _callback, nullptr);
			#else
				DestroyDebugReportCallbackEXT(_instance, _callback, nullptr);
			#endif
		}

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);

		dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_destroy();
	}

	bool VulkanBackend::is_running() {
		return dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_is_running();
	}


	void VulkanBackend::init_window() {
		dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_init(this);
	}

	void VulkanBackend::init_vulkan() {
		create_instance();
		setup_debug_callback();
		create_surface();
		pick_physical_device();
		create_logical_device();
		create_swap_chain();
		create_image_views();
		create_render_pass();
		create_command_pools();

		create_depth_resources();
		create_framebuffers();
		create_uniform_buffers();
		create_descriptor_pool();
		load_assets();
		create_command_buffers();
		create_sync_objects();
	}

	void VulkanBackend::cleanup_swap_chain() {
		for (auto framebuffer : _swap_chain_framebuffers) {
			vkDestroyFramebuffer(_device, framebuffer, nullptr);
		}

		vkFreeCommandBuffers(_device, _graphics_command_pool, static_cast<uint32_t>(_graphics_command_buffers.size()), _graphics_command_buffers.data());

		vkDestroyRenderPass(_device, _render_pass, nullptr);

		vkDestroyImageView(_device, _depth_image_view, nullptr);
		vkDestroyImage(_device, _depth_image, nullptr);
		vkFreeMemory(_device, _depth_image_memory, nullptr);

		for (auto image_view : _swap_chain_image_views) {
			vkDestroyImageView(_device, image_view, nullptr);
		}

		vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
	}

	void VulkanBackend::recreate_swapchain() {
		vkDeviceWaitIdle(_device);

		LOG_D("Recreating swap chain\n");

		cleanup_swap_chain();

		create_swap_chain();
		create_image_views();
		create_render_pass();
		_vulkan_model->recreate();
		create_depth_resources();
		create_framebuffers();
		create_command_buffers();
	}

	void VulkanBackend::create_instance() {
		if (_enable_validation_layers && !check_validation_layer_support()) {
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Hello triangle";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "protoBlaze Engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		auto extensions = get_required_extensions();

		check_extension_support(extensions);

		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

		if (_enable_validation_layers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
			create_info.ppEnabledLayerNames = _validation_layers.data();
		} else {
			create_info.enabledLayerCount = 0;
		}

		VkResult result = vkCreateInstance(&create_info, nullptr, &_instance);
		LOG_D("result = {}\n", result);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vulkan instance!");
		}
	}

	void VulkanBackend::setup_debug_callback() {
		if (!_enable_validation_layers) {
			return;
		}

		#if !defined(__ANDROID__)
			VkDebugUtilsMessengerCreateInfoEXT create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			create_info.pfnUserCallback = vk_debug_callback;
			create_info.pUserData = nullptr;

			VkResult result = CreateDebugUtilsMessengerEXT(_instance, &create_info, nullptr, &_callback);
			LOG_D("result = {}\n", result);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to create vulkan instance!");
			}
		#else
			VkDebugReportCallbackCreateInfoEXT create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			create_info.pNext = nullptr;
			create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			create_info.pfnCallback = vk_debug_callback;
			create_info.pUserData = nullptr;

			VkResult result = CreateDebugReportCallbackEXT(_instance, &create_info, nullptr, &_callback);
			LOG_D("result = {}\n", result);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to create vulkan instance!");
			}
		#endif
	}

	void VulkanBackend::create_surface() {
		LOG_D("Creating surface\n");
		_surface = dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_create_surface(_instance);

		LOG_D("Surface created\n");
	}

	void VulkanBackend::pick_physical_device() {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);

		if (device_count == 0) {
			throw std::runtime_error("Failed to find GPU with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		static int& log_level = CVar::get<int>("log_level");
		for (const auto& device : devices) {
			int score = rate_device_suitability(device);
			candidates.insert({score, device});

			if ((Level)log_level <= Level::debug) {
				VkPhysicalDeviceProperties device_properties;
				vkGetPhysicalDeviceProperties(device, &device_properties);

				LOG_D("Rated GPU: {} ({})\n", device_properties.deviceName, score);
			}
		}

		if (candidates.size() > 0 && candidates.rbegin()->first > 0) {
			_physical_device = candidates.rbegin()->second;

			if ((Level)log_level <= Level::debug) {
				VkPhysicalDeviceProperties device_properties;
				vkGetPhysicalDeviceProperties(_physical_device, &device_properties);

				LOG_D("Selected GPU: {} ({})\n", device_properties.deviceName, candidates.rbegin()->first);
			}
		} else {
			throw std::runtime_error("Failed to find suitable GPU!");
		}
	}

	void VulkanBackend::create_logical_device() {
		QueueFamilyIndices indices = find_queue_families(_physical_device);

		LOG_D("Graphics family: {}\n", indices.graphics_family.value());
		LOG_D("Present family: {}\n", indices.present_family.value());
		LOG_D("Transfer family: {}\n", indices.transfer_family.value());

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value(), indices.transfer_family.value()};

		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info = {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;

			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures device_features = {};
		device_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = &device_features;

		create_info.enabledExtensionCount = static_cast<uint32_t>(_device_extensions.size());
		create_info.ppEnabledExtensionNames = _device_extensions.data();

		if (_enable_validation_layers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
			create_info.ppEnabledLayerNames = _validation_layers.data();
		} else {
			create_info.enabledLayerCount = 0;
		}

		if (vkCreateDevice(_physical_device, &create_info, nullptr, &_device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(_device, indices.graphics_family.value(), 0, &_graphics_queue);
		vkGetDeviceQueue(_device, indices.present_family.value(), 0, &_present_queue);
		vkGetDeviceQueue(_device, indices.transfer_family.value(), 0, &_transfer_queue);
	}

	void VulkanBackend::create_swap_chain() {
		SwapChainSupportDetails swap_chain_support = query_swap_chain_support(_physical_device);

		VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
		VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
		VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

		LOG_D("Resolution {}x{}\n", extent.width, extent.height);

		LOG_D("Selecting minimum image count\n");
		uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
		if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
			LOG_D("Minimum image count exceeded maximum: {} > {}\n", image_count, swap_chain_support.capabilities.maxImageCount);
			image_count = swap_chain_support.capabilities.maxImageCount;
		}
		LOG_D("Using {}\n", image_count);

		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = _surface;

		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = find_queue_families(_physical_device);
		uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

		if (indices.graphics_family != indices.present_family) {
			// Performance of this is worse, but otherwise we need to do ownership transfers
			// @todo Implement ownership transfers
			LOG_D("Image sharing mode: CONCURRENT\n");
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		} else {
			LOG_D("Image sharing mode: EXCLUSIVE\n");
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		//create_info.preTransform = swap_chain_support.capabilities.currentTransform;
		create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		// @todo Linux needs INHERIT, while windows needs OPAQUE
		// Figure out code to auto deterime this
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		//create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		create_info.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(_device, &create_info, nullptr, &_swap_chain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(_device, _swap_chain, &image_count, nullptr);
		_swap_chain_images.resize(image_count);
		if (vkGetSwapchainImagesKHR(_device, _swap_chain, &image_count, _swap_chain_images.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swapchain images!");
		}
		LOG_D("Created {} swap chain images\n", image_count);

		_swap_chain_image_format = surface_format.format;
		_swap_chain_extent = extent;
	}

	void VulkanBackend::create_image_views() {
		_swap_chain_image_views.resize(_swap_chain_images.size());

		for (size_t i = 0; i < _swap_chain_images.size(); ++i) {
			LOG_D("Creating image view ({})\n", i);
			_swap_chain_image_views[i] = create_image_view(_swap_chain_images[i], _swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void VulkanBackend::create_render_pass() {
		VkAttachmentDescription color_attachment = {};
		color_attachment.format = _swap_chain_image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment = {};
		depth_attachment.format = find_depth_format();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref = {};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = attachments.size();
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render pass!");
		}
	}

	void VulkanBackend::create_framebuffers() {
		_swap_chain_framebuffers.resize(_swap_chain_image_views.size());

		for (size_t i = 0; i < _swap_chain_image_views.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				_swap_chain_image_views[i],
				_depth_image_view
			};

			VkFramebufferCreateInfo framebuffer_info = {};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = _render_pass;
			framebuffer_info.attachmentCount = attachments.size();
			framebuffer_info.pAttachments = attachments.data();
			framebuffer_info.width = _swap_chain_extent.width;
			framebuffer_info.height = _swap_chain_extent.height;
			framebuffer_info.layers = 1;

			if (vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_swap_chain_framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}

	void VulkanBackend::create_command_pools() {
		QueueFamilyIndices queue_family_indices = find_queue_families(_physical_device);

		{
			VkCommandPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
			pool_info.flags = 0;

			if (vkCreateCommandPool(_device, &pool_info, nullptr, &_graphics_command_pool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create command pool!");
			}
		}

		{
			VkCommandPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			pool_info.queueFamilyIndex = queue_family_indices.transfer_family.value();
			pool_info.flags = 0;

			if (vkCreateCommandPool(_device, &pool_info, nullptr, &_transfer_command_pool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create command pool!");
			}
		}
	}

	void VulkanBackend::create_depth_resources() {
		VkFormat depth_format = find_depth_format();
		create_image(_swap_chain_extent.width, _swap_chain_extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depth_image, _depth_image_memory);
		_depth_image_view = create_image_view(_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

		transition_image_layout(_depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	// @todo We need to make sure that at any point load new assets
	// That way we can start to move most of this outside of the backend
	// The goal is to create this in the main game loop and render it from there
	void VulkanBackend::load_assets() {
		auto shader = asset_manager::new_asset<VulkanShader>("/resources/base/shader/Vertex", "/resources/base/shader/Fragment", this);
		auto texture = asset_manager::new_asset<VulkanTexture>("/resources/base/texture/Test", this);
		auto texture2 = asset_manager::new_asset<VulkanTexture>("/resources/base/texture/Chalet", this);

		while (asset_manager::loading_count() > 0) {
			asset_manager::load_assets();
		}
		// @todo This should be based on a GameAsset or something and wait with creating things until 
		auto material = std::make_shared<VulkanMaterial>(shader, texture2, this);

		_vulkan_model = asset_manager::new_asset<VulkanModel>("/resources/base/model/Chalet", material, this);

		while (asset_manager::loading_count() > 0) {
			asset_manager::load_assets();
		}
	}

	void VulkanBackend::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
		// @todo In the future we should collect all commands and then submit all at once
		VkCommandBuffer command_buffer = begin_single_time_commands(_transfer_command_pool);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;

		barrier.image = image;

		if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (has_stencil_component(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		} else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			QueueFamilyIndices indices = find_queue_families(_physical_device);
			barrier.srcQueueFamilyIndex = indices.transfer_family.value();
			barrier.dstQueueFamilyIndex = indices.graphics_family.value();
		} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

			QueueFamilyIndices indices = find_queue_families(_physical_device);
			barrier.srcQueueFamilyIndex = indices.transfer_family.value();
			barrier.dstQueueFamilyIndex = indices.graphics_family.value();
		} else {
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		end_single_time_commands(command_buffer, _transfer_queue);
	}

	void VulkanBackend::create_uniform_buffers() {
		VkDeviceSize buffer_size = sizeof(UniformBufferObject);

		_uniform_buffers.resize(_swap_chain_images.size());
		_uniform_buffers_memory.resize(_swap_chain_images.size());

		for (size_t i = 0; i < _swap_chain_images.size(); ++i) {
			create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniform_buffers[i], _uniform_buffers_memory[i]);
		}
	}

	void VulkanBackend::create_descriptor_pool() {
		std::array<VkDescriptorPoolSize,2> pool_sizes = {};
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<uint32_t>(_swap_chain_images.size());
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[1].descriptorCount = static_cast<uint32_t>(_swap_chain_images.size());

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = pool_sizes.size();
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(_swap_chain_images.size());

		if (vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool!");
		}
	}

	void VulkanBackend::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propteries, VkBuffer& buffer, VkDeviceMemory& buffer_memory) {
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		// @todo We should release owner ship to the graphics queue so we can use exclusive
		// buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		QueueFamilyIndices indices = find_queue_families(_physical_device);
		uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};
		if (indices.transfer_family != indices.graphics_family) {
			// Performance of this is worse, but otherwise we need to do ownership transfers
			// @todo Implement ownership transfers
			LOG_D("Buffer sharing mode: CONCURRENT\n");
			buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
			buffer_info.queueFamilyIndexCount = 2;
			buffer_info.pQueueFamilyIndices = queue_family_indices;
		} else {
			LOG_D("Buffer sharing mode: EXCLUSIVE\n");
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = 0;
			buffer_info.pQueueFamilyIndices = nullptr;
		}

		if (vkCreateBuffer(_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer!");
		}

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(_device, buffer, &mem_requirements);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, propteries);

		if (vkAllocateMemory(_device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(_device, buffer, buffer_memory, 0);
	}

	void VulkanBackend::create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory) {
		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = width;
		image_info.extent.height = height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;

		image_info.format = format;
		image_info.tiling = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = usage;

		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.flags = 0;

		if (vkCreateImage(_device, &image_info, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(_device, image, &mem_requirements);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(_device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(_device, image, image_memory, 0);
	}

	void VulkanBackend::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
		VkCommandBuffer command_buffer = begin_single_time_commands(_transfer_command_pool);

		VkBufferCopy copy_region = {};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;

		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

		end_single_time_commands(command_buffer, _transfer_queue);
	}

	void VulkanBackend::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer command_buffer = begin_single_time_commands(_transfer_command_pool);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			command_buffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		end_single_time_commands(command_buffer, _transfer_queue);
	}

	VkImageView VulkanBackend::create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) {
		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = format;
		view_info.subresourceRange.aspectMask = aspect_flags;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		VkImageView image_view;
		if (vkCreateImageView(_device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture image view!");
		}

		return image_view;
	}

	void VulkanBackend::create_command_buffers() {
		_graphics_command_buffers.resize(_swap_chain_framebuffers.size());

		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = _graphics_command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = static_cast<uint32_t>(_graphics_command_buffers.size());

		if (vkAllocateCommandBuffers(_device, &alloc_info, _graphics_command_buffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		for (size_t i = 0; i < _graphics_command_buffers.size(); i++) {
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			begin_info.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(_graphics_command_buffers[i], &begin_info) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo render_pass_info = {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = _render_pass;
			render_pass_info.framebuffer = _swap_chain_framebuffers[i];
			render_pass_info.renderArea.offset = {0, 0};
			render_pass_info.renderArea.extent = _swap_chain_extent;

			std::array<VkClearValue, 2> clear_values = {};
			clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
			clear_values[1].depthStencil = {1.0f, 0};
			render_pass_info.clearValueCount = clear_values.size();
			render_pass_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(_graphics_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			_vulkan_model->commands(_graphics_command_buffers[i], i);

			vkCmdEndRenderPass(_graphics_command_buffers[i]);

			if (vkEndCommandBuffer(_graphics_command_buffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void VulkanBackend::create_sync_objects() {
		_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			if (vkCreateSemaphore(_device, &semaphore_info, nullptr, &_image_available_semaphores[i]) != VK_SUCCESS || vkCreateSemaphore(_device, &semaphore_info, nullptr, &_render_finished_semaphores[i]) != VK_SUCCESS || vkCreateFence(_device, &fence_info, nullptr, &_in_flight_fences[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create semaphores!");
			}
		}
	}

	void VulkanBackend::draw_frame() {
		vkWaitForFences(_device, 1, &_in_flight_fences[_current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		uint32_t image_index;
		VkResult result = vkAcquireNextImageKHR(_device, _swap_chain, std::numeric_limits<uint64_t>::max(), _image_available_semaphores[_current_frame], VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			_framebuffer_resized = false;
			recreate_swapchain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		update_uniform_buffer(image_index);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = {_image_available_semaphores[_current_frame]};
		VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &_graphics_command_buffers[image_index];

		VkSemaphore signal_semaphores[] = {_render_finished_semaphores[_current_frame]};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		vkResetFences(_device, 1, &_in_flight_fences[_current_frame]);

		if (vkQueueSubmit(_graphics_queue, 1, &submit_info, _in_flight_fences[_current_frame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = {_swap_chain};
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;

		result = vkQueuePresentKHR(_present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebuffer_resized) {
			_framebuffer_resized = false;
			recreate_swapchain();
			return;
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		_current_frame = (_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanBackend::update_uniform_buffer(uint32_t image_index) {
		static auto start_time = std::chrono::high_resolution_clock::now();

		auto current_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), _swap_chain_extent.width / static_cast<float>(_swap_chain_extent.height), 0.1f, 10.0f);
		// glm was made for OpenGL, y axis is inverted compared to Vulkan so we scale the y axis with -1
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(_device, _uniform_buffers_memory[image_index], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(_device, _uniform_buffers_memory[image_index]);
	}

	std::vector<const char*> VulkanBackend::get_required_extensions() {
		auto extensions = dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_get_required_extensions();

		if (_enable_validation_layers) {
			#if !defined(__ANDROID__)
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			#else
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			#endif
		}

		return extensions;
	}

	bool VulkanBackend::check_extension_support(std::vector<const char*> required_extensions) {
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		for (const auto& extension_name : required_extensions) {
			LOG_D("Looking for extension: {}\n", extension_name);
			if (std::find_if(extensions.begin(), extensions.end(), [&extension_name](const auto& e) {
						return strcmp(e.extensionName, extension_name) == 0;
						}) == extensions.end()) {
				LOG_E("Extension not found: {}\n", extension_name);
				return false;
			} else {
				LOG_D("Extension found: {}\n", extension_name);
			}
		}

		return true;
	}

	bool VulkanBackend::check_validation_layer_support() {
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

		for (const auto& layer_name : _validation_layers) {
			LOG_D("Looking for layer: {}\n", layer_name);
			if (std::find_if(layers.begin(), layers.end(), [&layer_name](const auto& l) {
						return strcmp(l.layerName, layer_name) == 0;
						}) == layers.end()) {
				LOG_E("Validation layer not found: {}\n", layer_name);
				return false;
			} else {
				LOG_D("Validation layer found: {}\n", layer_name);
			}
		}

		return true;
	}

	int VulkanBackend::rate_device_suitability(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;

		vkGetPhysicalDeviceProperties(device, &device_properties);
		vkGetPhysicalDeviceFeatures(device, &device_features);

		int score = 0;

		// Make sure that the device supports all required features
		//if (!device_features.geometryShader) {
			//LOG_D("Device does not support geometry shader\n");
			//return 0;
		//}
		if (!device_features.samplerAnisotropy) {
			LOG_D("Device does not support anisotropy\n");
			return 0;
		}

		// Make sure the device has the required queues
		if (!find_queue_families(device).is_complete()) {
			LOG_D("Device does not support families\n");
			return 0;
		}

		if (!check_device_extension_support(device)) {
			LOG_D("Device does not support device extensions\n");
			return 0;
		}

		SwapChainSupportDetails swap_chain_support = query_swap_chain_support(device);
		if (swap_chain_support.formats.empty() || swap_chain_support.present_modes.empty()) {
			LOG_D("Device does not support swapchain stuff\n");
			return 0;
		}

		// Select discrete gpu over integrated
		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		// Prefer higher resolution capabilities
		score += device_properties.limits.maxImageDimension2D/100;

		return score;
	}

	// @todo This function gets called multiple times
	VulkanBackend::QueueFamilyIndices VulkanBackend::find_queue_families(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		for (size_t i = 0; i < queue_families.size(); ++i) {
			const auto& queue_family = queue_families[i];
			if (queue_family.queueCount > 0) {
				if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphics_family = i;
				}

				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &present_support);
				if (present_support) {
					indices.present_family = i;
				}

				// Look for queue that does transfer but not graphics
				if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT && !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
					indices.transfer_family = i;
				}
			}

			if (indices.is_complete()) {
				break;
			}
		}

		// Fallback in case we can't find an appropriate queue
		if (!indices.transfer_family.has_value()) {
			indices.transfer_family = indices.graphics_family;
		}

		return indices;
	}

	bool VulkanBackend::check_device_extension_support(VkPhysicalDevice device) {
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

		for (const auto& extension_name : _device_extensions) {
			LOG_D("Looking for device extension: {}\n", extension_name);
			if (std::find_if(extensions.begin(), extensions.end(), [&extension_name](const auto& e) {
						return strcmp(e.extensionName, extension_name) == 0;
						}) == extensions.end()) {
				LOG_E("Device extension not found: {}\n", extension_name);
				return false;
			} else {
				LOG_D("Device extension found: {}\n", extension_name);
			}
		}

		return true;
	}

	VulkanBackend::SwapChainSupportDetails VulkanBackend::query_swap_chain_support(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, nullptr);
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, details.formats.data());

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, nullptr);
		details.present_modes.resize(format_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, details.present_modes.data());

		return details;
	}

	VkSurfaceFormatKHR VulkanBackend::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> available_formats) {
		LOG_D("Selecting swap format\n");
		if (available_formats.size() == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED) {
			LOG_D("Using VK_FORMAT_B8G8R8A8_UNORM and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR\n");
			return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}

		for (const auto& available_format : available_formats) {
			if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				LOG_D("Using VK_FORMAT_B8G8R8A8_UNORM and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR\n");
				return available_format;
			}
		}

		LOG_D("Falling back to first option\n");
		return available_formats[0];
	}

	VkPresentModeKHR VulkanBackend::choose_swap_present_mode(const std::vector<VkPresentModeKHR> available_present_modes) {
		LOG_D("Selecting swap present mode\n");
		for (const auto& available_present_mode : available_present_modes) {
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				LOG_D("Using VK_PRESENT_MODE_MAILBOX_KHR\n");
				return available_present_mode;
			}
		}

		LOG_D("Falling back to VK_PRESENT_MODE_FIFO_KHR\n");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanBackend::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			int width = 0;
			int height = 0;
			dynamic_cast<VulkanPlatformSupport*>(blaze::get_platform().get())->vulkan_get_framebuffer_size(width, height);

			VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

			actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
			actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

			return actual_extent;
		}
	}

	VkShaderModule VulkanBackend::create_shader_module(const std::vector<uint8_t>& code) {
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = code.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shader_module;
		if (vkCreateShaderModule(_device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module!");
		}

		return shader_module;
	}

	VkFormat VulkanBackend::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(_physical_device, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
				return format;
			}

			throw std::runtime_error("Unable to find supported format!");
		}
	}

	VkFormat VulkanBackend::find_depth_format() {
		return find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	bool VulkanBackend::has_stencil_component(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	uint32_t VulkanBackend::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags propteries) {
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(_physical_device, &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
			if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & propteries) == propteries) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	VkCommandBuffer VulkanBackend::begin_single_time_commands(VkCommandPool command_pool) {
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		if (vkAllocateCommandBuffers(_device, &alloc_info, &command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void VulkanBackend::end_single_time_commands(VkCommandBuffer command_buffer, VkQueue queue) {
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(_device, _transfer_command_pool, 1, &command_buffer);
	}

	#if !defined(__ANDROID__)
		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanBackend::vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT /* message_severity */, VkDebugUtilsMessageTypeFlagsEXT /* message_type */, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* /* user_data */) {
			LOG_D("VkDebug: {}\n", callback_data->pMessage);

			return VK_FALSE;
		}
	#else
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanBackend::vk_debug_callback(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData) {
			LOG_D("VkDebug: {}\n", pMsg);

			return VK_FALSE;
		}
	#endif
}
