#pragma once

#include "FXLib.h"

namespace FX
{
	struct FXLIB_API PostPass
	{
		const fdm::Shader* shader = nullptr;
		uint32_t targetFBO = 0;
		uint32_t targetTex = 0;
		int width = 1, height = 1;
		uint32_t internalFormat = GL_RGB;
		uint32_t format = GL_RGB;
		uint32_t formatType = GL_FLOAT;
		std::unordered_map<std::string, uint32_t> textures{};

		PostPass() {}
		PostPass(const fdm::Shader* shader) :
			shader(shader)
		{
		}
		PostPass(PostPass&& other) noexcept;
		PostPass& operator=(PostPass&& other) noexcept;
		PostPass(const PostPass& other);
		PostPass& operator=(const PostPass& other);
		void initTarget(int width, int height, uint32_t internalFormat, uint32_t format, uint32_t formatType);
		void cleanup();
		~PostPass();

		static const fdm::Shader* loadPassShader(const fdm::stl::string& name, const fdm::stl::string& fragmentPath);
	};

	using FramebufferInitCallback = std::add_pointer<void(uint32_t fbo, uint32_t colorTex, uint32_t depthTex, int width, int height, std::vector<PostPass>& passes)>::type;
	FXLIB_API void applyPostProcessing(fdm::Framebuffer& fb, FramebufferInitCallback initCallback);
}
