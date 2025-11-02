#include "include/fxlib/FXLib.h"
#include "include/fxlib/TrailRenderer.h"

using namespace FX;
using namespace fdm;

const FX::Shader* FX::TrailRenderer::defaultShader = nullptr;

TrailRenderer::TrailRenderer(size_t maxPointsPerTrail, size_t trails)
{
	this->maxPointsPerTrail = maxPointsPerTrail;

	this->trails.reserve(trails);
	for (size_t i = 0; i < trails; i++)
		this->trails.emplace_back(maxPointsPerTrail);

	mesh.indices.reserve(maxPointsPerTrail * trails * 20 + 8);
	mesh.vertices.reserve(maxPointsPerTrail * trails * 4);
}
void TrailRenderer::initRenderer()
{
	renderer.setMesh(&mesh);
}
void TrailRenderer::setMode(GLenum mode)
{
	renderer.mode = mode;
}
void TrailRenderer::render() const
{
	renderer.render();
}
void TrailRenderer::update()
{
	float curTime = glfwGetTime();

	for (auto& trail : trails)
	{
		for (int j = 0; j < trail.points.size();)
		{
			TrailPoint& p = trail.points[j];

			float t = (curTime - p.createTime) / lifetime;

			if (t >= 1.01f)
			{
				trail.points.erase(trail.points.begin() + j);
			}
			else
			{
				++j;
			}
		}
	}
}
void TrailRenderer::updateMesh(const glm::vec4& camLeft, const glm::vec4& camUp, const glm::vec4& camForward, const glm::vec4& camOver)
{
	mesh.vertices.clear();
	mesh.indices.clear();

	if (trails.empty()) return;

	constexpr size_t numThreads = 4;
	size_t trailsPerThread = trails.size() / numThreads;
	size_t remainingTrails = trails.size() % numThreads;

	auto processTrails = [&](size_t start, size_t end, std::vector<TrailRenderer::TrailMesh::Vert>& vertices, std::vector<uint32_t>& indices) {
		size_t v = 0;
		glm::vec4 left, up, over, forward;

		for (size_t id = start; id < end; ++id)
		{
			Trail& trail = trails[id];

			if (trail.points.size() < 2) continue;

			float curTime = glfwGetTime();

			for (size_t i = 0; i < trail.points.size(); ++i)
			{
				size_t nextI = i + 1;
				if (nextI >= trail.points.size())
					nextI = i - 1;

				TrailPoint& a = trail.points[i];
				TrailPoint& b = trail.points[nextI];

				float t = (curTime - a.createTime) / lifetime;
				t = glm::clamp(t, 0.0f, 1.0f);
				float p = (float)i / ((float)trail.points.size() - 1);
				p = glm::clamp(p, 0.0f, 1.0f);

				float widthHalf = widthFunc(t, p, id, user) * 0.5f;
				glm::vec4 color = colorFunc(t, p, id, user);

				glm::vec4 dir = glm::normalize(b.pos - a.pos);
				if (nextI == i - 1)
					dir *= -1;

				left = dir;
				if (a.createdByTrail && i == trail.points.size() - 1)
				{
					forward = glm::normalize(trail.normal);
					up = glm::normalize(trail.tangent);
				}
				else
				{
					forward = glm::normalize(a.normal);
					up = glm::normalize(a.tangent);
				}
				over = glm::normalize(m4::cross(left, forward, up));

				if (billboard)
				{
					up = glm::normalize(m4::cross(left, camForward, camOver));
					forward = glm::normalize(m4::cross(left, up, camOver));
					over = glm::normalize(m4::cross(left, forward, up));
				}

				glm::vec4 pos = a.pos;
				if (a.createdByTrail && i == trail.points.size() - 1)
				{
					pos = trail.pos;
				}

				if (!tesseractal)
				{
					vertices.emplace_back(pos + up * widthHalf + over * widthHalf, forward, up, dir, color, glm::vec3{ p, 1, 1 });
					vertices.emplace_back(pos + up * widthHalf - over * widthHalf, forward, up, dir, color, glm::vec3{ p, 1, 0 });
					vertices.emplace_back(pos - up * widthHalf + over * widthHalf, forward, up, dir, color, glm::vec3{ p, 0, 1 });
					vertices.emplace_back(pos - up * widthHalf - over * widthHalf, forward, up, dir, color, glm::vec3{ p, 0, 0 });
				}
				else
				{
					vertices.emplace_back(pos + up * widthHalf + over * widthHalf - forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 1, 1 });
					vertices.emplace_back(pos + up * widthHalf - over * widthHalf - forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 1, 0 });
					vertices.emplace_back(pos - up * widthHalf + over * widthHalf - forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 0, 1 });
					vertices.emplace_back(pos - up * widthHalf - over * widthHalf - forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 0, 0 });

					vertices.emplace_back(pos + up * widthHalf + over * widthHalf + forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 1, 1 });
					vertices.emplace_back(pos + up * widthHalf - over * widthHalf + forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 1, 0 });
					vertices.emplace_back(pos - up * widthHalf + over * widthHalf + forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 0, 1 });
					vertices.emplace_back(pos - up * widthHalf - over * widthHalf + forward * widthHalf, forward, up, dir, color, glm::vec3{ p, 0, 0 });
				}

				if (i == 0 || i == trail.points.size() - 1)
				{
					if (!tesseractal)
					{
						indices.emplace_back(v + 0);
						indices.emplace_back(v + 1);
						indices.emplace_back(v + 2);
						indices.emplace_back(v + 3);
					}
					else
					{
						indices.emplace_back(v + 0);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 5);
						indices.emplace_back(v + 1);

						indices.emplace_back(v + 5);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 6);
						indices.emplace_back(v + 7);

						indices.emplace_back(v + 0);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 6);
						indices.emplace_back(v + 2);

						indices.emplace_back(v + 0);
						indices.emplace_back(v + 5);
						indices.emplace_back(v + 6);
						indices.emplace_back(v + 4);

						indices.emplace_back(v + 0);
						indices.emplace_back(v + 5);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 6);
					}
				}

				if (i != trail.points.size() - 1)
				{
					if (!tesseractal)
					{
						indices.emplace_back(v + 0);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 5);
						indices.emplace_back(v + 1);

						indices.emplace_back(v + 5);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 6);
						indices.emplace_back(v + 7);

						indices.emplace_back(v + 0);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 6);
						indices.emplace_back(v + 2);

						indices.emplace_back(v + 0);
						indices.emplace_back(v + 5);
						indices.emplace_back(v + 6);
						indices.emplace_back(v + 4);

						indices.emplace_back(v + 0);
						indices.emplace_back(v + 5);
						indices.emplace_back(v + 3);
						indices.emplace_back(v + 6);
					}
					else
					{
						// side -x
						{
							indices.emplace_back(v + 0);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 1);

							indices.emplace_back(v + 5);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 6);
							indices.emplace_back(v + 7);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 6);
							indices.emplace_back(v + 2);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 6);
							indices.emplace_back(v + 4);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 6);
						}

						// side +z
						{
							indices.emplace_back(v + 0);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 9);
							indices.emplace_back(v + 1);

							indices.emplace_back(v + 9);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 11);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 2);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 9);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 8);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 9);
							indices.emplace_back(v + 3);
							indices.emplace_back(v + 10);
						}

						// side -z
						{
							indices.emplace_back(v + 4);
							indices.emplace_back(v + 7);
							indices.emplace_back(v + 13);
							indices.emplace_back(v + 5);

							indices.emplace_back(v + 13);
							indices.emplace_back(v + 7);
							indices.emplace_back(v + 14);
							indices.emplace_back(v + 15);

							indices.emplace_back(v + 4);
							indices.emplace_back(v + 7);
							indices.emplace_back(v + 14);
							indices.emplace_back(v + 6);

							indices.emplace_back(v + 4);
							indices.emplace_back(v + 13);
							indices.emplace_back(v + 14);
							indices.emplace_back(v + 12);

							indices.emplace_back(v + 4);
							indices.emplace_back(v + 13);
							indices.emplace_back(v + 7);
							indices.emplace_back(v + 14);
						}

						// side +y
						{
							indices.emplace_back(v + 0);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 9);
							indices.emplace_back(v + 1);

							indices.emplace_back(v + 9);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 12);
							indices.emplace_back(v + 13);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 12);
							indices.emplace_back(v + 4);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 9);
							indices.emplace_back(v + 12);
							indices.emplace_back(v + 8);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 9);
							indices.emplace_back(v + 5);
							indices.emplace_back(v + 12);
						}

						// side -y
						{
							indices.emplace_back(v + 0 + 2);
							indices.emplace_back(v + 5 + 2);
							indices.emplace_back(v + 9 + 2);
							indices.emplace_back(v + 1 + 2);

							indices.emplace_back(v + 9 + 2);
							indices.emplace_back(v + 5 + 2);
							indices.emplace_back(v + 12 + 2);
							indices.emplace_back(v + 13 + 2);

							indices.emplace_back(v + 0 + 2);
							indices.emplace_back(v + 5 + 2);
							indices.emplace_back(v + 12 + 2);
							indices.emplace_back(v + 4 + 2);

							indices.emplace_back(v + 0 + 2);
							indices.emplace_back(v + 9 + 2);
							indices.emplace_back(v + 12 + 2);
							indices.emplace_back(v + 8 + 2);

							indices.emplace_back(v + 0 + 2);
							indices.emplace_back(v + 9 + 2);
							indices.emplace_back(v + 5 + 2);
							indices.emplace_back(v + 12 + 2);
						}

						// side +w
						{
							indices.emplace_back(v + 0);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 12);
							indices.emplace_back(v + 8);

							indices.emplace_back(v + 12);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 6);
							indices.emplace_back(v + 11);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 6);
							indices.emplace_back(v + 2);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 12);
							indices.emplace_back(v + 6);
							indices.emplace_back(v + 4);

							indices.emplace_back(v + 0);
							indices.emplace_back(v + 12);
							indices.emplace_back(v + 10);
							indices.emplace_back(v + 6);
						}

						// side -w
						{
							indices.emplace_back(v + 0 + 1);
							indices.emplace_back(v + 10 + 1);
							indices.emplace_back(v + 12 + 1);
							indices.emplace_back(v + 8 + 1);

							indices.emplace_back(v + 12 + 1);
							indices.emplace_back(v + 10 + 1);
							indices.emplace_back(v + 6 + 1);
							indices.emplace_back(v + 11 + 1);

							indices.emplace_back(v + 0 + 1);
							indices.emplace_back(v + 10 + 1);
							indices.emplace_back(v + 6 + 1);
							indices.emplace_back(v + 2 + 1);

							indices.emplace_back(v + 0 + 1);
							indices.emplace_back(v + 12 + 1);
							indices.emplace_back(v + 6 + 1);
							indices.emplace_back(v + 4 + 1);

							indices.emplace_back(v + 0 + 1);
							indices.emplace_back(v + 12 + 1);
							indices.emplace_back(v + 10 + 1);
							indices.emplace_back(v + 6 + 1);
						}
					}
				}

				v += !tesseractal ? 4 : 8;
			}
		}
		};

	std::vector<std::vector<TrailRenderer::TrailMesh::Vert>> vertices(numThreads);
	std::vector<std::vector<uint32_t>> indices(numThreads);
	std::vector<std::future<void>> futures;
	futures.reserve(numThreads);

	size_t start = 0;
	for (size_t i = 0; i < numThreads; ++i)
	{
		size_t end = start + trailsPerThread + (i < remainingTrails ? 1 : 0);
		futures.emplace_back(threadPool.enqueue(processTrails, start, end, std::ref(vertices[i]), std::ref(indices[i])));
		start = end;
	}

	for (auto& future : futures)
	{
		future.get();
	}

	for (size_t i = 0; i < numThreads; ++i)
	{
		for (auto& i : indices[i])
		{
			i += mesh.vertices.size();
		}
		mesh.vertices.insert(mesh.vertices.end(), vertices[i].begin(), vertices[i].end());
		mesh.indices.insert(mesh.indices.end(), indices[i].begin(), indices[i].end());
	}

	if (renderer.VAO)
		renderer.updateMesh(&mesh);
}
bool TrailRenderer::addPoint(const glm::vec4& pos, const glm::vec4& normal, const glm::vec4& tangent, size_t trailID, float timeOffset)
{
	if (trailID >= trails.size()) return false;
	Trail& trail = trails[trailID];

	if (trail.points.size() == trail.points.capacity()) return false;

	trail.points.emplace_back(pos, normal, tangent, (float)glfwGetTime() + timeOffset);

	return true;
}
const std::vector<TrailRenderer::TrailPoint>& TrailRenderer::getPoints(size_t trailID) const
{
	if (trailID >= trails.size()) return {};
	const Trail& trail = trails[trailID];

	return trail.points;
}
size_t TrailRenderer::getPointCount(size_t trailID) const
{
	if (trailID >= trails.size()) return 0;
	const Trail& trail = trails[trailID];

	return trail.points.size();
}
size_t TrailRenderer::getPointCount() const
{
	size_t total = 0;
	for (auto& trail : trails)
	{
		total += trail.points.size();
	}
	return total;
}
void TrailRenderer::setMaxPoints(size_t maxPoints)
{
	maxPointsPerTrail = maxPoints;
	for (auto& trail : trails)
	{
		trail.points.reserve(maxPoints);

		if (trail.points.size() > maxPoints)
			trail.points.resize(maxPoints);
	}
	mesh.indices.reserve(maxPointsPerTrail * trails.size() * 20 + 8);
	mesh.vertices.reserve(maxPointsPerTrail * trails.size() * 4);
}
size_t TrailRenderer::getMaxPoints() const
{
	return maxPointsPerTrail;
}
void TrailRenderer::setTrailsCount(size_t trails)
{
	this->trails.reserve(trails);
	if (this->trails.size() < trails)
	{
		for (size_t i = 0; i < trails - this->trails.size(); i++)
			this->trails.emplace_back(maxPointsPerTrail);
	}
	if (this->trails.size() > trails)
	{
		this->trails.resize(trails);
	}
}
size_t TrailRenderer::getTrailsCount() const
{
	return trails.size();
}
size_t TrailRenderer::getVertexCount() const
{
	return mesh.vertices.size();
}
size_t TrailRenderer::getIndexCount() const
{
	return mesh.indices.size();
}
void TrailRenderer::clearPoints(size_t trailID)
{
	if (trailID >= trails.size()) return;
	Trail& trail = trails[trailID];
	trail.points.clear();
}
void TrailRenderer::clearPoints()
{
	for (auto& trail : trails)
	{
		trail.points.clear();
	}
}

