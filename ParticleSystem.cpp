#include "include/fxlib/FXLib.h"
#include "include/fxlib/ParticleSystem.h"

using namespace FX;
using namespace fdm;

const FX::Shader* FX::ParticleSystem::defaultShader = nullptr;

ParticleSystem::ParticleSystem(const glm::vec4& origin, RND<float> lifetime, ParticleSpace particleSpace, size_t maxParticles)
	: origin(origin),
	lifetime(lifetime),
	particleSpace(particleSpace),
	maxParticles(maxParticles),
	trailRenderer(500, maxParticles)
{
	trailRenderer.user = this;
	trailRenderer.billboard = true;
	setMaxParticles(maxParticles);
}

ParticleSystem::ParticleSystem(
	const glm::vec4& origin,
	const glm::vec4& gravity,
	const glm::vec4& drag,
	float lifetimeMin,
	float lifetimeMax,
	const glm::vec4& startVelocityMin,
	const glm::vec4& startVelocityMax,
	const glm::vec4& velocityDeviationMin,
	const glm::vec4& velocityDeviationMax,
	const glm::vec4& startScaleMin,
	const glm::vec4& startScaleMax,
	const glm::vec4& endScaleMin,
	const glm::vec4& endScaleMax,
	const m4::Rotor& startRotMin,
	const m4::Rotor& startRotMax,
	const m4::Rotor& endRotMin,
	const m4::Rotor& endRotMax,
	bool angleTowardsVelocity,
	const glm::vec4& startColor,
	const glm::vec4& endColor,
	ParticleSpace particleSpace,
	size_t maxParticles)
	: origin(origin),
	gravity(gravity),
	drag(drag),
	lifetime(lifetimeMin, lifetimeMax),
	startVelocity(startVelocityMin, startVelocityMax),
	velocityDeviation(velocityDeviationMin, velocityDeviationMax),
	startScale(startScaleMin, startScaleMax),
	endScale(endScaleMin, endScaleMax),
	startRot(startRotMin, startRotMax),
	endRot(endRotMin, endRotMax),
	startColor(startColor),
	endColor(endColor),
	angleTowardsVelocity(angleTowardsVelocity),
	particleSpace(particleSpace),
	maxParticles(maxParticles),
	trailRenderer(500, maxParticles)
{
	trailRenderer.user = this;
	trailRenderer.billboard = true;
	setMaxParticles(maxParticles);
}

ParticleSystem::ParticleSystem(
	const glm::vec4& origin,
	const glm::vec4& gravity,
	const glm::vec4& drag,
	const RND<float>& lifetime,
	const RND<glm::vec4>& startVelocity,
	const RND<glm::vec4>& velocityDeviation,
	const RND<glm::vec4>& startScale,
	const RND<glm::vec4>& endScale,
	const RND<m4::Rotor>& startRot,
	const RND<m4::Rotor>& endRot,
	bool angleTowardsVelocity,
	const glm::vec4& startColor,
	const glm::vec4& endColor,
	ParticleSpace particleSpace,
	size_t maxParticles)
	: origin(origin),
	gravity(gravity),
	drag(drag),
	lifetime(lifetime),
	startVelocity(startVelocity),
	velocityDeviation(velocityDeviation),
	startScale(startScale),
	endScale(endScale),
	startRot(startRot),
	endRot(endRot),
	startColor(startColor),
	endColor(endColor),
	angleTowardsVelocity(angleTowardsVelocity),
	particleSpace(particleSpace),
	maxParticles(maxParticles),
	trailRenderer(500, maxParticles)
{
	trailRenderer.user = this;
	trailRenderer.billboard = true;
	setMaxParticles(maxParticles);
}

void ParticleSystem::initRenderer(const Mesh* mesh, const fdm::Shader* particleShader, const fdm::Shader* trailShader)
{
	this->particleShader = particleShader;
	this->trailShader = trailShader;
	renderer.setMesh(mesh);
	renderer.setDataSize(sizeof(ParticleData));
	renderer.setCount(maxParticles);
	trailRenderer.initRenderer();
}

void ParticleSystem::update(double dt)
{
	for (size_t i = 0; i < particles.size();)
	{
		Particle& p = particles[i];
		ParticleData& pData = gpuData[i];
		if (p.time <= p.lifetime)
		{
			updateParticle(p, pData, i, dt);
			++i;
		}
		else
		{
			std::swap(p, particles.back());
			trailRenderer.swapTrails(p.trailID, particles.back().trailID);
			p.trailID = particles.back().trailID;
			std::swap(pData, *(gpuData.begin() + (particles.size() - 1)));
			particles.pop_back();
		}
	}
	if (trails)
	{
		trailRenderer.update();
	}
}

