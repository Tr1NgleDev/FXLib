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
	std::vector<PostPassGroup>* passGroups{};
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

	if (s->passGroups)
	{
		s->passGroups->clear();
		delete s->passGroups;
		s->passGroups = nullptr;
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

	s->passGroups = new std::vector<PostPassGroup>();

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

	for (auto& callback : s->initCallbacks) callback(s->fbo, s->colorTex, s->depthTex, s->width, s->height, *s->passGroups);
}

$hook(void, Framebuffer, render)
{
	FB* s = (FB*)self;
	if (s->_magic_number != MAGIC_NUMBER)
		return original(self);

	uint32_t outputID = s->colorTex;
	auto& passGroups = *s->passGroups;
	if (!passGroups.empty())
	{
		int fb = NULL;
		int blendEquationRGB = NULL;
		int blendEquationA = NULL;
		int blendSrcRGB = NULL;
		int blendSrcA = NULL;
		int blendDstRGB = NULL;
		int blendDstA = NULL;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRGB);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationA);
		glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRGB);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcA);
		glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRGB);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstA);

		glDisable(GL_ALPHA_TEST);
		for (int i = 0; i < passGroups.size(); ++i)
		{
			PostPassGroup& group = passGroups[i];
			if (group.passes.empty()) continue;

			if (!group.targetFBO)
			{
				glCreateFramebuffers(1, &group.targetFBO);
			}

			if (group.blending.mode == PostPassGroup::Blending::DISABLED)
			{
				glDisable(GL_BLEND);
			}
			else
			{
				glEnable(GL_BLEND);
				switch (group.blending.mode)
				{
				case PostPassGroup::Blending::ADD:
					glBlendEquation(GL_FUNC_ADD);
					break;
				case PostPassGroup::Blending::SUBTRACT:
					glBlendEquation(GL_FUNC_SUBTRACT);
					break;
				case PostPassGroup::Blending::REVERSE_SUBTRACT:
					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
					break;
				case PostPassGroup::Blending::MIN:
					glBlendEquation(GL_MIN);
					break;
				case PostPassGroup::Blending::MAX:
					glBlendEquation(GL_MAX);
					break;
				}
				glBlendFunc(group.blending.func.srcFactor, group.blending.func.dstFactor);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, group.targetFBO);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			using enum PostPassGroup::PassIteration::Direction;

			auto renderPass = [&](int ind, int prevInd, int nextInd)
				{
					const PostPass& prevPass = group.passes[prevInd];
					PostPass& pass = group.passes[ind];
					const PostPass& nextPass = group.passes[nextInd];

					int prevPassWidth = glm::max(s->width / prevPass.sizeDiv, 1);
					int prevPassHeight = glm::max(s->height / prevPass.sizeDiv, 1);
					int passWidth = glm::max(s->width / pass.sizeDiv, 1);
					int passHeight = glm::max(s->height / pass.sizeDiv, 1);
					int nextPassWidth = glm::max(s->width / nextPass.sizeDiv, 1);
					int nextPassHeight = glm::max(s->height / nextPass.sizeDiv, 1);

					switch (group.viewportMode)
					{
					case PostPassGroup::CURRENT_PASS_SIZE:
						glViewport(0, 0, passWidth, passHeight);
						break;
					case PostPassGroup::PREV_PASS_SIZE:
						glViewport(0, 0, prevPassWidth, prevPassHeight);
						break;
					case PostPassGroup::NEXT_PASS_SIZE:
						glViewport(0, 0, nextPassWidth, nextPassHeight);
						break;
					}

					if (pass.targetTex == 0 || pass.width != passWidth || pass.height != passHeight)
						pass.initTexture(passWidth, passHeight, GL_RGBA, GL_RGBA, GL_FLOAT);

					glNamedFramebufferTexture(group.targetFBO, GL_COLOR_ATTACHMENT0, pass.targetTex, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					pass.shader->use();

					glBindTextureUnit(0, s->colorTex);
					glProgramUniform1i(pass.shader->id(), glGetUniformLocation(pass.shader->id(), "source"), 0);

					glBindTextureUnit(1, s->depthTex);
					glProgramUniform1i(pass.shader->id(), glGetUniformLocation(pass.shader->id(), "sourceDepth"), 1);

					glBindTextureUnit(2, outputID);
					glProgramUniform1i(pass.shader->id(), glGetUniformLocation(pass.shader->id(), "prevPassGroup"), 2);

					glProgramUniform4f(pass.shader->id(), glGetUniformLocation(pass.shader->id(), "sourceSize"),
						s->width, s->height, 1.0f / s->width, 1.0f / s->height);

					int j = 0;
					for (int l = 0; l < group.passes.size(); ++l)
					{
						const PostPass& otherPass = group.passes[l];
						if (!otherPass.targetTex) continue;
						glBindTextureUnit(j + 3, otherPass.targetTex);
						{
							int loc = glGetUniformLocation(pass.shader->id(), std::format("pass{}", l).c_str());
							if (loc != -1)
								glProgramUniform1i(pass.shader->id(), loc, j + 3);
						}
						{
							int loc = glGetUniformLocation(pass.shader->id(), std::format("pass{}_size", l).c_str());
							if (loc != -1)
								glProgramUniform4f(pass.shader->id(), loc, otherPass.width, otherPass.height, 1.0f / otherPass.width, 1.0f / otherPass.height);
						}
						if (l == prevInd && ind != prevInd)
						{
							{
								int loc = glGetUniformLocation(pass.shader->id(), "prevPass");
								if (loc != -1)
									glProgramUniform1i(pass.shader->id(), loc, j + 3);
							}
							{
								int loc = glGetUniformLocation(pass.shader->id(), "prevPass_size");
								if (loc != -1)
									glProgramUniform4f(pass.shader->id(), loc, otherPass.width, otherPass.height, 1.0f / otherPass.width, 1.0f / otherPass.height);
							}
						}
						++j;
					}
					// fallback prevPass to outputID (prevPassGroup/default)
					if (ind == prevInd)
					{
						glBindTextureUnit(j + 3, outputID);
						{
							int loc = glGetUniformLocation(pass.shader->id(), "prevPass");
							if (loc != -1)
								glProgramUniform1i(pass.shader->id(), loc, j + 3);
						}
						{
							int loc = glGetUniformLocation(pass.shader->id(), "prevPass_size");
							if (loc != -1)
								glProgramUniform4f(pass.shader->id(), loc, s->width, s->height, 1.0f / s->width, 1.0f / s->height);
						}
						++j;
					}

					for (auto& tex : group.uniformTextures)
					{
						glBindTextureUnit(j + 3, tex.second);
						{
							int loc = glGetUniformLocation(pass.shader->id(), tex.first.c_str());
							if (loc != -1)
								glProgramUniform1i(pass.shader->id(), loc, j + 3);
						}
						++j;
					}

					for (int k = 0; k < i; ++k)
					{
						const PostPassGroup& _group = passGroups[k];
						int k_ = i - k;
						std::string prefix = std::string(k_, 'p');
						std::string name = std::format("{}_group", prefix);
						glBindTextureUnit(j + 3, _group.outputTex);
						{
							int loc = glGetUniformLocation(pass.shader->id(), name.c_str());
							glProgramUniform1i(pass.shader->id(), loc, j + 3);
							++j;
						}
						int l = 0;
						for (auto& p : _group.passes)
						{
							glBindTextureUnit(j + 3, p.targetTex);
							{
								int loc = glGetUniformLocation(pass.shader->id(), std::format("{}_pass{}", name, l++).c_str());
								if (loc != -1)
									glProgramUniform1i(pass.shader->id(), loc, j + 3);
							}
							{
								int loc = glGetUniformLocation(pass.shader->id(), std::format("{}_pass{}_size", name, l).c_str());
								if (loc != -1)
									glProgramUniform4f(pass.shader->id(), loc, p.width, p.height, 1.0f / p.width, 1.0f / p.height);
							}
							if (l == ind)
							{
								{
									int loc = glGetUniformLocation(pass.shader->id(), std::format("{}_passInd", name).c_str());
									if (loc != -1)
										glProgramUniform1i(pass.shader->id(), loc, j + 3);
								}
								{
									int loc = glGetUniformLocation(pass.shader->id(), std::format("{}_passInd_size", name).c_str());
									if (loc != -1)
										glProgramUniform4f(pass.shader->id(), loc, p.width, p.height, 1.0f / p.width, 1.0f / p.height);
								}
							}
							++j;
						}
					}
					{
						{
							std::string prefix = std::string(i + 1, 'p');
							int loc = glGetUniformLocation(pass.shader->id(), std::format("{}_group", prefix).c_str());
							if (loc != -1)
								glProgramUniform1i(pass.shader->id(), loc, 0);
							++j;
						}
					}

					for (auto& uniform : group.uniforms)
					{
						int loc = glGetUniformLocation(pass.shader->id(), uniform.name.c_str());
						if (loc == -1)
							continue;

						switch (uniform.type)
						{
						case Uniform::FLOAT:
							glProgramUniform1fv(pass.shader->id(), loc, 1, (float*)uniform.value);
							break;
						case Uniform::VEC2:
							glProgramUniform2fv(pass.shader->id(), loc, 1, (float*)uniform.value);
							break;
						case Uniform::VEC3:
							glProgramUniform3fv(pass.shader->id(), loc, 1, (float*)uniform.value);
							break;
						case Uniform::VEC4:
							glProgramUniform4fv(pass.shader->id(), loc, 1, (float*)uniform.value);
							break;
						case Uniform::INT:
							glProgramUniform1iv(pass.shader->id(), loc, 1, (int*)uniform.value);
							break;
						case Uniform::IVEC2:
							glProgramUniform2iv(pass.shader->id(), loc, 1, (int*)uniform.value);
							break;
						case Uniform::IVEC3:
							glProgramUniform3iv(pass.shader->id(), loc, 1, (int*)uniform.value);
							break;
						case Uniform::IVEC4:
							glProgramUniform4iv(pass.shader->id(), loc, 1, (int*)uniform.value);
							break;
						case Uniform::UINT:
							glProgramUniform1uiv(pass.shader->id(), loc, 1, (uint32_t*)uniform.value);
							break;
						case Uniform::UVEC2:
							glProgramUniform2uiv(pass.shader->id(), loc, 1, (uint32_t*)uniform.value);
							break;
						case Uniform::UVEC3:
							glProgramUniform3uiv(pass.shader->id(), loc, 1, (uint32_t*)uniform.value);
							break;
						case Uniform::UVEC4:
							glProgramUniform4uiv(pass.shader->id(), loc, 1, (uint32_t*)uniform.value);
							break;
						}
					}

					glBindVertexArray(passRenderer.VAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				};

			using enum PostPassGroup::PassIteration::Count;
			int last = group.passes.size() - (group.iteration.count == SKIP_LAST ? 2 : 1);
			int first = (group.iteration.count == SKIP_FIRST ? 1 : 0);

			switch (group.iteration.dir)
			{
			case FORWARD:
				for (int j = first; j <= last; ++j)
				{
					int prev = glm::clamp(j - 1, 0, (int)group.passes.size() - 1);
					int next = glm::clamp(j + 1, 0, (int)group.passes.size() - 1);
					renderPass(j, prev, next);
				}
				outputID = group.outputTex = group.passes[last].targetTex;
				break;
			case BACKWARD:
				for (int j = last; j >= first; --j)
				{
					int prev = glm::clamp(j + 1, 0, (int)group.passes.size() - 1);
					int next = glm::clamp(j - 1, 0, (int)group.passes.size() - 1);
					renderPass(j, prev, next);
				}
				outputID = group.outputTex = group.passes[first].targetTex;
				break;
			}
		}

		glEnable(GL_BLEND);
		glBlendFuncSeparate(blendSrcRGB, blendDstRGB, blendSrcA, blendDstA);
		glBlendEquationSeparate(blendEquationRGB, blendEquationA);
		glEnable(GL_ALPHA_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, fb);
	}

	s->shader->use();

	glBindTextureUnit(0, outputID);
	glProgramUniform1i(s->shader->id(), glGetUniformLocation(s->shader->id(), "texture"), 0);

	glBindTextureUnit(1, s->depthTex);
	glProgramUniform1i(s->shader->id(), glGetUniformLocation(s->shader->id(), "depth"), 1);

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
		s->passGroups = nullptr;
		s->initCallbacks = std::vector<FramebufferInitCallback>{};
	}
	for (auto& callback : s->initCallbacks)
	{
		if (callback == initCallback)
			return;
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
