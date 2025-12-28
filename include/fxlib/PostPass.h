#pragma once

#include "FXLib.h"

namespace FX
{
	struct FXLIB_API Uniform
	{
		enum Type
		{
			FLOAT,
			VEC2,
			VEC3,
			VEC4,
			INT,
			IVEC2,
			IVEC3,
			IVEC4,
			UINT,
			UVEC2,
			UVEC3,
			UVEC4
		} type = FLOAT;
		std::string name = "";
		void* value = nullptr;
	};

	struct FXLIB_API PostPass
	{
		const fdm::Shader* shader = nullptr;
		int sizeDiv = 1;
		enum
		{
			R,
			RG,
			RGB,
			RGBA
		} passFormat = RGBA;

		// internal stuff
		uint32_t targetTex = 0;
		int width = 1, height = 1;
		void initTexture(int width, int height);

		PostPass() {}
		PostPass(const fdm::Shader* shader, int sizeDiv = 1) :
			shader(shader), sizeDiv(sizeDiv) { }
		PostPass(PostPass&& other) noexcept;
		PostPass& operator=(PostPass&& other) noexcept;
		PostPass(const PostPass& other);
		PostPass& operator=(const PostPass& other);
		void cleanup();
		~PostPass();

		static const fdm::Shader* loadPassShader(const fdm::stl::string& name, const fdm::stl::string& fragmentPath);
	};

	struct FXLIB_API PostPassGroup
	{
		using DrawCallback = std::add_pointer<void(PostPassGroup& pass)>::type;

		std::vector<PostPass> passes{ };
		std::vector<Uniform> uniforms{ };
		std::unordered_map<std::string, uint32_t> uniformTextures{ }; // uniform name -> texture id
		enum ViewportMode
		{
			CURRENT_PASS_SIZE,
			PREV_PASS_SIZE,
			NEXT_PASS_SIZE
		} viewportMode = CURRENT_PASS_SIZE;
		struct PassIteration
		{
			enum Direction
			{
				FORWARD,
				BACKWARD,
			} dir = FORWARD;
			enum Count
			{
				ALL,
				SKIP_FIRST,
				SKIP_LAST,
			} count = ALL;
		} iteration;
		struct Blending
		{
			enum Mode
			{
				DISABLED,
				ADD,
				SUBTRACT,
				REVERSE_SUBTRACT,
				MIN,
				MAX
			} mode = DISABLED;
			struct Func
			{
				enum Factor
				{
					ZERO = GL_ZERO,
					ONE = GL_ONE,
					SRC_COLOR = GL_SRC_COLOR,
					ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
					DST_COLOR = GL_DST_COLOR,
					ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
					SRC_ALPHA = GL_SRC_ALPHA,
					ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
					DST_ALPHA = GL_DST_ALPHA,
					ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA
				};
				Factor srcFactor = ONE;
				Factor dstFactor = ZERO;
			} func;
		} blending;

		bool copyLastGroup = false; // blit last group's passes into this group's passes
		bool clearColor = true;

		uint32_t targetFBO = 0;
		uint32_t outputTex = 0;

		DrawCallback preDrawCallback = nullptr;
		DrawCallback postDrawCallback = nullptr;

		PostPassGroup() {}
		PostPassGroup(const std::vector<PostPass>& passes)
			: passes(passes) {
		}
		PostPassGroup(const PostPass& pass)
			: passes({ pass }) {
		}
		PostPassGroup(PostPassGroup&& other) noexcept;
		PostPassGroup& operator=(PostPassGroup&& other) noexcept;
		PostPassGroup(const PostPassGroup& other);
		PostPassGroup& operator=(const PostPassGroup& other);
		~PostPassGroup();
	};

	using FramebufferInitCallback = std::add_pointer<void(uint32_t fbo, uint32_t colorTex, uint32_t depthTex, int width, int height, std::vector<PostPassGroup>& passGroups)>::type;
	FXLIB_API void applyPostProcessing(fdm::Framebuffer& fb, FramebufferInitCallback initCallback);
}
