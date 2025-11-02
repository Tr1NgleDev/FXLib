#include "include/fxlib/FXLib.h"

using namespace FX;
using namespace fdm;

inline static constexpr uint32_t MAGIC_NUMBER = 1529352098;

struct
{
	inline static int8_t quadVertices[]
	{
		-1, -1,
		-1,  1,
		 1,  1,
		-1, -1,
		 1,  1,
		 1, -1,
	};

	uint32_t VAO = 0, VBO = 0;
} passRenderer;
static TexRenderer fbRenderer{};
static Tex2D fbRendererTex{};

class FB
{
public:
	uint32_t fbo = 0;
	uint32_t colorTex = 0;
	int width = 1;
	int height = 1;
	uint32_t _magic_number = MAGIC_NUMBER;
	uint32_t depthTex = 0;
	PAD(8);
	std::vector<PostPass>* passes{};
	void* texPtr = &colorTex;
	const fdm::Shader* shader = nullptr;
	std::vector<FramebufferInitCallback> initCallbacks{};
};

inline static std::set<FB*> framebuffers{};

$hook(void, Framebuffer, cleanup)
{
	FB* s = (FB*)self;
	if (s->_magic_number != MAGIC_NUMBER)
		return original(self);

	if (s->fbo)
	{
		glDeleteFramebuffers(1, &s->fbo);
		s->fbo = NULL;

		glDeleteTextures(1, &s->colorTex);
		s->colorTex = NULL;

		glDeleteTextures(1, &s->depthTex);
		s->depthTex = NULL;

		s->width = 1;
		s->height = 1;
	}

	if (s->passes)
	{
		s->passes->clear();
		delete s->passes;
		s->passes = nullptr;
	}

	framebuffers.erase(s);
}

$hook(void, Framebuffer, destr_Framebuffer)
{
	FB* s = (FB*)self;
	if (s->_magic_number != MAGIC_NUMBER)
		return original(self);

	return self->cleanup();
}

$hook(void, Framebuffer, init, GLsizei width, GLsizei height, bool alphaChannel)
{
	FB* s = (FB*)self;
	if (s->_magic_number != MAGIC_NUMBER)
		return original(self, width, height, alphaChannel);

	uint32_t format = alphaChannel ? GL_RGBA : GL_RGB;

	self->cleanup();

	s->width = width;
	s->height = height;

	s->passes = new std::vector<PostPass>();

	glGenFramebuffers(1, &s->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);

	{
		glGenTextures(1, &s->colorTex);
		glBindTexture(GL_TEXTURE_2D, s->colorTex);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, nullptr); // 4dm actually does GL_UNSIGNED_BYTE but i changed it to GL_FLOAT for hdr ig idk
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // 4dm doesn't even set this lol
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); // 4dm doesn't even set this lol
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s->colorTex, 0);
	}

	{
		glGenTextures(1, &s->depthTex);
		glBindTexture(GL_TEXTURE_2D, s->depthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s->depthTex, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	framebuffers.insert(s);

	for (auto& callback : s->initCallbacks) callback(s->fbo, s->colorTex, s->depthTex, s->width, s->height, *s->passes);
}

$hook(void, Framebuffer, render)
{
	FB* s = (FB*)self;
	if (s->_magic_number != MAGIC_NUMBER)
		return original(self);

	uint32_t outputID = s->colorTex;
	auto& passes = *s->passes;
	if (!passes.empty())
	{
		int fb = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		for (int i = 0; i < passes.size(); ++i)
		{
			PostPass& pass = passes[i];
			if (pass.width != s->width || pass.height != s->height)
				pass.initTarget(s->width, s->height, GL_RGBA, GL_RGBA, GL_FLOAT);

			glBindFramebuffer(GL_FRAMEBUFFER, pass.targetFBO);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			pass.shader->use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, s->colorTex);
			glUniform1i(glGetUniformLocation(pass.shader->id(), "source"), 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, s->depthTex);
			glUniform1i(glGetUniformLocation(pass.shader->id(), "sourceDepth"), 1);

			int j = 0;
			for (; j < i; ++j)
			{
				glActiveTexture(GL_TEXTURE2 + j);
				glBindTexture(GL_TEXTURE_2D, passes[j].targetTex);
				glUniform1i(glGetUniformLocation(pass.shader->id(), std::format("pass{}", j).c_str()), j + 2);
			}

			for (auto& tex : pass.textures)
			{
				glActiveTexture(GL_TEXTURE2 + j);
				glBindTexture(GL_TEXTURE_2D, tex.second);
				glUniform1i(glGetUniformLocation(pass.shader->id(), tex.first.c_str()), j + 2);
				++j;
			}

			glUniform4f(glGetUniformLocation(pass.shader->id(), "sourceSize"), s->width, s->height, 1.0f / s->width, 1.0f / s->height);

			glBindVertexArray(passRenderer.VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);

		outputID = passes.back().targetTex;

		glBindFramebuffer(GL_FRAMEBUFFER, fb);
	}

	s->shader->use();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, s->depthTex);
	glUniform1i(glGetUniformLocation(s->shader->id(), "depth"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outputID);
	glUniform1i(glGetUniformLocation(s->shader->id(), "texture"), 0);

	fbRendererTex.ID = outputID;
	fbRendererTex.width = s->width;
	fbRendererTex.height = s->height;
	fbRenderer.setPos(0, s->height, s->width, -s->height);
	fbRenderer.render();
}

void FX::applyPostProcessing(fdm::Framebuffer& fb, FramebufferInitCallback initCallback)
{
	FB* s = (FB*)&fb;
	if (s->_magic_number != MAGIC_NUMBER)
	{
		s->_magic_number = MAGIC_NUMBER;
		s->passes = nullptr;
		s->initCallbacks = std::vector<FramebufferInitCallback>{};
	}
	s->initCallbacks.emplace_back(initCallback);
}

$hookStatic(bool, ShaderManager, loadFromShaderList, const stl::string& jsonListPath)
{
	bool result = original(jsonListPath);

	if (!FX::hasInitializedContext())
	{
		return result;
	}

	if (!passRenderer.VAO)
	{
		{
			glGenVertexArrays(1, &passRenderer.VAO);
			glGenBuffers(1, &passRenderer.VBO);

			glBindVertexArray(passRenderer.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, passRenderer.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(passRenderer.quadVertices), passRenderer.quadVertices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 2, GL_BYTE, GL_FALSE, 2 * sizeof(int8_t), (void*)0);
			glEnableVertexAttribArray(0);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		{
			fbRendererTex.ID = 0;
			fbRendererTex.width = 1;
			fbRendererTex.height = 1;

			fbRenderer = TexRenderer{ &fbRendererTex, ShaderManager::get("postShader") };
			fbRenderer.init();
		}
	}

	return result;
}