void ParticleSystem::render(const m4::Mat5& view)
{
	if (trails)
	{
		trailRenderer.updateMesh(
			glm::vec4(view[0][0], view[1][0], view[2][0], view[3][0]),
			glm::vec4(view[0][1], view[1][1], view[2][1], view[3][1]),
			glm::vec4(view[0][2], view[1][2], view[2][2], view[3][2]),
			glm::vec4(view[0][3], view[1][3], view[2][3], view[3][3]));
		trailShader->use();
		((const FX::Shader*)trailShader)->setUniform("MV", view); // compat
		((const FX::Shader*)trailShader)->setUniform("view", view);
		trailRenderer.setMode(GL_LINES_ADJACENCY);
		trailRenderer.render();
	}

	particleShader->use();
	((const FX::Shader*)particleShader)->setUniform("MV", view); // compat
	((const FX::Shader*)particleShader)->setUniform("view", view);
	((const FX::Shader*)particleShader)->setUniform("billboard", billboard);
	renderer.updateData(gpuData.data(), particles.size());
	renderer.render();
}

ParticleSystem::Particle* ParticleSystem::emit(size_t count)
{
	int cCount = glm::min(count, maxParticles - particles.size());

	if (cCount == 0)
		return nullptr;

	for (int i = 0; i < cCount; i++)
	{
		Particle& p = particles.emplace_back();

		if (trails)
		{
			trailRenderer.clearPoints(particles.size() - 1);
			p.trailID = particles.size() - 1;
		}

		if (particleSpace == GLOBAL)
			p.pos = origin;

		p.velDeviation = velocityDeviation.evalValue();
		p.lifetime = glm::max(lifetime.evalValue(), 0.001f);
		ParticleData& pData = gpuData[particles.size() - 1];
		pData.t = 0;

		p.startScale = startScale.evalValue();
		p.endScale = endScale.evalValue();
		p.startColor = startColor;
		p.endColor = endColor;
		pData.color = p.startColor;
		pData.scale = p.startScale;

		if (emitFunc)
			emitFunc(this, p, pData, i);
		else
		{
			p.vel = startVelocity.evalValue();
			switch (spawnMode)
			{
			case BOX:
			{
				glm::vec4 offset
				{
					utils::random(-spawnBoxParams.size.x * 0.5f, spawnBoxParams.size.x * 0.5f),
					utils::random(-spawnBoxParams.size.y * 0.5f, spawnBoxParams.size.y * 0.5f),
					utils::random(-spawnBoxParams.size.z * 0.5f, spawnBoxParams.size.z * 0.5f),
					utils::random(-spawnBoxParams.size.w * 0.5f, spawnBoxParams.size.w * 0.5f)
				};
				p.pos += offset;
			} break;
			case SPHERE:
			{
				glm::vec4 dir = glm::normalize(utils::random(glm::vec4{ -1 }, glm::vec4{ 1 }));

				p.pos += dir * spawnSphereParams.radius;
				p.vel += dir * spawnSphereParams.force;
			} break;
			}
		}

		//updateParticle(p, pData, i, 0);
		pData.model = p.mat;

		if (particleSpace == LOCAL)
			pData.pos() += origin;
	}
	lastEmitTime = glfwGetTime();

	return &particles.back();
}

void ParticleSystem::setMaxParticles(size_t maxParticles)
{
	this->maxParticles = maxParticles;

	particles.reserve(maxParticles);
	gpuData.resize(maxParticles);
	trailRenderer.setTrailsCount(maxParticles);

	if (particles.size() > maxParticles)
	{
		particles.resize(maxParticles);
		renderer.setCount(maxParticles);
	}
}

