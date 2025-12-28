#include "include/fxlib/FXLib.h"
#include "include/fxlib/PostPass.h"

using namespace FX;
using namespace fdm;

void PostPass::initTexture(int width, int height)
{
	cleanup();

	this->width = width;
	this->height = height;
	GLenum internalFormat = 0;
	GLenum format = 0;
	GLenum formatType = GL_FLOAT;

	switch (passFormat)
	{
	case FX::PostPass::R:
		internalFormat = GL_R16F;
		format = GL_RED;
		break;
	case FX::PostPass::RG:
		internalFormat = GL_RG16F;
		format = GL_RG;
		break;
	case FX::PostPass::RGB:
		internalFormat = GL_RGB16F;
		format = GL_RGB;
		break;
	case FX::PostPass::RGBA:
		internalFormat = GL_RGBA16F;
		format = GL_RGBA;
		break;
	}

	{
		glGenTextures(1, &targetTex);
		glBindTexture(GL_TEXTURE_2D, targetTex);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, formatType, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

PostPass::PostPass(PostPass&& other) noexcept
{
	this->shader = other.shader;
	this->targetTex = other.targetTex;
	this->width = other.width;
	this->height = other.height;
	this->sizeDiv = other.sizeDiv;
	this->passFormat = other.passFormat;

	other.shader = nullptr;
	other.targetTex = 0;
	other.width = 1;
	other.height = 1;
	other.sizeDiv = 1;
	other.passFormat = RGBA;
}

PostPass& PostPass::operator=(PostPass&& other) noexcept
{
	if (this != &other)
	{
		this->shader = other.shader;
		this->targetTex = other.targetTex;
		this->width = other.width;
		this->height = other.height;
		this->sizeDiv = other.sizeDiv;
		this->passFormat = other.passFormat;

		other.shader = nullptr;
		other.targetTex = 0;
		other.width = 1;
		other.height = 1;
		other.sizeDiv = 1;
		other.passFormat = RGBA;
	}

	return *this;
}

PostPass::PostPass(const PostPass& other)
{
	this->shader = other.shader;
	this->width = other.width;
	this->height = other.height;
	this->sizeDiv = other.sizeDiv;
	this->passFormat = other.passFormat;
}

PostPass& PostPass::operator=(const PostPass& other)
{
	this->shader = other.shader;
	this->width = other.width;
	this->height = other.height;
	this->sizeDiv = other.sizeDiv;
	this->passFormat = other.passFormat;

	return *this;
}

void PostPass::cleanup()
{
	if (targetTex)
	{
		glDeleteTextures(1, &targetTex);
		targetTex = 0;

		width = 1;
		height = 1;
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

PostPassGroup::~PostPassGroup()
{
	if (targetFBO)
	{
		glDeleteFramebuffers(1, &targetFBO);
		targetFBO = 0;
	}
	outputTex = 0;
	passes.clear();
	uniforms.clear();
	uniformTextures.clear();
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
	this->outputTex = other.outputTex;
	this->copyLastGroup = other.copyLastGroup;
	this->clearColor = other.clearColor;

	other.passes.clear();
	other.uniforms.clear();
	other.uniformTextures.clear();
	other.viewportMode = ViewportMode::CURRENT_PASS_SIZE;
	other.iteration = PassIteration{};
	other.blending = Blending{};
	other.targetFBO = 0;
	other.outputTex = 0;
	other.copyLastGroup = false;
	other.clearColor = true;
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
		this->outputTex = other.outputTex;
		this->copyLastGroup = other.copyLastGroup;
		this->clearColor = other.clearColor;

		other.passes.clear();
		other.uniforms.clear();
		other.uniformTextures.clear();
		other.viewportMode = ViewportMode::CURRENT_PASS_SIZE;
		other.iteration = PassIteration{};
		other.blending = Blending{};
		other.targetFBO = 0;
		other.outputTex = 0;
		other.copyLastGroup = false;
		other.clearColor = true;
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
	this->copyLastGroup = other.copyLastGroup;
	this->clearColor = other.clearColor;
}
PostPassGroup& PostPassGroup::operator=(const PostPassGroup& other)
{
	this->passes = other.passes;
	this->uniforms = other.uniforms;
	this->uniformTextures = other.uniformTextures;
	this->viewportMode = other.viewportMode;
	this->iteration = other.iteration;
	this->blending = other.blending;
	this->copyLastGroup = other.copyLastGroup;
	this->clearColor = other.clearColor;

	return *this;
}