int TrailRenderer::TrailMesh::buffCount() const
{
	return 1;
}
const void* TrailRenderer::TrailMesh::buffData(int buffIndex) const
{
	return vertices.data();
}
int TrailRenderer::TrailMesh::buffSize(int buffIndex) const
{
	return vertices.size() * sizeof(Vert);
}
int TrailRenderer::TrailMesh::attrCount(int buffIndex) const
{
	return 6;
}
unsigned int TrailRenderer::TrailMesh::attrType(int buffIndex, int attrIndex) const
{
	return GL_FLOAT;
}
int TrailRenderer::TrailMesh::attrSize(int buffIndex, int attrIndex) const
{
	if (attrIndex == 5)
		return 3;
	return 4;
}
int TrailRenderer::TrailMesh::attrStride(int buffIndex, int attrIndex) const
{
	return sizeof(Vert);
}
int TrailRenderer::TrailMesh::vertCount() const
{
	return indices.size();
}
const void* TrailRenderer::TrailMesh::indexBuffData() const
{
	return indices.data();
}
int TrailRenderer::TrailMesh::indexBuffSize() const
{
	return indices.size() * sizeof(uint32_t);
}

TrailRenderer& TrailRenderer::operator=(const TrailRenderer& other)
{
	this->lifetime = other.lifetime;
	this->widthFunc = other.widthFunc;
	this->colorFunc = other.colorFunc;
	this->user = other.user;
	this->billboard = other.billboard;
	this->tesseractal = other.tesseractal;
	this->trails = other.trails;
	this->mesh.vertices = other.mesh.vertices;
	this->mesh.indices = other.mesh.indices;
	this->maxPointsPerTrail = other.maxPointsPerTrail;
	setMaxPoints(maxPointsPerTrail);

	return *this;
}

