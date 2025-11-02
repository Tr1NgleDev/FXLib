#pragma once

#include "FXLib.h"

#include <glm/gtc/random.hpp>

#include "TrailRenderer.h"
#include "InstancedMeshRenderer.h"

namespace FX
{
	template<typename T>
	class RND
	{
	private:
		T min, max;
		T val;
	public:
		T evalValue() { return val = utils::random(min, max); }
		RND(T minMax) : min(minMax), max(minMax) { evalValue(); }
		RND(T min, T max) : min(min), max(max) { evalValue(); }
		RND(const RND<T>& other) : min(other.min), max(other.max) { evalValue(); }
		RND(RND<T>&& other) : min(other.min), max(other.max) { evalValue(); }
		T getValue() const { return val; }
		T getMin() const { return min; }
		T getMax() const { return max; }
		void setMin(T min) { this->min = min; evalValue(); }
		void setMax(T max) { this->max = max; evalValue(); }
		void setMinMax(T minMax) { this->min = minMax; this->max = minMax; evalValue(); }
		void setMinMax(T min, T max) { this->min = min; this->max = max; evalValue(); }
		operator T() const { return val; }
		RND& operator=(const RND<T>& other) { this->max = other.max; this->min = other.min; evalValue(); return *this; }
		RND& operator=(RND<T>&& other) { this->max = other.max; this->min = other.min; evalValue(); return *this; }
	};

	class FXLIB_API ParticleSystem
	{
	public:
		enum ParticleSpace
		{
			LOCAL, // particles will move with the particle system's origin
			GLOBAL // particles will inherit particle system's origin position on emission
		};
		enum SpawnMode
		{
			BOX,
			SPHERE
		};
		struct Particle
		{
			fdm::m4::Mat5 mat{ 1 };
			glm::vec4 pos{ 0 };
			glm::vec4 vel{ 0 };
			glm::vec4 velDeviation{ 0 };
			glm::vec4 startScale{ 1 };
			glm::vec4 endScale{ 0 };
			glm::vec4 startColor{ 1 };
			glm::vec4 endColor{ 0 };
			float time = 0;
			float lifetime = 1;
			size_t trailID = 0;
		};
		struct ParticleData
		{
			fdm::m4::Mat5 model{ 1 };
			glm::vec4 scale{ 1 };
			glm::vec4 color{ 1 };
			float t = 0; // time / lifetime

			glm::vec4& pos() { return *(glm::vec4*)model[4]; }
		};

	private:
		InstancedMeshRenderer renderer;

		size_t maxParticles = 100;
		std::vector<Particle> particles;
		std::vector<ParticleData> gpuData;

	public:
		static const FX::Shader* defaultShader;

		const fdm::Shader* particleShader;
		const fdm::Shader* trailShader;
		TrailRenderer trailRenderer;

		glm::vec4 origin{ 0 };
		glm::vec4 gravity{ 0 };
		glm::vec4 drag{ 0 };
		RND<float> lifetime{ 1.f };
		RND<glm::vec4> startVelocity{ glm::vec4{ 0 } };
		RND<glm::vec4> velocityDeviation{ glm::vec4{ 0 } };
		RND<glm::vec4> startScale{ glm::vec4{ 1 } };
		RND<glm::vec4> endScale{ glm::vec4{ 0 } };
		RND<fdm::m4::Rotor> startRot{ fdm::m4::Rotor{ {0,0,0,0,0,0}, 0.f } };
		RND<fdm::m4::Rotor> endRot{ fdm::m4::Rotor{ {0,0,0,0,0,0}, 0.f } };
		bool angleTowardsVelocity = false;
		glm::vec4 startColor{ 1 };
		glm::vec4 endColor{ 1 };
		ParticleSpace particleSpace = GLOBAL;
		bool trails = false;
		bool billboard = false;

		struct
		{
			glm::vec4 size{ 0 };
		} spawnBoxParams;
		struct
		{
			float radius = 0.5f;
			float force = 2.f;
		} spawnSphereParams;

		SpawnMode spawnMode = BOX;

