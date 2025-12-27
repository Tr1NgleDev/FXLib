#include <4dm.h>

using namespace fdm;

#include "include/fxlib/FXLib.h"

static bool initializedParticles = false;
static FX::ParticleSystem particles;

$hook(void, StateGame, update, StateManager& s, double dt)
{
	double time = glfwGetTime();
	static double lastEmission = 0;
	if (time - lastEmission >= 0.04)
	{
		particles.emit();
		lastEmission = time;
	}

	particles.update(dt);

	return original(self, s, dt);
}
$hook(void, WorldManager, render, const m4::Mat5& MV, bool glasses, glm::vec3 worldColor)
{
	original(self, MV, glasses, worldColor);
	if (initializedParticles)
		particles.render(MV);
}

$hook(void, StateGame, init, StateManager& s)
{
	if (!initializedParticles)
	{
		particles = FX::ParticleSystem
		{
			{ 0,10,0,0 },
			{ 0,-12,0,0 },
			glm::vec4{ 0 },
			{ 2.0f, 0.5f },
			{ glm::vec4{ -0.5f, 5, -0.5f, -0.5f }, glm::vec4{ 0.5f, 10, 0.5f, 0.5f } },
			glm::vec4{ 0 },
			{ glm::vec4{ 0.3f }, glm::vec4{ 0.5f } },
			{ glm::vec4{ 0.2f }, glm::vec4{ 0.0f } },
			m4::Rotor{},
			m4::Rotor{},
			true,
			glm::vec4{ 1, 1, 1, 1 },
			glm::vec4{ 1, 0, 0, 0 },
		};

		particles.trails = true;
		particles.setTrailBillboard(true);
		particles.setTrailColorFunc([](float t, float p, size_t id, void* user)
			{
				return ((FX::ParticleSystem*)user)->getParticleData()[id].color;
			});
		particles.setTrailWidthFunc([](float t, float p, size_t id, void* user)
			{
				return ((FX::ParticleSystem*)user)->getParticleData()[id].scale.x * (p * (1.f - t));
			});
		
		MeshBuilder particleMesh = MeshBuilder{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
		particleMesh.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
		particleMesh.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
		particleMesh.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));
		particles.initRenderer(&particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);

		initializedParticles = true;
	}

	FX::applyPostProcessing(self->renderFramebuffer, [](uint32_t fbo, uint32_t colorTex, uint32_t depthTex, int width, int height, std::vector<FX::PostPassGroup>& passes)
		{
			passes.emplace_back(FX::PostPass::loadPassShader("me.test.invertPass", std::format("{}/{}", fdm::getModPath(fdm::modID), "assets/shaders/invert.frag")));
		});

	return original(self, s);
}

$hook(void, StateGame, updateProjection, GLsizei width, GLsizei height)
{
	original(self, width, height);

	FX::ParticleSystem::defaultShader->setUniform("P", self->projection3D);
	FX::TrailRenderer::defaultShader->setUniform("P", self->projection3D);
}
