#include "include/fxlib/FXLib.h"
#include "include/fxlib/PostPass.h"

using namespace FX;
using namespace fdm;

void PostPass::initTexture(int width, int height, uint32_t internalFormat, uint32_t format, uint32_t formatType)
{
	cleanup();

	//glGenFramebuffers(1, &targetFBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
	{
		glGenTextures(1, &targetTex);
		glBindTexture(GL_TEXTURE_2D, targetTex);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, formatType, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTex, 0);
	}
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PostPass::PostPass(PostPass&& other) noexcept
{
	this->shader = other.shader;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->internalFormat = other.internalFormat;
	this->format = other.format;
	this->formatType = other.formatType;

	other.shader = nullptr;
	other.targetTex = 0;
	other.width = 1;
	other.height = 1;
	other.internalFormat = 0;
	other.format = 0;
	other.formatType = 0;
}

PostPass& PostPass::operator=(PostPass&& other) noexcept
{
	if (this != &other)
	{
		this->shader = other.shader;
		this->targetTex = other.targetTex;
		this->width = other.width;
		this->height = other.height;
		this->internalFormat = other.internalFormat;
		this->format = other.format;
		this->formatType = other.formatType;

		other.shader = nullptr;
		other.targetTex = 0;
		other.width = 1;
		other.height = 1;
		other.internalFormat = 0;
		other.format = 0;
		other.formatType = 0;
	}

	return *this;
}

PostPass::PostPass(const PostPass& other)
{
	this->shader = other.shader;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->internalFormat = other.internalFormat;
	this->format = other.format;
	this->formatType = other.formatType;
}

PostPass& PostPass::operator=(const PostPass& other)
{
	this->shader = other.shader;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->internalFormat = other.internalFormat;
	this->format = other.format;
	this->formatType = other.formatType;

	return *this;
}

void PostPass::cleanup()
{
	if (targetTex)
	{
		//glDeleteFramebuffers(1, &targetFBO);

		glDeleteTextures(1, &targetTex);
		targetTex = 0;

		width = 1;
		height = 1;
		internalFormat = 0;
		format = 0;
		formatType = 0;
	}
}

PostPass::~PostPass()
{
	cleanup();
}

const fdm::Shader* PostPass::loadPassShader(const stl::string& name, const stl::string& fragmentPath)
{
	if (ShaderManager::shaders.contains(name))
		return ShaderManager::get(name);

	return ShaderManager::load(name, "assets/shaders/pass.vert", std::format("../../{}", fragmentPath));
}

PostPassGroup::PostPassGroup(PostPassGroup&& other) noexcept
{
	this->passes = other.passes;
	this->uniforms = other.uniforms;
	this->uniformTextures = other.uniformTextures;
	this->viewportMode = other.viewportMode;
	this->iteration = other.iteration;
	this->blending = other.blending;
	this->targetFBO = other.targetFBO;

	other.passes.clear();
	other.uniforms.clear();
	other.uniformTextures.clear();
	other.viewportMode = ViewportMode::CURRENT_PASS_SIZE;
	other.iteration = PassIteration{};
	other.blending = Blending{};
	other.targetFBO = 0;
}
PostPassGroup& PostPassGroup::operator=(PostPassGroup&& other) noexcept
{
	if (this != &other)
	{
		this->passes = other.passes;
		this->uniforms = other.uniforms;
		this->uniformTextures = other.uniformTextures;
		this->viewportMode = other.viewportMode;
		this->iteration = other.iteration;
		this->blending = other.blending;
		this->targetFBO = other.targetFBO;

		other.passes.clear();
		other.uniforms.clear();
		other.uniformTextures.clear();
		other.viewportMode = ViewportMode::CURRENT_PASS_SIZE;
		other.iteration = PassIteration{};
		other.blending = Blending{};
		other.targetFBO = 0;
	}

	return *this;
}
PostPassGroup::PostPassGroup(const PostPassGroup& other)
{
	this->passes = other.passes;
	this->uniforms = other.uniforms;
	this->uniformTextures = other.uniformTextures;
	this->viewportMode = other.viewportMode;
	this->iteration = other.iteration;
	this->blending = other.blending;
	this->targetFBO = other.targetFBO;
}
PostPassGroup& PostPassGroup::operator=(const PostPassGroup& other)
{
	this->passes = other.passes;
	this->uniforms = other.uniforms;
	this->uniformTextures = other.uniformTextures;
	this->viewportMode = other.viewportMode;
	this->iteration = other.iteration;
	this->blending = other.blending;
	this->targetFBO = other.targetFBO;

	return *this;
}