ParticleSystem& ParticleSystem::operator=(const ParticleSystem& other)
{
	this->particleShader = other.particleShader;
	this->trailShader = other.trailShader;
	this->origin = other.origin;
	this->gravity = other.gravity;
	this->drag = other.drag;
	this->lifetime = other.lifetime;
	this->startVelocity = other.startVelocity;
	this->velocityDeviation = other.velocityDeviation;
	this->startScale = other.startScale;
	this->endScale = other.endScale;
	this->startRot = other.startRot;
	this->endRot = other.endRot;
	this->angleTowardsVelocity = other.angleTowardsVelocity;
	this->startColor = other.startColor;
	this->endColor = other.endColor;
	this->particleSpace = other.particleSpace;
	this->spawnBoxParams = other.spawnBoxParams;
	this->spawnSphereParams = other.spawnSphereParams;
	this->spawnMode = other.spawnMode;
	this->maxParticles = other.maxParticles;
	this->particles = other.particles;
	this->gpuData = other.gpuData;
	this->evalFunc = other.evalFunc;
	this->emitFunc = other.emitFunc;
	this->user = other.user;
	this->trailRenderer = other.trailRenderer;
	this->trails = other.trails;
	trailRenderer.user = this;

	setMaxParticles(maxParticles);

	return *this;
}

ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) noexcept
{
	this->particleShader = other.particleShader;
	this->trailShader = other.trailShader;
	this->origin = other.origin;
	this->gravity = other.gravity;
	this->drag = other.drag;
	this->lifetime = other.lifetime;
	this->startVelocity = other.startVelocity;
	this->velocityDeviation = other.velocityDeviation;
	this->startScale = other.startScale;
	this->endScale = other.endScale;
	this->startRot = other.startRot;
	this->endRot = other.endRot;
	this->angleTowardsVelocity = other.angleTowardsVelocity;
	this->startColor = other.startColor;
	this->endColor = other.endColor;
	this->particleSpace = other.particleSpace;
	this->spawnBoxParams = other.spawnBoxParams;
	this->spawnSphereParams = other.spawnSphereParams;
	this->spawnMode = other.spawnMode;
	this->maxParticles = other.maxParticles;
	this->particles = other.particles;
	this->gpuData = other.gpuData;
	this->evalFunc = other.evalFunc;
	this->emitFunc = other.emitFunc;
	this->user = other.user;
	this->trailRenderer = other.trailRenderer;
	this->trails = other.trails;
	trailRenderer.user = this;

	other.particleShader = nullptr;
	other.trailShader = nullptr;
	other.origin = glm::vec4{ 0 };
	other.gravity = glm::vec4{ 0 };
	other.drag = glm::vec4{ 0 };
	other.lifetime = RND<float>{ 1.f };
	other.startVelocity = RND<glm::vec4>{ glm::vec4{ 0 } };
	other.velocityDeviation = RND<glm::vec4>{ glm::vec4{ 0 } };
	other.startScale = RND<glm::vec4>{ glm::vec4{ 1 } };
	other.endScale = RND<glm::vec4>{ glm::vec4{ 0 } };
	other.startRot = RND<m4::Rotor>{ m4::Rotor{ {0,0,0,0,0,0}, 0.f } };
	other.endRot = RND<m4::Rotor>{ m4::Rotor{ {0,0,0,0,0,0}, 0.f } };
	other.angleTowardsVelocity = false;
	other.startColor = glm::vec4{ 0 };
	other.endColor = glm::vec4{ 0 };
	other.particleSpace = GLOBAL;
	other.spawnBoxParams = {};
	other.spawnSphereParams = {};
	other.spawnMode = BOX;
	other.maxParticles = 0;
	other.particles.clear();
	other.gpuData.clear();
	other.evalFunc = nullptr;
	other.emitFunc = nullptr;
	other.user = nullptr;
	other.trailRenderer.setTrailsCount(0);
	other.trails = false;

	setMaxParticles(maxParticles);

	return *this;
}

void ParticleSystem::updateParticle(Particle& p, ParticleData& pData, size_t i, double dt)
{
	p.time += dt;

	pData.t = p.time / p.lifetime;

	if (evalFunc)
	{
		evalFunc(this, p, pData, i, dt);
	}
	else
	{
		p.vel += gravity * (float)dt;
		p.vel += p.velDeviation * (float)dt * pData.t;
		p.vel += -drag * p.vel * (float)dt;
		pData.scale = utils::lerp(p.startScale, p.endScale, pData.t);
		pData.color = utils::lerp(p.startColor, p.endColor, pData.t);
	}

	p.pos += p.vel * (float)dt;

	pData.model = p.mat;

	if (!evalFunc)
		pData.model *= utils::slerp(startRot, endRot, pData.t);

	if (angleTowardsVelocity)
		pData.model *= m4::Rotor({ 0,0,1,0 }, glm::normalize(p.vel));

	pData.pos() += p.pos;
	if (particleSpace == LOCAL)
		pData.pos() += origin;

	if (trails)
	{
		trailRenderer.setTrailPos(p.trailID, pData.pos(), *(glm::vec4*)pData.model[0], *(glm::vec4*)pData.model[1]);
	}
}
