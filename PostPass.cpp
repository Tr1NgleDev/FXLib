#include "include/fxlib/FXLib.h"
#include "include/fxlib/PostPass.h"

using namespace FX;
using namespace fdm;

void PostPass::initTarget(int width, int height, uint32_t internalFormat, uint32_t format, uint32_t formatType)
{
	cleanup();

	glGenFramebuffers(1, &targetFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
	{
		glGenTextures(1, &targetTex);
		glBindTexture(GL_TEXTURE_2D, targetTex);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, formatType, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTex, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PostPass::PostPass(PostPass&& other) noexcept
{
	this->shader = other.shader;
	this->targetFBO = other.targetFBO;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->internalFormat = other.internalFormat;
	this->format = other.format;
	this->formatType = other.formatType;
	this->textures = other.textures;

	other.shader = nullptr;
	other.targetFBO = 0;
	other.targetTex = 0;
	other.width = 1;
	other.height = 1;
	other.internalFormat = 0;
	other.format = 0;
	other.formatType = 0;
	other.textures.clear();
}

PostPass& PostPass::operator=(PostPass&& other) noexcept
{
	if (this != &other)
	{
		this->shader = other.shader;
		this->targetFBO = other.targetFBO;
		this->targetTex = other.targetTex;
		this->width = other.width;
		this->height = other.height;
		this->internalFormat = other.internalFormat;
		this->format = other.format;
		this->formatType = other.formatType;
		this->textures = other.textures;

		other.shader = nullptr;
		other.targetFBO = 0;
		other.targetTex = 0;
		other.width = 1;
		other.height = 1;
		other.internalFormat = 0;
		other.format = 0;
		other.formatType = 0;
		other.textures.clear();
	}

	return *this;
}

PostPass::PostPass(const PostPass& other)
{
	this->shader = other.shader;
	this->targetFBO = other.targetFBO;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->internalFormat = other.internalFormat;
	this->format = other.format;
	this->formatType = other.formatType;
	this->textures = other.textures;
}

PostPass& PostPass::operator=(const PostPass& other)
{
	this->shader = other.shader;
	this->targetFBO = other.targetFBO;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->internalFormat = other.internalFormat;
	this->format = other.format;
	this->formatType = other.formatType;
	this->textures = other.textures;

	return *this;
}

void PostPass::cleanup()
{
	if (targetFBO)
	{
		glDeleteFramebuffers(1, &targetFBO);
		targetFBO = 0;

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