TrailRenderer& TrailRenderer::operator=(TrailRenderer&& other) noexcept
{
	this->lifetime = other.lifetime;
	this->widthFunc = other.widthFunc;
	this->colorFunc = other.colorFunc;
	this->user = other.user;
	this->billboard = other.billboard;
	this->tesseractal = other.tesseractal;
	this->trails = other.trails;
	this->mesh.vertices = other.mesh.vertices;
	this->mesh.indices = other.mesh.indices;
	this->maxPointsPerTrail = other.maxPointsPerTrail;
	this->minTrailPointDist = other.minTrailPointDist;
	setMaxPoints(maxPointsPerTrail);

	other.lifetime = 1.f;
	other.widthFunc = defaultWidth;
	other.colorFunc = defaultColor;
	other.user = nullptr;
	other.billboard = false;
	other.tesseractal = false;
	other.trails.clear();
	other.mesh.vertices.clear();
	other.mesh.indices.clear();
	other.maxPointsPerTrail = 0;
	other.minTrailPointDist = 0.2f;

	return *this;
}

void TrailRenderer::setTrailPos(size_t trailID, const glm::vec4& pos, const glm::vec4& normal, const glm::vec4& tangent)
{
	if (trailID >= trails.size()) return;
	Trail& trail = trails[trailID];

	glm::vec4 lastPos = trail.points.empty() ? pos : trail.points.back().pos;
	glm::vec4 diff = pos - lastPos;
	if (trail.points.empty() || glm::abs(glm::dot(diff, diff)) >= minTrailPointDist * minTrailPointDist)
	{
		addPoint(pos, glm::normalize(normal), glm::normalize(tangent), trailID);
		trail.points.back().createdByTrail = true;
	}
	trail.pos = pos;
	trail.normal = normal;
	trail.tangent = tangent;
}

glm::vec4 TrailRenderer::getTrailPos(size_t trailID) const
{
	if (trailID >= trails.size()) return glm::vec4{ 0 };
	const Trail& trail = trails[trailID];

	return trail.pos;
}
glm::vec4 TrailRenderer::getTrailNormal(size_t trailID) const
{
	if (trailID >= trails.size()) return glm::vec4{ 0 };
	const Trail& trail = trails[trailID];

	return trail.normal;
}
glm::vec4 TrailRenderer::getTrailTangent(size_t trailID) const
{
	if (trailID >= trails.size()) return glm::vec4{ 0 };
	const Trail& trail = trails[trailID];

	return trail.tangent;
}

void TrailRenderer::swapTrails(size_t trailA, size_t trailB)
{
	if (trailA >= trails.size()) return;
	if (trailB >= trails.size()) return;

	std::swap(trails[trailA], trails[trailB]);
}
