//#include "common.h"
//#include "shader.h"
#include "../common.h"
#include "vulkan.h"

#include "shader.h"

VkShaderModule createShaderModule(VkDevice device, const char* path)
{
	FILE* file = fopen(path, "rb"); // read binary flag
	assert(file);

	fseek(file, 0, SEEK_END);
	long length = ftell(file); // length (bytes) of the shader bytecode
	assert(length >= 0);
	fseek(file, 0, SEEK_SET);

	string buffer;
	buffer.reserve(length);

	size_t rc = fread(&buffer[0], 1, length, file);
	assert(rc == size_t(length));
	fclose(file);

	VkShaderModuleCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.codeSize = length; // note: this needs to be a number of bytes!
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.c_str());

	VkShaderModule shaderModule = nullptr;
	VKCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	assert(shaderModule);
	assert(length % 4 == 0);

	return shaderModule;
}