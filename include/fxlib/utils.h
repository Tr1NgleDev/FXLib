#pragma once

#include <random>

namespace FX::utils
{
	inline static float deltaRatio(float ratio, double dt, double targetDelta)
	{
		const double rDelta = dt / (1.0 / (1.0 / targetDelta));
		const double s = 1.0 - ratio;

		return (float)(1.0 - pow(s, rDelta));
	}

	inline static float deltaRatio(float ratio, double dt)
	{
		return deltaRatio(ratio, dt, 1.0 / 100.0);
	}

	inline static float lerp(float a, float b, float ratio, bool clampRatio = true)
	{
		if (clampRatio)
			ratio = glm::clamp(ratio, 0.f, 1.f);
		return a + (b - a) * ratio;
	}

	inline static glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float ratio, bool clampRatio = true)
	{
		return { lerp(a.x, b.x, ratio, clampRatio), lerp(a.y, b.y, ratio, clampRatio) };
	}

	inline static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float ratio, bool clampRatio = true)
	{
		return { lerp(a.x, b.x, ratio, clampRatio), lerp(a.y, b.y, ratio, clampRatio), lerp(a.z, b.z, ratio, clampRatio) };
	}

	inline static glm::vec4 lerp(const glm::vec4& a, const glm::vec4& b, float ratio, bool clampRatio = true)
	{
		return { lerp(a.x, b.x, ratio, clampRatio), lerp(a.y, b.y, ratio, clampRatio), lerp(a.z, b.z, ratio, clampRatio), lerp(a.w, b.w, ratio, clampRatio) };
	}

	inline static fdm::m4::BiVector4 lerp(const fdm::m4::BiVector4& a, const fdm::m4::BiVector4& b, float ratio, bool clampRatio = true)
	{
		return
		{
			lerp(a.xy, b.xy, ratio, clampRatio),
			lerp(a.xz, b.xz, ratio, clampRatio),
			lerp(a.xw, b.xw, ratio, clampRatio),
			lerp(a.yz, b.yz, ratio, clampRatio),
			lerp(a.yw, b.yw, ratio, clampRatio),
			lerp(a.zw, b.zw, ratio, clampRatio)
		};
	}

	inline static fdm::m4::Rotor slerp(const fdm::m4::Rotor& a, const fdm::m4::Rotor& b, float ratio, bool clampRatio = true)
	{
		if (a.a == b.a && a.b == b.b && a.b0123 == b.b0123)
			return a;

		float dot =
			a.a * b.a + a.b.b01 * b.b.b01 + a.b.b02 * b.b.b02 +
			a.b.b03 * b.b.b03 + a.b.b12 * b.b.b12 +
			a.b.b13 * b.b.b13 + a.b.b23 * b.b.b23 +
			a.b0123 * b.b0123;

		dot = glm::clamp(dot, -1.0f, 1.0f);

		float theta = glm::acos(dot);

		if (glm::abs(theta) < 1e-6f)
		{
			fdm::m4::Rotor result{};
			result.a = lerp(a.a, b.a, ratio);
			result.b = lerp(a.b, b.b, ratio);
			result.b0123 = lerp(a.b0123, b.b0123, ratio);
			return result.normalized();
		}

		float sinT = glm::sin(theta);
		float c1 = glm::sin((1.0f - ratio) * theta) / sinT;
		float c2 = glm::sin(ratio * theta) / sinT;

		fdm::m4::Rotor result{};
		result.a = c1 * a.a + c2 * b.a;
		result.b.b01 = c1 * a.b.b01 + c2 * b.b.b01;
		result.b.b02 = c1 * a.b.b02 + c2 * b.b.b02;
		result.b.b03 = c1 * a.b.b03 + c2 * b.b.b03;
		result.b.b12 = c1 * a.b.b12 + c2 * b.b.b12;
		result.b.b13 = c1 * a.b.b13 + c2 * b.b.b13;
		result.b.b23 = c1 * a.b.b23 + c2 * b.b.b23;
		result.b0123 = c1 * a.b0123 + c2 * b.b0123;

		return result.normalized();
	}

	inline static float ilerp(float a, float b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}

	inline static glm::vec2 ilerp(const glm::vec2& a, const glm::vec2& b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}

	inline static glm::vec3 ilerp(const glm::vec3& a, const glm::vec3& b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}

	inline static glm::vec4 ilerp(const glm::vec4& a, const glm::vec4& b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}

	inline static fdm::m4::BiVector4 ilerp(const fdm::m4::BiVector4& a, const fdm::m4::BiVector4& b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}

	inline static double prevVal = -10.0 / 3.0;
	inline static int prevValI = -3;

	inline static float random(float min, float max)
	{
		static std::random_device device;
		std::mt19937 generator(device() + (int)(prevVal * 35248.356314f));
		std::uniform_real_distribution<float> distribution(glm::min(min, max), glm::max(min, max));

		return prevVal = distribution(generator);
	}

	inline static double random(double min, double max)
	{
		static std::random_device device;
		std::mt19937 generator(device() + (int)(prevVal * 35248.356314f));
		std::uniform_real_distribution<double> distribution(glm::min(min, max), glm::max(min, max));

		return prevVal = distribution(generator);
	}

	inline static int random(int min, int max)
	{
		static std::random_device device;
		std::mt19937 generator(device() + (int)(prevVal * 35248.356314f));
		std::uniform_int_distribution<int> distribution(glm::min(min, max), glm::max(min, max));

		return prevVal = distribution(generator);
	}

	inline static glm::vec2 random(const glm::vec2& min, const glm::vec2& max)
	{
		return { random(min.x, max.x), random(min.y, max.y) };
	}
	inline static glm::vec3 random(const glm::vec3& min, const glm::vec3& max)
	{
		return { random(min.x, max.x), random(min.y, max.y), random(min.z, max.z) };
	}
	inline static glm::vec4 random(const glm::vec4& min, const glm::vec4& max)
	{
		return { random(min.x, max.x), random(min.y, max.y), random(min.z, max.z), random(min.w, max.w) };
	}
	inline fdm::m4::BiVector4 random(const fdm::m4::BiVector4& min, const fdm::m4::BiVector4& max)
	{
		return
		{
			random(min.xy, max.xy),
			random(min.xz, max.xz),
			random(min.xw, max.xw),
			random(min.yz, max.yz),
			random(min.yw, max.yw),
			random(min.zw, max.zw)
		};
	}
	inline fdm::m4::Rotor random(const fdm::m4::Rotor& min, const fdm::m4::Rotor& max)
	{
		return slerp(min, max, random(0.0f, 1.0f));
	}

	inline void trimStart(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](uint8_t c) {
			return !std::isspace(c);
			}));
	}
	inline void trimEnd(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](uint8_t c) {
			return !std::isspace(c);
			}).base(), s.end());
	}
	inline void trim(std::string& s)
	{
		trimStart(s);
		trimEnd(s);
	}
	inline void toLower(std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(),
			[](uint8_t c) { return std::tolower(c); });
	}
	inline void toUpper(std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(),
			[](uint8_t c) { return std::toupper(c); });
	}
}