		// if present, does not apply startRot/endRot, startScale/endScale, startColor/endColor, gravity, deviation, drag.
		// but it does still apply `p.vel` to `p.pos`, as well as `angleTowardsVelocity` to `pData.model` (all of this after running this function).
		// `pData.model` gets set to `p.mat` after this func before applying angle and origin offset.
		std::function<void(ParticleSystem* ps, Particle& p, ParticleData& pData, size_t i, double dt)> evalFunc = nullptr;
		// if present, does not apply startVelocity or spawnModes.
		// but it still does set the `velDeviation`, `startColor/endColor` (tho unused if `evalFunc` is present) and `lifetime` to `p`.
		std::function<void(ParticleSystem* ps, Particle& p, ParticleData& pData, size_t i)> emitFunc = nullptr;
		// a pointer you can, for example, get in `evalFunc` or `emitFunc` through `ps->user`
		void* user = nullptr;

		double lastEmitTime = 0.f;

		~ParticleSystem() { particles.clear(); renderer.~InstancedMeshRenderer(); }
		ParticleSystem() {}
		ParticleSystem(const glm::vec4& origin, RND<float> lifetime, ParticleSpace particleSpace, size_t maxParticles);
		ParticleSystem(
			const glm::vec4& origin,
			const glm::vec4& gravity,
			const glm::vec4& drag,
			float			 lifetimeMin,
			float			 lifetimeMax,
			const glm::vec4& startVelocityMin,
			const glm::vec4& startVelocityMax,
			const glm::vec4& velocityDeviationMin,
			const glm::vec4& velocityDeviationMax,
			const glm::vec4& startScaleMin,
			const glm::vec4& startScaleMax,
			const glm::vec4& endScaleMin,
			const glm::vec4& endScaleMax,
			const fdm::m4::Rotor& startRotMin,
			const fdm::m4::Rotor& startRotMax,
			const fdm::m4::Rotor& endRotMin,
			const fdm::m4::Rotor& endRotMax,
			bool			 angleTowardsVelocity,
			const glm::vec4& startColor,
			const glm::vec4& endColor,
			ParticleSpace	 particleSpace = GLOBAL,
			size_t maxParticles = 100);
		ParticleSystem(
			const glm::vec4& origin,
			const glm::vec4& gravity,
			const glm::vec4& drag,
			const RND<float>& lifetime,
			const RND<glm::vec4>& startVelocity,
			const RND<glm::vec4>& velocityDeviation,
			const RND<glm::vec4>& startScale,
			const RND<glm::vec4>& endScale,
			const RND<fdm::m4::Rotor>& startRot,
			const RND<fdm::m4::Rotor>& endRot,
			bool				  angleTowardsVelocity,
			const glm::vec4& startColor,
			const glm::vec4& endColor,
			ParticleSpace		  particleSpace = GLOBAL,
			size_t maxParticles = 100);
		void initRenderer(const fdm::Mesh* mesh, const fdm::Shader* particleShader, const fdm::Shader* trailShader);
		void initRenderer(const fdm::Mesh* mesh, const FX::Shader* particleShader, const FX::Shader* trailShader) { initRenderer(mesh, (const fdm::Shader*)particleShader, (const fdm::Shader*)trailShader); }
		void update(double dt);
		void render(const fdm::m4::Mat5& view);
		ParticleSystem::Particle* emit(size_t count = 1);
		size_t getMaxParticles() const { return maxParticles; }
		size_t getAliveParticlesCount() const { return particles.size(); }
		const std::vector<Particle>& getParticles() const { return particles; }
		const std::vector<ParticleData>& getParticleData() const { return gpuData; }

		void setMaxParticles(size_t maxParticles);

		void updateParticle(Particle& p, ParticleData& pData, size_t i, double dt);

		void setTrailLifetime(float lifetime = 0.5f) { trailRenderer.lifetime = lifetime; }
		float getTrailLifetime() const { return trailRenderer.lifetime; }
		void setTrailWidthFunc(const decltype(TrailRenderer::widthFunc)& widthFunc) { trailRenderer.widthFunc = widthFunc; }
		void setTrailColorFunc(const decltype(TrailRenderer::colorFunc)& colorFunc) { trailRenderer.colorFunc = colorFunc; }
		void setTrailBillboard(bool billboard) { trailRenderer.billboard = billboard; }
		bool getTrailBillboard() const { return trailRenderer.billboard; }
		void setMinTrailPointDist(float minTrailPointDist) { trailRenderer.minTrailPointDist = minTrailPointDist; }
		bool getMinTrailPointDist() const { return trailRenderer.minTrailPointDist; }
		void resetTrails() { trailRenderer.clearPoints(); }

		ParticleSystem& operator=(const ParticleSystem& other);
		ParticleSystem& operator=(ParticleSystem&& other) noexcept;
	};
}
