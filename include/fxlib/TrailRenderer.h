#pragma once

#include "FXLib.h"

#include "ThreadPool.h"

namespace FX
{
	class FXLIB_API TrailRenderer
	{
	public:
		struct TrailPoint
		{
			glm::vec4 pos{ 0 };
			glm::vec4 normal{ 0 };
			glm::vec4 tangent{ 0 };
			float createTime = 0.f;
			bool createdByTrail = false;
			float width;
			glm::vec4 color;
		};
		
		struct Trail
		{
			std::vector<TrailPoint> points{ };
			glm::vec4 pos{ 0 };
			glm::vec4 normal{ 0 };
			glm::vec4 tangent{ 0 };

			Trail(size_t maxPoints = 0) { points.reserve(maxPoints); }
		};

		class TrailMesh : public fdm::Mesh
		{
		public:
			struct Vert
			{
				glm::vec4 pos{ 0 }, normal{ 0 }, tangent{ 0 }, biTangent{ 0 }, color{ 1 };
				glm::vec3 uvw;
			};
			std::vector<Vert> vertices{ };
			std::vector<uint32_t> indices{ };

			int buffCount() const override;
			const void* buffData(int buffIndex) const override;
			int buffSize(int buffIndex) const override;
			int attrCount(int buffIndex) const override;
			unsigned int attrType(int buffIndex, int attrIndex) const override;
			int attrSize(int buffIndex, int attrIndex) const override;
			int attrStride(int buffIndex, int attrIndex) const override;
			int vertCount() const override;
			const void* indexBuffData() const override;
			int indexBuffSize() const override;
		};

	private:
		std::vector<Trail> trails{ };
		TrailMesh mesh{ };
		fdm::MeshRenderer renderer{ };
		size_t maxPointsPerTrail = 0;
		inline static ThreadPool threadPool{ 4 };

	public:
		static const FX::Shader* defaultShader;

		inline static float defaultWidth(float t, float p, size_t trailID, void* user) { return 1.f; }
		inline static glm::vec4 defaultColor(float t, float p, size_t trailID, void* user) { return { 1.f, 1.f, 1.f, p }; }

		float lifetime = 1.f;
		std::function<float(float t, float p, size_t trailID, void* user)> widthFunc = defaultWidth;
		std::function<glm::vec4(float t, float p, size_t trailID, void* user)> colorFunc = defaultColor;
		void* user = nullptr;
		bool billboard = false;
		bool tesseractal = false;
		float minTrailPointDist = 0.1f;

		TrailRenderer(size_t maxPointsPerTrail = 100, size_t trails = 1);
		void initRenderer();
		void update();
		void updateMesh(const glm::vec4& camLeft, const glm::vec4& camUp, const glm::vec4& camForward, const glm::vec4& camOver);
		bool addPoint(const glm::vec4& pos, const glm::vec4& normal, const glm::vec4& tangent, size_t trailID = 0, float timeOffset = 0);
		const std::vector<TrailPoint>& getPoints(size_t trailID = 0) const;
		size_t getPointCount(size_t trailID) const;
		size_t getPointCount() const;
		void setMaxPoints(size_t maxPoints);
		size_t getMaxPoints() const;
		void setTrailsCount(size_t trails);
		size_t getTrailsCount() const;
		void setMode(GLenum mode);
		void render() const;
		size_t getVertexCount() const;
		size_t getIndexCount() const;
		void clearPoints(size_t trailID);
		void clearPoints();
		void setTrailPos(size_t trailID, const glm::vec4& pos, const glm::vec4& normal, const glm::vec4& tangent);
		glm::vec4 getTrailPos(size_t trailID) const;
		glm::vec4 getTrailNormal(size_t trailID) const;
		glm::vec4 getTrailTangent(size_t trailID) const;
		void swapTrails(size_t trailA, size_t trailB);

		TrailRenderer& operator=(const TrailRenderer& other);
		TrailRenderer& operator=(TrailRenderer&& other) noexcept;
	};
}
