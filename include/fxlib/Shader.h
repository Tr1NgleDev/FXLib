#pragma once

#include "FXLib.h"
#include "TextureBuffer.h"

namespace FX
{
	// a simple wrapper for the fdm::Shader class with the addition of setUniform functions using DSA
	class Shader
	{
	private:
		uint32_t ID;

	public:
		uint32_t id() const
		{
			return ID;
		}
		void use() const
		{
			glUseProgram(ID);
		}

		int getUniformLocation(const std::string& name) const
		{
			return glGetUniformLocation(ID, name.c_str());
		}

		void setUniform(int loc, int count, const float* v) const { if (loc != -1) glProgramUniform1fv(ID, loc, count, v); }
		void setUniform(int loc, float v) const { if (loc != -1) glProgramUniform1f(ID, loc, v); }
		void setUniform(int loc, const glm::vec2& v) const { if (loc != -1) glProgramUniform2fv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, const glm::vec3& v) const { if (loc != -1) glProgramUniform3fv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, const glm::vec4& v) const { if (loc != -1) glProgramUniform4fv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, bool v) const { if (loc != -1) glProgramUniform1i(ID, loc, v); }
		void setUniform(int loc, int count, const int* v) const { if (loc != -1) glProgramUniform1iv(ID, loc, count, v); }
		void setUniform(int loc, int v) const { if (loc != -1) glProgramUniform1i(ID, loc, v); }
		void setUniform(int loc, const glm::ivec2& v) const { if (loc != -1) glProgramUniform2iv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, const glm::ivec3& v) const { if (loc != -1) glProgramUniform3iv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, const glm::ivec4& v) const { if (loc != -1) glProgramUniform4iv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, int count, const uint32_t* v) const { if (loc != -1) glProgramUniform1uiv(ID, loc, count, v); }
		void setUniform(int loc, uint32_t v) const { if (loc != -1) glProgramUniform1ui(ID, loc, v); }
		void setUniform(int loc, const glm::uvec2& v) const { if (loc != -1) glProgramUniform2uiv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, const glm::uvec3& v) const { if (loc != -1) glProgramUniform3uiv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, const glm::uvec4& v) const { if (loc != -1) glProgramUniform4uiv(ID, loc, 1, &v[0]); }
		void setUniform(int loc, float x, float y) const { if (loc != -1) glProgramUniform2f(ID, loc, x, y); }
		void setUniform(int loc, float x, float y, float z) const { if (loc != -1) glProgramUniform3f(ID, loc, x, y, z); }
		void setUniform(int loc, float x, float y, float z, float w) const { if (loc != -1) glProgramUniform4f(ID, loc, x, y, z, w); }
		void setUniform(int loc, int x, int y) const { if (loc != -1) glProgramUniform2i(ID, loc, x, y); }
		void setUniform(int loc, int x, int y, int z) const { if (loc != -1) glProgramUniform3i(ID, loc, x, y, z); }
		void setUniform(int loc, int x, int y, int z, int w) const { if (loc != -1) glProgramUniform4i(ID, loc, x, y, z, w); }
		void setUniform(int loc, uint32_t x, uint32_t y) const { if (loc != -1) glProgramUniform2ui(ID, loc, x, y); }
		void setUniform(int loc, uint32_t x, uint32_t y, uint32_t z) const { if (loc != -1) glProgramUniform3ui(ID, loc, x, y, z); }
		void setUniform(int loc, uint32_t x, uint32_t y, uint32_t z, uint32_t w) const { if (loc != -1) glProgramUniform4ui(ID, loc, x, y, z, w); }
		void setUniform(int loc, const glm::mat2& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix2fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat3& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix3fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat4& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix4fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat2x3& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix2x3fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat3x2& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix3x2fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat2x4& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix2x4fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat4x2& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix4x2fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat3x4& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix3x4fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const glm::mat4x3& v, bool transpose = false) const { if (loc != -1) glProgramUniformMatrix4x3fv(ID, loc, 1, transpose, &v[0][0]); }
		void setUniform(int loc, const fdm::m4::Mat5& v) const { if (loc != -1) glProgramUniform1fv(ID, loc, 5 * 5, &v[0][0]); }
		void setUniform(int loc, const fdm::m4::BiVector4& v) const { if (loc != -1) glProgramUniform1fv(ID, loc, 6, &v.xy); }
		void setUniform(int loc, const TextureBuffer& v) const { if (loc != -1) glProgramUniformHandleui64ARB(ID, loc, v.getHandle()); }

		void setUniform(const std::string& name, int count, const float* v) const { setUniform(getUniformLocation(name), count, v); }
		void setUniform(const std::string& name, float v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::vec2& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::vec3& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::vec4& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, bool v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, int count, const int* v) const { setUniform(getUniformLocation(name), count, v); }
		void setUniform(const std::string& name, int v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::ivec2& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::ivec3& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::ivec4& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, int count, const uint32_t* v) const { setUniform(getUniformLocation(name), count, v); }
		void setUniform(const std::string& name, uint32_t v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::uvec2& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::uvec3& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const glm::uvec4& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, float x, float y) const { setUniform(getUniformLocation(name), x, y); }
		void setUniform(const std::string& name, float x, float y, float z) const { setUniform(getUniformLocation(name), x, y, z); }
		void setUniform(const std::string& name, float x, float y, float z, float w) const { setUniform(getUniformLocation(name), x, y, z, w); }
		void setUniform(const std::string& name, int x, int y) const { setUniform(getUniformLocation(name), x, y); }
		void setUniform(const std::string& name, int x, int y, int z) const { setUniform(getUniformLocation(name), x, y, z); }
		void setUniform(const std::string& name, int x, int y, int z, int w) const { setUniform(getUniformLocation(name), x, y, z, w); }
		void setUniform(const std::string& name, uint32_t x, uint32_t y) const { setUniform(getUniformLocation(name), x, y); }
		void setUniform(const std::string& name, uint32_t x, uint32_t y, uint32_t z) const { setUniform(getUniformLocation(name), x, y, z); }
		void setUniform(const std::string& name, uint32_t x, uint32_t y, uint32_t z, uint32_t w) const { setUniform(getUniformLocation(name), x, y, z, w); }
		void setUniform(const std::string& name, const glm::mat2& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat3& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat4& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat2x3& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat3x2& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat2x4& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat4x2& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat3x4& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const glm::mat4x3& v, bool transpose = false) const { setUniform(getUniformLocation(name), v, transpose); }
		void setUniform(const std::string& name, const fdm::m4::Mat5& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const fdm::m4::BiVector4& v) const { setUniform(getUniformLocation(name), v); }
		void setUniform(const std::string& name, const TextureBuffer& v) const { setUniform(getUniformLocation(name), v); }
	};
}
